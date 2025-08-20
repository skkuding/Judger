#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

int main() {
    int fd = open("/", O_RDONLY | O_DIRECTORY);
    if (fd < 0) { perror("open"); return 1; }
    char buf[4096];
#ifdef SYS_getdents64
    long r = syscall(SYS_getdents64, fd, buf, sizeof(buf)); // blacklist → SIGSYS
#else
    #error "SYS_getdents64 not defined on this arch"
#endif
    printf("getdents64 returned %ld (SHOULD NOT SEE)\n", r);
    return 0;
}