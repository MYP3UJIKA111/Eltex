#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>

#define MAX_MSG_LEN 128

volatile sig_atomic_t task_completed = 0;
volatile sig_atomic_t busy = 0;
int task_duration = 0;

void alarm_handler(int sig) {
    (void)sig;
    task_completed = 1;
    busy = 0;
}

void term_handler(int sig) {
    (void)sig;
    fprintf(stderr, "Driver %d: Terminating\n", getpid());
    exit(0);
}

void safe_write(int fd, const char *msg) {
    if (!msg || fd < 0) return;
    size_t len = strlen(msg);
    ssize_t written = 0;
    while (written < (ssize_t)len) {
        ssize_t result = write(fd, msg + written, len - written);
        if (result < 0) {
            if (errno == EINTR) continue;
            perror("write");
            break;
        }
        written += result;
    }
}

int main() {
    signal(SIGALRM, alarm_handler);
    signal(SIGTERM, term_handler);
    signal(SIGINT, term_handler);

    pid_t my_pid = getpid();
    fprintf(stderr, "Driver %d: Started\n", my_pid);
    fflush(stderr);

    char buffer[MAX_MSG_LEN];

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &tv);
        
        if (ret < 0) {
            if (errno == EINTR) {
                if (task_completed) {
                    safe_write(STDOUT_FILENO, "DONE");
                    task_completed = 0;
                }
                continue;
            }
            perror("select");
            break;
        }

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) {
            int n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (n <= 0) {
                fprintf(stderr, "Driver %d: Parent closed pipe\n", my_pid);
                break;
            }
            buffer[n] = '\0';
            buffer[strcspn(buffer, "\r\n")] = '\0';

            if (strncmp(buffer, "TASK", 4) == 0) {
                if (busy) {
                    struct itimerval timer;
                    int remaining = task_duration;
                    if (getitimer(ITIMER_REAL, &timer) == 0) {
                        remaining = timer.it_value.tv_sec + (timer.it_value.tv_usec > 0 ? 1 : 0);
                    }
                    char msg[MAX_MSG_LEN];
                    snprintf(msg, sizeof(msg), "Busy %d", remaining);
                    safe_write(STDOUT_FILENO, msg);
                } else {
                    int duration;
                    if (sscanf(buffer, "TASK %d", &duration) == 1) {
                        busy = 1;
                        task_completed = 0;
                        task_duration = duration;

                        struct itimerval timer;
                        timer.it_value.tv_sec = duration;
                        timer.it_value.tv_usec = 0;
                        timer.it_interval.tv_sec = 0;
                        timer.it_interval.tv_usec = 0;
                        setitimer(ITIMER_REAL, &timer, NULL);

                        char msg[MAX_MSG_LEN];
                        snprintf(msg, sizeof(msg), "STATUS Busy %d", duration);
                        safe_write(STDOUT_FILENO, msg);
                    }
                }
            } else if (strcmp(buffer, "STATUS") == 0) {
                char msg[MAX_MSG_LEN];
                if (busy) {
                    struct itimerval timer;
                    if (getitimer(ITIMER_REAL, &timer) == 0) {
                        int remaining = timer.it_value.tv_sec + (timer.it_value.tv_usec > 0 ? 1 : 0);
                        snprintf(msg, sizeof(msg), "STATUS Busy %d", remaining);
                    } else {
                        snprintf(msg, sizeof(msg), "STATUS Busy");
                    }
                } else {
                    snprintf(msg, sizeof(msg), "STATUS Available");
                }
                safe_write(STDOUT_FILENO, msg);
            } else {
                fprintf(stderr, "Driver %d: Unknown command: %s\n", my_pid, buffer);
                fflush(stderr);
            }
        }
        
        if (task_completed && !busy) {
            safe_write(STDOUT_FILENO, "DONE");
            task_completed = 0;
        }
    }

    return 0;
}