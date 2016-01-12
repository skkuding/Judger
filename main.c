#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/time.h>


#define __DEBUG__


#ifdef __DEBUG__
#define DEBUG(format, ...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif


#define RUN_SUCCEEDED 0
#define FORK_FAILED -1
#define WAIT4_FAILED -2
#define RUN_FAILED -3


#define SUCCESS 0
#define CPU_TIME_LIMIT_EXCEEDED 1
#define REAL_TIME_LIMIT_EXCEEDED 2
#define MEMORY_LIMIT_EXCEEDED 3
#define RUNTIME_ERROR 4
#define SYSTEM_ERROR 5


struct result {
    int cpu_time;
    long memory;
    int real_time;
    int signal;
    int flag;
    int err;
};


struct config {
    int max_cpu_time;
    int max_memory;
    char path[200];
    char in_file[200];
    char out_file[200];
};


void set_timer(int sec, int ms, int is_cpu_time) {
    struct itimerval time_val;
    time_val.it_interval.tv_sec = time_val.it_interval.tv_usec = 0;
    time_val.it_value.tv_sec = sec;
    time_val.it_value.tv_usec = ms * 1000;
    if (setitimer(is_cpu_time ? ITIMER_VIRTUAL : ITIMER_REAL, &time_val, NULL) == -1) {
        DEBUG("settimer failed\n");
    }
}


int run(struct config *config, struct result *result) {
    int status;
    struct rusage resource_usage;
    struct timeval start, end;
    struct rlimit memory_limit;
    int signal;
    char *argv[] = {config->path, NULL};

    gettimeofday(&start, NULL);

    memory_limit.rlim_cur = memory_limit.rlim_max = (rlim_t) (config->max_memory);

    pid_t pid = fork();

    if (pid < 0) {
        DEBUG("fork failed\n");
        result->flag = SYSTEM_ERROR;
        result->err = FORK_FAILED;
        return RUN_FAILED;
    }

    if (pid > 0) {
        //parent process
        DEBUG("I'm parent process\n");
        if (wait4(pid, &status, 0, &resource_usage) == -1) {
            DEBUG("wait4 failed\n");
            result->flag = SYSTEM_ERROR;
            result->err = WAIT4_FAILED;
            return RUN_FAILED;
        }
        result->cpu_time = (int) (resource_usage.ru_utime.tv_sec * 1000 +
                                  resource_usage.ru_utime.tv_usec / 1000 +
                                  resource_usage.ru_stime.tv_sec * 1000 +
                                  resource_usage.ru_stime.tv_usec / 1000);

        result->memory = resource_usage.ru_maxrss * 1024;
        result->signal = 0;
        result->flag = result->err = SUCCESS;

        if (WIFSIGNALED(status)) {
            signal = WTERMSIG(status);
            DEBUG("signal %d\n", signal);
            result->signal = signal;
            if (signal == SIGALRM) {
                result->flag = REAL_TIME_LIMIT_EXCEEDED;
            }
            else if (signal == SIGVTALRM) {
                result->flag = CPU_TIME_LIMIT_EXCEEDED;
            }
            else if (signal == SIGSEGV) {
                if (result->memory > config->max_memory) {
                    result->flag = MEMORY_LIMIT_EXCEEDED;
                }
                else {
                    result->flag = RUNTIME_ERROR;
                }
            }
            else {
                result->flag = RUNTIME_ERROR;
            }
        }
        gettimeofday(&end, NULL);
        result->real_time = (int) (end.tv_sec * 1000 + end.tv_usec / 1000 - start.tv_sec * 1000 - start.tv_usec / 1000);
        return RUN_SUCCEEDED;
    }
    else {
        //child process
        DEBUG("I'm child process\n");
        if (setrlimit(RLIMIT_AS, &memory_limit) != 0)
            DEBUG("setrlimit failed\n");
        // cpu time
        set_timer(config->max_cpu_time / 1000, config->max_cpu_time % 1000, 1);
        // real time * 3
        set_timer(config->max_cpu_time / 1000 * 3, (config->max_cpu_time % 1000) * 3 % 1000, 0);

        //dup2(fileno(fopen(config->in_file, "r")), 0);
        //dup2(fileno(fopen(config->out_file, "w")), 1);

        execve(config->path, argv, NULL);
        DEBUG("execve failed\n");
        return RUN_FAILED;
    }
}


int main() {
    struct config config;
    struct result result;
    int run_ret;

    config.max_cpu_time = 4300;
    config.max_memory = 150000000;

    strcpy(config.path, "/Users/virusdefender/Desktop/judger/limit");
    strcpy(config.in_file, "/Users/virusdefender/Desktop/judger/in");
    strcpy(config.out_file, "/Users/virusdefender/Desktop/judger/out");

    run_ret = run(&config, &result);

    if (run_ret) {
        DEBUG("run failed\n");
        return RUN_FAILED;
    }

    DEBUG("cpu time %d\nreal time %d\nmemory %ld\nflag %d\nsignal %d\nerr %d",
          result.cpu_time, result.real_time, result.memory, result.flag, result.signal, result.err);

    return 0;
}