#include "common/stdlib.h"
#include "mem/paging.h"
#include "mem/kheap.h"
#include "monitor/monitor.h"
#include "sync/yieldlock.h"
#include "task/thread.h"
#include "task/process.h"
#include "task/scheduler.h"
#include "utils/math.h"
#include "utils/hash_table.h"
#include "utils/debug.h"

// kernel's page directory
static page_directory_t kernel_page_directory;

// the current page directory;
page_directory_t* current_page_directory = 0;

static bitmap_t phy_frames_map;
static uint32 bitarray[PHYSICAL_MEM_SIZE / PAGE_SIZE / 32];
static yieldlock_t phy_frames_map_lock;

// copy-on-write frames' reference counts
static bool copy_on_write_ready = false;
static hash_table_t frame_cow_ref_counts;
static yieldlock_t frame_cow_ref_counts_lock;

// locks for page copy
static yieldlock_t page_copy_lock;

void init_paging() {
  // Initialize phy_frames_map. Note we have already used the first 3MB for kernel initialization.
  //
  // totally 8192 frames
  for (int i = 0; i < 3 * 1024 * 1024 / PAGE_SIZE / 32; i++) {
    bitarray[i] = 0xFFFFFFFF;
  }
  phy_frames_map = bitmap_create(bitarray, PHYSICAL_MEM_SIZE / PAGE_SIZE);
  // Last virtual page is kernel stack.
  bitmap_set_bit(&phy_frames_map, PHYSICAL_MEM_SIZE / PAGE_SIZE - 1);

  // Initialize page directory.
  kernel_page_directory.page_dir_entries_phy = KERNEL_PAGE_DIR_PHY;
  current_page_directory = &kernel_page_directory;

  // Release memory for loading kernel binany - it's no longer needed.
  release_pages(0xFFFFFFFF - KERNEL_BIN_LOAD_SIZE + 1, KERNEL_BIN_LOAD_SIZE / PAGE_SIZE, true);

  // Register page fault handler.
  register_interrupt_handler(14, page_fault_handler);
}

void init_paging_stage2() {
  yieldlock_init(&phy_frames_map_lock);

  hash_table_init(&frame_cow_ref_counts);
  yieldlock_init(&frame_cow_ref_counts_lock);

  yieldlock_init(&page_copy_lock);

  copy_on_write_ready = true;
}

int32 allocate_phy_frame() {
  yieldlock_lock(&phy_frames_map_lock);
  uint32 frame;
  if (!bitmap_allocate_first_free(&phy_frames_map, &frame)) {
    yieldlock_unlock(&phy_frames_map_lock);
    return -1;
  }
  yieldlock_unlock(&phy_frames_map_lock);
  return (int32)frame;
}

void release_phy_frame(uint32 frame) {
  yieldlock_lock(&phy_frames_map_lock);
  bitmap_clear_bit(&phy_frames_map, frame);
  yieldlock_unlock(&phy_frames_map_lock);
}

void clear_page(uint32 addr) {
  addr = addr / PAGE_SIZE * PAGE_SIZE;
  for (int i = 0; i < PAGE_SIZE / 4; i++) {
    *(((uint32*)addr) + i) = 0;
  }
}

