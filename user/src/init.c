#include "common/common.h"
#include "common/stdio.h"
#include "syscall/syscall.h"

// The "init" process (pid = 1), will:
//  - Switch to user mode and start shell;
//  - Serves as the system daemon process;
int main(uint32 argc, char* argv[]) {
  int32 pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
  } else if (pid > 0) {
    // parent: thread-3
    //printf("created child process %d\n", pid);
    uint32 status;
    wait(pid, &status);
    //printf("child process %d exit with code %d\n", pid, status);

    // TODO: do infinite wait()
    while (1) {}
  } else {
    // child: thread-4
    //printf("child process start ok\n");

    // thread-5
    char* prog = "shell";
    exec(prog, 0, nullptr);
  }
}
