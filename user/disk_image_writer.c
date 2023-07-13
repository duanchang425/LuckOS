#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
  char* dir_path = "./progs";
  if (argc > 1) {
    dir_path = argv[1];
  }

  char* file_names[256];
  char* file_paths[256];

  // List all programs.
  DIR *d = opendir(dir_path);
  struct dirent *dir;
  int num = 0;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
        continue;
      }

      char* filename = (char*)malloc(strlen(dir->d_name) + 1);
      strcat(filename, dir->d_name);
      file_names[num] = filename;

      char* filepath = (char*)malloc(strlen(dir->d_name) + strlen(dir_path) + 2);
      strcat(filepath, dir_path);
      strcat(filepath, "/");
      strcat(filepath, dir->d_name);
      file_paths[num] = filepath;

      num++;
    }
    closedir(d);
  }

  // Create disk image file.
  FILE* image_file;
  char buffer[100] = "This is a test";
  if ((image_file = fopen("./user_disk_image", "w+")) == 0) {
    printf("create image file failed");
    exit(1);
  }

  fwrite((const void*)&num, sizeof(int), 1, image_file);
  char char_end = '\0';

  int data_offset = sizeof(int) + num * (64 + sizeof(int) + sizeof(int));
  int file_size[num];
  int file_data_offsets[num];

  // Write meta data.
  for (int i = 0; i < num; i++) {
    char* file_name = file_names[i];
    char* file_path = file_paths[i];

    struct stat st;
    stat(file_path, &st);
    int size = st.st_size;
    file_size[i] = size;
    //printf("%s, length = %ld, size = %d\n", file_path, strlen(file_path), size);

    fwrite((const void*)file_name, sizeof(char), strlen(file_name), image_file);
    for (int j = 0; j < 64 - strlen(file_name); j++) {
      fwrite((const void*)&char_end, sizeof(char), 1, image_file);
    }
    fwrite((const void*)&size, sizeof(int), 1, image_file);

    file_data_offsets[i] = data_offset;
    fwrite((const void*)&file_data_offsets[i], sizeof(int), 1, image_file);

    data_offset += size;
  }

  // Write file data.
  for (int i = 0; i < num; i++) {
    char* file_path = file_paths[i];
    int size = file_size[i];
    int offset = file_data_offsets[i];

    FILE* prog_file;
    if ((prog_file = fopen(file_path, "r")) == 0) {
      printf("open %s failed!", file_path);
      exit(1);
    }
    char* buffer = (char*)malloc(size);
    fread(buffer, 1, size, prog_file);
    fwrite(buffer, 1, size, image_file);
    free(buffer);
    fclose(prog_file);
  }

  // Close.
  fclose(image_file);

  // Print and verify.
  image_file = fopen("./user_disk_image", "r");
  if (image_file == 0) {
    printf("open iamge file failed!");
    exit(1);
  }
  int file_num;
  fread(&file_num, sizeof(int), 1, image_file);
  printf("disk image has %d files\n", file_num);
  for (int i = 0; i < file_num; i++) {
    fseek(image_file, 4 + i * (64 + sizeof(int) + sizeof(int)), SEEK_SET);

    char filename[64];
    fread(filename, sizeof(char), 64, image_file);
    int size;
    fread(&size, sizeof(int), 1, image_file);
    int data_offset;
    fread(&data_offset, sizeof(int), 1, image_file);
    printf(" - %s, data_offset = %d, size = %d\n", filename, data_offset, size);

    // read data
    char* buffer = (char*)malloc(size);
    fseek(image_file, data_offset, SEEK_SET);
    fread(buffer, 1, size, image_file);

    // get origin file data.
    char* file_path = file_paths[i];
    FILE* origin_file;
    if ((origin_file = fopen(file_path, "r")) == 0) {
      printf("open origin file %s failed!", file_path);
      exit(1);
    }
    char* origin_file_buffer = (char*)malloc(size);
    fread(origin_file_buffer, 1, size, origin_file);

    // compare
    if (memcmp(buffer, origin_file_buffer, size) != 0) {
      printf("file %s diff!\n", file_path);
      exit(1);
    }

    free(buffer);
    free(origin_file_buffer);
    fclose(origin_file);
  }
}