void enable_paging() {
  uint32 cr0;
  asm volatile("mov %%cr0, %0": "=r"(cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void reload_page_directory(page_directory_t *dir) {
  current_page_directory = dir;
  asm volatile("mov %0, %%cr3":: "r"(dir->page_dir_entries_phy));
}

page_directory_t* get_crt_page_directory() {
  return current_page_directory;
}

static int32 change_cow_frame_refcount(uint32 frame, int32 refcount_delta) {
  if (!copy_on_write_ready) {
    return 0;
  }

  yieldlock_lock(&frame_cow_ref_counts_lock);
  int32* cnt_ptr = hash_table_get(&frame_cow_ref_counts, frame);
  int32 old_cnt;
  int32 new_cnt;
  if (cnt_ptr != nullptr) {
    old_cnt = *cnt_ptr;
    new_cnt = old_cnt + refcount_delta;
  } else {
    old_cnt = 0;
    new_cnt = old_cnt + refcount_delta;
  }

  if (new_cnt > 0) {
    if (cnt_ptr != nullptr) {
      *cnt_ptr = new_cnt;
    } else {
      cnt_ptr = (int32*)kmalloc(sizeof(int32));
      *cnt_ptr = new_cnt;
      hash_table_put(&frame_cow_ref_counts, frame, cnt_ptr);
    }
  } else {
    if (cnt_ptr != nullptr) {
      hash_table_remove(&frame_cow_ref_counts, frame);
    }
  }
  yieldlock_unlock(&frame_cow_ref_counts_lock);
  return old_cnt;
}

void page_fault_handler(isr_params_t params) {
  // The faulting address is stored in the CR2 register
  uint32 faulting_address;
  asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

  // The error code gives us details of what happened.
  // page not present?
  int present = params.err_code & 0x1;
  // write operation?
  int rw = params.err_code & 0x2;
  // processor was in user-mode?
  int user_mode = params.err_code & 0x4;
  // overwritten CPU-reserved bits of page entry?
  int reserved = params.err_code & 0x8;
  // caused by an instruction fetch?
  int id = params.err_code & 0x10;

  // handle page fault
  // monitor_printf(
  //  "page fault: %x, present %d, write %d, user-mode %d, reserved %d\n",
  //  faulting_address, present, rw, user_mode, reserved);

  map_page(faulting_address / PAGE_SIZE * PAGE_SIZE);
  reload_page_directory(current_page_directory);
}

// Note this function itself must NOT trigger another page fault inside.
static void map_page_with_frame_impl(uint32 virtual_addr, int32 frame) {
  // Lookup pde - note we use virtual address 0xC0701000 to access page
  // directory, which is the actually the 2nd page table of kernel space.
  uint32 pde_index = virtual_addr >> 22;
  pde_t* pd = (pde_t*)PAGE_DIR_VIRTUAL;
  pde_t* pde = pd + pde_index;

  // Allcoate page table for this pde, if needed.
  if (!pde->present) {
    int32 page_table_frame = allocate_phy_frame();
    if (page_table_frame < 0) {
      monitor_printf("couldn't alloc frame for page table on %d\n", pde_index);
      PANIC();
    }
    //monitor_printf("alloca frame %d for page table %d\n", frame, pde_index);
    pde->present = 1;
    pde->rw = 1;
    pde->user = 1;
    pde->frame = page_table_frame;

    // Reset page table pointed by this pde.
    reload_page_directory(current_page_directory);
    clear_page(PAGE_TABLES_VIRTUAL + pde_index * PAGE_SIZE);
  }

  // Lookup pte - still use virtual address. Note all 1024 page tables are
  // continuous in virtual space, from 0xC0400000 to 0xC0800000.
  uint32 pte_index = virtual_addr >> 12;
  pte_t* kernel_page_tables_virtual = (pte_t*)PAGE_TABLES_VIRTUAL;
  pte_t* pte = kernel_page_tables_virtual + pte_index;
  if (frame > 0) {
    // If frame is provided, simply map it.
    pte->present = 1;
    pte->rw = 1;
    pte->user = 1;
    pte->frame = frame;
    reload_page_directory(current_page_directory);
  } else {
    if (!pte->present) {
      // Allocate a new frame and map it.
      frame = allocate_phy_frame();
      if (frame < 0) {
        monitor_printf("couldn't alloc frame for addr %x\n", virtual_addr);
        PANIC();
      }

      pte->present = 1;
      pte->rw = 1;
      pte->user = 1;
      pte->frame = frame;
      reload_page_directory(current_page_directory);
      clear_page(virtual_addr);
    } else if (!pte->rw) {
      //monitor_printf("handle page fault rw on %x\n", virtual_addr);

      // Handle copy-on-write page fault:
      //   - copy the content of this page to a new frame;
      //   - re-map fault page to the new frame.
      //   - decrease ref count of shared frame.
      int32 cow_refs = change_cow_frame_refcount(pte->frame, -1);
      if (cow_refs > 0) {
        //monitor_printf("cow copy %x on process %d\n", virtual_addr, get_crt_thread()->process->id);

        // Allocate a new frame for "copy" on write.
        frame = allocate_phy_frame();
        if (frame < 0) {
          monitor_printf("couldn't alloc frame for addr %x\n", virtual_addr);
          PANIC();
        }

        // Do NOT kmalloc page for copying, because kmalloc may trigger another page fault
        // which will result in a deadlock.
        yieldlock_lock(&page_copy_lock);
        void* copy_page = (void*)COPIED_PAGE_VADDR;
        map_page_with_frame_impl((uint32)copy_page, frame);
        memcpy(copy_page, (void*)(virtual_addr / PAGE_SIZE * PAGE_SIZE), PAGE_SIZE);
        yieldlock_unlock(&page_copy_lock);
        pte->frame = frame;
        pte->rw = 1;

        //kfree(copy_page);
        release_pages((uint32)copy_page, 1, false);
      } else {
        //monitor_printf("cow rw %x on process %d\n", virtual_addr, get_crt_thread()->process->id);
        pte->rw = 1;
        reload_page_directory(current_page_directory);
      }
    }
  }
}

static void map_page_with_frame(uint32 virtual_addr, int32 frame) {
  if (multi_task_is_enabled()) {
    yieldlock_lock(&get_crt_thread()->process->page_dir_lock);
  }
  map_page_with_frame_impl(virtual_addr, frame);
  if (multi_task_is_enabled()) {
    yieldlock_unlock(&get_crt_thread()->process->page_dir_lock);
  }
}

void map_page(uint32 virtual_addr) {
  map_page_with_frame(virtual_addr, -1);
}

static void release_page(uint32 virtual_addr, bool free_frame) {
  // reset pte
  uint32 pte_index = virtual_addr >> 12;
  pte_t* pte = (pte_t*)PAGE_TABLES_VIRTUAL + pte_index;
  if (!pte->present) {
    return;
  }
  uint32 frame = pte->frame;
  if (free_frame) {
    //monitor_printf("release page %x\n", virtual_addr);

    // Decrease ref count of the cow frame.
    int32 cow_refs = change_cow_frame_refcount(frame, -1);
    if (cow_refs <= 0) {
      release_phy_frame(frame);
      //clear_page(virtual_addr);
    }
  }

  *((uint32*)pte) = 0;
  reload_page_directory(current_page_directory);
}

void release_pages(uint32 virtual_addr, uint32 pages, bool free_frame) {
  virtual_addr = (virtual_addr / PAGE_SIZE) * PAGE_SIZE;

  uint32 pte_index_start = (virtual_addr >> 12);
  uint32 pte_index_end = pte_index_start + pages;

  uint32 pde_index_start = (pte_index_start >> 10);
  uint32 pde_index_end = ((pte_index_end - 1) >> 10) + 1;

  for (uint32 i = pde_index_start; i < pde_index_end; i++) {
    pde_t* pde = (pde_t*)PAGE_DIR_VIRTUAL + i;
    if (!pde->present) {
      continue;
    }

    for (uint32 j = max(pte_index_start, i * 1024); j < min(pte_index_end, i * 1024 + 1024); j++) {
      release_page(j * PAGE_SIZE, free_frame);
    }
  }
}

void release_pages_tables(uint32 pde_index_start, uint32 num) {
  for (uint32 i = pde_index_start; i < pde_index_start + num; i++) {
    pde_t* pde = (pde_t*)PAGE_DIR_VIRTUAL + i;
    if (!pde->present) {
      continue;
    }
    release_phy_frame(pde->frame);
    *((uint32*)pde) = 0;
  }
}

// Copy the entire page dir and its page tables:
//  - 256 kernel page tables are shared;
//  - user space page tables are copied, and page entries are marked copy-on-write;
//
// Return the physical address of new page dir.
page_directory_t clone_crt_page_dir() {
  // Map copied page tables (including the new page dir) to some virtual space so that
  // we can access them.
  //
  // First, map the new page dir.
  int32 new_pd_frame = allocate_phy_frame();
  if (new_pd_frame < 0) {
    monitor_printf("couldn't alloc frame for new page dir\n");
    PANIC();
  }

  uint32 copied_page_dir = (uint32)kmalloc_aligned(PAGE_SIZE);
  map_page_with_frame(copied_page_dir, new_pd_frame);
  reload_page_directory(current_page_directory);
  clear_page(copied_page_dir);

  // First page dir entry is shared - the first 4MB virtual space is reserved.
  pde_t* new_pd = (pde_t*)copied_page_dir;
  pde_t* crt_pd = (pde_t*)PAGE_DIR_VIRTUAL;
  *new_pd = *crt_pd;

  // Share all 256 kernel pdes - except pde 769, which should points to new page dir itself.
  for (uint32 i = 768; i < 1024; i++) {
    pde_t* new_pde = new_pd + i;
    if (i == 769) {
      new_pde->present = 1;
      new_pde->rw = 1;
      new_pde->user = 1;
      new_pde->frame = new_pd_frame;
    } else {
      *new_pde = *(crt_pd + i);
    }
  }

  // Copy user space page tables.
  uint32 copied_page_table = (uint32)kmalloc_aligned(PAGE_SIZE);

  for (uint32 i = 1; i < 768; i++) {
    pde_t* crt_pde = crt_pd + i;
    if (!crt_pde->present) {
      continue;
    }

    // Alloc a new frame for copied page table.
    int32 new_pt_frame = allocate_phy_frame();
    if (new_pt_frame < 0) {
      monitor_printf("couldn't alloc frame for copied page table\n");
      PANIC();
    }

    // Copy page table and set ptes copy-on-write.
    map_page_with_frame(copied_page_table, new_pt_frame);
    reload_page_directory(current_page_directory);
    memcpy((void*)copied_page_table, (void*)(PAGE_TABLES_VIRTUAL + i * PAGE_SIZE), PAGE_SIZE);
    for (int j = 0; j < 1024; j++) {
      pte_t* crt_pte = (pte_t*)(PAGE_TABLES_VIRTUAL + i * PAGE_SIZE) + j;
      pte_t* new_pte = (pte_t*)copied_page_table + j;
      if (!new_pte->present) {
        continue;
      }
      // Mark copy-on-write: increase copy-on-write ref count.
      crt_pte->rw = 0;
      new_pte->rw = 0;
      int32 cow_refs = change_cow_frame_refcount(new_pte->frame, 1);
    }

    // Set page dir entry.
    pde_t* new_pde = new_pd + i;
    new_pde->present = 1;
    new_pde->rw = 1;
    new_pde->user = 1;
    new_pde->frame = new_pt_frame;
  }

  // Release mapping for new page tables on current process.
  kfree((void*)copied_page_dir);
  kfree((void*)copied_page_table);
  release_pages(copied_page_dir, 1, false);
  release_pages(copied_page_table, 1, false);

  page_directory_t page_directory;
  page_directory.page_dir_entries_phy = new_pd_frame * PAGE_SIZE;
  return page_directory;
}

// ******************************** unit tests **********************************
void memory_killer() {
  uint32 *ptr = (uint32*)0xC0900000;
  while (1) {
    *ptr = 3;
    ptr += 1;
  }
}
