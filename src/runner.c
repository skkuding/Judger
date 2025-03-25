#define _GNU_SOURCE
#define _POSIX_SOURCE

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "child.h"
#include "killer.h"
#include "logger.h"
#include "runner.h"

extern char MEMORY_PEAK_FILE_PATH[256], MEMORY_EVENTS_FILE_PATH[256];

static int check_oom_kill_occurred() {
    // read MEMORY_EVENTS_FILE_PATH to check oom_kill is greater than 0
    // file content is like "oom_kill 1"
    FILE *memory_events_fp = fopen(MEMORY_EVENTS_FILE_PATH, "r");
    if (memory_events_fp != NULL) {
        char event_name[256];
        int oom_kill;
        while (fscanf(memory_events_fp, "%s %d", event_name, &oom_kill) !=
               EOF) {
            if (strcmp(event_name, "oom_kill") == 0 && oom_kill > 0) {
                fclose(memory_events_fp);
                return 1;
            }
        }
        fclose(memory_events_fp);
    }
    return 0;
}

void init_result(struct result *_result) {
    _result->result = _result->error = SUCCESS;
    _result->cpu_time = _result->real_time = _result->signal =
        _result->exit_code = 0;
    _result->memory = 0;
}

void run(struct config *_config, struct result *_result) {
    // init log fp
    FILE *log_fp = log_open(_config->log_path);

    // init result
    init_result(_result);

    // check whether current user is root
    uid_t uid = getuid();
    if (uid != 0) {
        ERROR_EXIT(ROOT_REQUIRED);
    }

    // check args
    if ((_config->max_cpu_time < 1 && _config->max_cpu_time != UNLIMITED) ||
        (_config->max_real_time < 1 && _config->max_real_time != UNLIMITED) ||
        (_config->max_stack < 1) ||
        (_config->max_memory < 1 && _config->max_memory != UNLIMITED) ||
        (_config->max_process_number < 1 &&
         _config->max_process_number != UNLIMITED) ||
        (_config->max_output_size < 1 &&
         _config->max_output_size != UNLIMITED)) {
        ERROR_EXIT(INVALID_CONFIG);
    }

    // record current time
    struct timeval start, end;
    gettimeofday(&start, NULL);

    pid_t child_pid = fork();

    // pid < 0 shows clone failed
    if (child_pid < 0) {
        ERROR_EXIT(FORK_FAILED);
    } else if (child_pid == 0) {
        child_process(log_fp, _config);
    } else if (child_pid > 0) {
        // create new thread to monitor process running time
        pthread_t tid = 0;
        if (_config->max_real_time != UNLIMITED) {
            struct timeout_killer_args killer_args;

            killer_args.timeout = _config->max_real_time;
            killer_args.pid = child_pid;
            if (pthread_create(&tid, NULL, timeout_killer,
                               (void *)(&killer_args)) != 0) {
                kill_pid(child_pid);
                ERROR_EXIT(PTHREAD_FAILED);
            }
        }

        int status;
        struct rusage resource_usage;

        // wait for child process to terminate
        // on success, returns the process ID of the child whose state has
        // changed; On error, -1 is returned.
        if (wait4(child_pid, &status, WSTOPPED, &resource_usage) == -1) {
            kill_pid(child_pid);
            ERROR_EXIT(WAIT_FAILED);
        }
        // get end time
        gettimeofday(&end, NULL);
        _result->real_time = (int)(end.tv_sec * 1000 + end.tv_usec / 1000 -
                                   start.tv_sec * 1000 - start.tv_usec / 1000);

        // process exited, we may need to cancel timeout killer thread
        if (_config->max_real_time != UNLIMITED) {
            if (pthread_cancel(tid) != 0) {
                // todo logging
            };
        }

        if (WIFSIGNALED(status) != 0) {
            _result->signal = WTERMSIG(status);
        }

        if (_result->signal == SIGUSR1) {
            _result->result = SYSTEM_ERROR;
        } else {
            _result->exit_code = WEXITSTATUS(status);
            _result->cpu_time = (int)(resource_usage.ru_utime.tv_sec * 1000 +
                                      resource_usage.ru_utime.tv_usec / 1000);

            // read MEMORY_PEAK_FILE_PATH to get memory usage
            FILE *memory_peak_fp = fopen(MEMORY_PEAK_FILE_PATH, "r");
            if (memory_peak_fp != NULL) {
                fscanf(memory_peak_fp, "%ld", &_result->memory);
                fclose(memory_peak_fp);
            }

            if (_result->exit_code != 0) {
                _result->result = RUNTIME_ERROR;
            }

            if (_result->signal == SIGSEGV) {
                if (_config->max_memory != UNLIMITED &&
                    check_oom_kill_occurred()) {
                    _result->result = MEMORY_LIMIT_EXCEEDED;
                } else {
                    _result->result = RUNTIME_ERROR;
                }
            } else {
                if (_result->signal != 0) {
                    _result->result = RUNTIME_ERROR;
                }
                if (_config->max_memory != UNLIMITED &&
                    check_oom_kill_occurred()) {
                    _result->result = MEMORY_LIMIT_EXCEEDED;
                }
                if (_config->max_real_time != UNLIMITED &&
                    _result->real_time > _config->max_real_time) {
                    _result->result = REAL_TIME_LIMIT_EXCEEDED;
                }
                if (_config->max_cpu_time != UNLIMITED &&
                    _result->cpu_time > _config->max_cpu_time) {
                    _result->result = CPU_TIME_LIMIT_EXCEEDED;
                }
            }
        }

        log_close(log_fp);
    }
}
