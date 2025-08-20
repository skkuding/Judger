#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int main(){
    int fd = open("seccomp_write_test.tmp", O_WRONLY|O_CREAT, 0600);
    if(fd==-1) perror("open");
    else { write(fd,"x",1); close(fd); }
    return 0;
}
