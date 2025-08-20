#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int fd = open("seccomp_out.txt", O_WRONLY | O_CREAT, 0600); // 조건 규칙 → SIGSYS
    if (fd < 0) { perror("open failed (maybe different reason)"); return 1; }
    write(fd, "X", 1);
    close(fd);
    puts("file write success (SHOULD NOT SEE)");
    return 0;
}