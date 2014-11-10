// This is a C function to set the pgid of our processes

#include <unistd.h>

int main(pid_t pid){
  setpgid(pid,pid);
  return 0;
}
