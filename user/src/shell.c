#include "common/common.h"
#include "common/stdio.h"
#include "common/stdlib.h"
#include "syscall/syscall.h"
#include "driver/keyhelp.h"
#include "utils/math.h"

#define LINE_CURSOR_INDEX_MIN  3

char cmd_buffer[512];
int32 cmd_end_index = 0;

int32 line_cursor_index = 3;

char* blank_line =
    "                                                                                ";

static void print_shell() {
  printf("duan@luck$ ");
}

static int32 run_program() {
  printf("\n");

  char program[128];
  char arg_buffer[32 * 128];
  int32 arg_buffer_index = 0;
  char* args[32];

  int32 split_start = -1;
  int32 split_end = -1;
  bool program_get = false;
  bool in_token = false;
  int32 args_index = 0;
  for (int32 i = 0; i <= cmd_end_index; i++) {
    if (!in_token) {
      if (cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') {
        split_start = i;
        in_token = true;
      }
      continue;
    } else {
      if (cmd_buffer[i] == ' ' || cmd_buffer[i] == '\0') {
        split_end = i;
        in_token = false;
      } else {
        continue;
      }
    }

    if (!program_get) {
      int32 length = split_end - split_start;
      length = min(length, 127);
      memcpy(program, cmd_buffer + split_start, length);
      program[length] = '\0';
      program_get = true;
    } else {
      int32 length = split_end - split_start;
      length = min(length, 127);
      memcpy(arg_buffer + arg_buffer_index, cmd_buffer + split_start, length);
      args[args_index] = arg_buffer + arg_buffer_index;
      arg_buffer_index += length;
      arg_buffer[arg_buffer_index++] = '\0';
      args_index++;
    }
  }

  //printf("get program %s, %d args\n", program, args_index);
  //for (int32 i = 0; i < args_index; i++) {
  //  printf("%s\n", args[i]);
  //}

  int32 pid = fork();
  if (pid < 0) {
    printf("fork failed");
  } else if (pid > 0) {
    // parent
    //printf("created child process %d\n", pid);
    int32 status;
    wait(pid, &status);
    //printf("child process %d exit with code %d\n", pid, status);
  } else {
    // child
    //printf("child process started\n");
    int32 ret = exec(program, args_index, (char**)args);
    exit(ret);
  }
}

static void clear_screen() {
  move_cursor(0, -25);
  for (int i = 0; i < 25; i++) {
    printf(blank_line);
  }
  move_cursor(0, -25);
}

static int32 run_shell() {
  cmd_end_index = 0;
  cmd_buffer[cmd_end_index] = '\0';

  while (1) {
    cmd_end_index = 0;
    line_cursor_index = 3;
    print_shell();

    while (1) {
      int32 c = read_char();

      if (c < 128) {
        //printf("%d", c);
        if (c == '\b') {
          if (line_cursor_index - 3 == 0) {
            continue;
          }
          for (int32 i = (line_cursor_index - 3); i < cmd_end_index; i++) {
            cmd_buffer[i - 1] = cmd_buffer[i];
          }
          cmd_end_index--;
          cmd_buffer[cmd_end_index] = '\0';
          line_cursor_index--;
          move_cursor(-1, 0);
          printf(cmd_buffer + (line_cursor_index - 3));
          printf(" ");
          move_cursor(-(cmd_end_index + 1 - (line_cursor_index - 3)), 0);
        } if (c == 12) {
          clear_screen();
          break;
        } else if (c == '\n') {
          if (cmd_end_index == 0) {
            printf("\n");
            break;
          } else if (strcmp("exit", cmd_buffer) == 0) {
            printf("\n");
            return 0;
          } else if (strcmp("clear", cmd_buffer) == 0) {
            clear_screen();
            break;
          } else {
            run_program();
          }
          break;
        } else if (c >= 32) {
          for (int32 i = cmd_end_index - 1; i >= (line_cursor_index - 3); i--) {
            cmd_buffer[i + 1] = cmd_buffer[i];
          }
          cmd_buffer[line_cursor_index - 3] = c;
          cmd_end_index++;
          cmd_buffer[cmd_end_index] = '\0';
          printf(cmd_buffer + (line_cursor_index - 3));
          line_cursor_index++;
          move_cursor(-(cmd_end_index - (line_cursor_index - 3)), 0);
        }
      } else {
        //printf("%d", c);
        switch (c) {
          case KHE_ARROW_UP: {
            //printf("arrow down ");
            break;
          }
          case KHE_ARROW_DOWN: {
            //printf("arrow down ");
            break;
          }
          case KHE_ARROW_LEFT: {
            if (line_cursor_index == LINE_CURSOR_INDEX_MIN) {
              break;
            }
            line_cursor_index--;;
            move_cursor(-1, 0);
            break;
          }
          case KHE_ARROW_RIGHT: {
            if (line_cursor_index == cmd_end_index + 3) {
              break;              
            }
            line_cursor_index++;
            move_cursor(1, 0);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

int32 main(uint32 argc, char* argv[]) {
  return run_shell();
}
