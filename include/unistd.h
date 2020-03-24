#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>
#include <system/system_call.h>
// #include <system/fs.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int get_ticks(); //临时

void exit(int status);
pid_t fork();
ssize_t read(int fd, const void *buf, size_t nbytes);
ssize_t write(int fd, const void *buf, size_t nbytes);
int open(const char *path, int flags, ...);
int close(int fd);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int execve(const char *filename, const char *argv[], const char *envp[]);
int chdir(const char *dirname);
int mount(char *dev_name, char *dir_name, char *type);
// int stat(char *filename, struct s_stat *statbuf);
off_t lseek(int fd, off_t offset, int whence);
pid_t getpid();
int alarm(unsigned int seconds);
int pause();
int mkdir(const char *dirname, int mode);
int dup(unsigned int oldfd);
int chroot(const char *dirname);
pid_t getppid();
// int getdents(unsigned int fd, struct linux_dirent *dirent, unsigned int count);

#endif