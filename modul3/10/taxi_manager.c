#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define MAX_DRIVERS 64
#define MAX_CMD_LEN 256
#define MAX_MSG_LEN 128
#define MAX_EVENTS 10

typedef struct driver_info {
    pid_t pid;
    int pipe_fd;
    int write_fd;
    char status[32];
    int busy;
    int task_timer;
    time_t start_time;
} driver_info_t;

driver_info_t drivers[MAX_DRIVERS];
int drivers_count = 0;
int epoll_fd = -1;
int timer_fd = -1;

void signal_handler(int sig) {
    (void)sig;
    printf("\nShutting down taxi manager...\n");
    for (int i = 0; i < drivers_count; i++) {
        if (drivers[i].pid > 0) kill(drivers[i].pid, SIGTERM);
        if (drivers[i].write_fd > 0) close(drivers[i].write_fd);
        if (drivers[i].pipe_fd > 0) close(drivers[i].pipe_fd);
    }
    if (epoll_fd > 0) close(epoll_fd);
    if (timer_fd > 0) close(timer_fd);
    exit(0);
}

void sync_driver_state(int idx) {
    if (idx < 0 || idx >= drivers_count || drivers[idx].pid <= 0) return;
    
    if (drivers[idx].busy) {
        time_t now = time(NULL);
        int elapsed = (int)(now - drivers[idx].start_time);
        int remaining = drivers[idx].task_timer - elapsed;
        
        if (remaining <= 0) {
            drivers[idx].busy = 0;
            drivers[idx].task_timer = 0;
            drivers[idx].start_time = 0;
            strcpy(drivers[idx].status, "Available");
        }
    }
}

void create_driver() {
    if (drivers_count >= MAX_DRIVERS) {
        printf("Error: Maximum number of drivers reached (%d)\n", MAX_DRIVERS);
        return;
    }

    int pipe_to_driver[2];
    int pipe_from_driver[2];

    if (pipe(pipe_to_driver) < 0 || pipe(pipe_from_driver) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        close(pipe_to_driver[1]);
        close(pipe_from_driver[0]);
        dup2(pipe_to_driver[0], STDIN_FILENO);
        dup2(pipe_from_driver[1], STDOUT_FILENO);
        close(pipe_to_driver[0]);
        close(pipe_from_driver[1]);
        
        execlp("./taxi_driver", "taxi_driver", NULL);
        perror("execlp");
        exit(1);
    }

    close(pipe_to_driver[0]);
    close(pipe_from_driver[1]);

    drivers[drivers_count].pid = pid;
    drivers[drivers_count].write_fd = pipe_to_driver[1];
    drivers[drivers_count].pipe_fd = pipe_from_driver[0];
    strcpy(drivers[drivers_count].status, "Available");
    drivers[drivers_count].busy = 0;
    drivers[drivers_count].task_timer = 0;
    drivers[drivers_count].start_time = 0;

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = pipe_from_driver[0];
    ev.data.u32 = drivers_count;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_from_driver[0], &ev) < 0) {
        perror("epoll_ctl add driver pipe");
        close(pipe_to_driver[1]);
        close(pipe_from_driver[0]);
        return;
    }

    printf("Driver created with PID: %d\n", pid);
    drivers_count++;
}

void send_task(pid_t pid, int task_timer) {
    int idx = -1;
    for (int i = 0; i < drivers_count; i++) {
        if (drivers[i].pid == pid) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("Error: Driver with PID %d not found\n", pid);
        return;
    }

    sync_driver_state(idx);

    if (drivers[idx].busy) {
        time_t now = time(NULL);
        int elapsed = (int)(now - drivers[idx].start_time);
        int remaining = drivers[idx].task_timer - elapsed;
        if (remaining > 0) {
            printf("Busy %d\n", remaining);
        } else {
            drivers[idx].busy = 0;
            drivers[idx].task_timer = 0;
            drivers[idx].start_time = 0;
            strcpy(drivers[idx].status, "Available");
        }
    }

    if (!drivers[idx].busy) {
        char msg[MAX_MSG_LEN];
        snprintf(msg, sizeof(msg), "TASK %d", task_timer);
        
        ssize_t w = write(drivers[idx].write_fd, msg, strlen(msg) + 1);
        (void)w;

        drivers[idx].busy = 1;
        drivers[idx].task_timer = task_timer;
        drivers[idx].start_time = time(NULL);
        snprintf(drivers[idx].status, sizeof(drivers[idx].status), "Busy %d", task_timer);

        printf("Task sent to driver %d (time: %d sec)\n", pid, task_timer);
    }
}

void get_status(pid_t pid) {
    int idx = -1;
    for (int i = 0; i < drivers_count; i++) {
        if (drivers[i].pid == pid) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("Error: Driver with PID %d not found\n", pid);
        return;
    }

    sync_driver_state(idx);

    if (drivers[idx].busy) {
        time_t now = time(NULL);
        int elapsed = (int)(now - drivers[idx].start_time);
        int remaining = drivers[idx].task_timer - elapsed;
        if (remaining > 0) {
            printf("Status: Busy %d\n", remaining);
        } else {
            drivers[idx].busy = 0;
            drivers[idx].task_timer = 0;
            drivers[idx].start_time = 0;
            strcpy(drivers[idx].status, "Available");
            printf("Status: Available\n");
        }
    } else {
        printf("Status: Available\n");
    }
}

void get_drivers() {
    if (drivers_count == 0) {
        printf("No drivers created\n");
        return;
    }

    printf("\n=== Drivers List ===\n");
    printf("PID\t\tStatus\n");
    printf("-------------------------------\n");
    
    for (int i = 0; i < drivers_count; i++) {
        sync_driver_state(i);
        
        if (drivers[i].busy) {
            time_t now = time(NULL);
            int elapsed = (int)(now - drivers[i].start_time);
            int remaining = drivers[i].task_timer - elapsed;
            if (remaining > 0) {
                printf("%d\t\tBusy %d\n", drivers[i].pid, remaining);
            } else {
                printf("%d\t\tAvailable\n", drivers[i].pid);
            }
        } else {
            printf("%d\t\tAvailable\n", drivers[i].pid);
        }
    }
    printf("================================\n");
}

void handle_driver_message(int driver_idx) {
    char buffer[MAX_MSG_LEN];
    ssize_t n = read(drivers[driver_idx].pipe_fd, buffer, sizeof(buffer) - 1);
    
    if (n <= 0) {
        if (n == 0) {
            printf("Driver %d disconnected\n", drivers[driver_idx].pid);
        } else {
            perror("read from driver");
        }
        
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, drivers[driver_idx].pipe_fd, NULL);
        close(drivers[driver_idx].pipe_fd);
        close(drivers[driver_idx].write_fd);
        drivers[driver_idx].pid = 0;
        drivers[driver_idx].pipe_fd = -1;
        drivers[driver_idx].write_fd = -1;
        drivers[driver_idx].busy = 0;
        strcpy(drivers[driver_idx].status, "Disabled");
        return;
    }

    buffer[n] = '\0';
    buffer[strcspn(buffer, "\r\n")] = '\0';
    
    if (strcmp(buffer, "DONE") == 0) {
        drivers[driver_idx].busy = 0;
        drivers[driver_idx].task_timer = 0;
        drivers[driver_idx].start_time = 0;
        strcpy(drivers[driver_idx].status, "Available");
        printf("Driver %d: Task completed\n", drivers[driver_idx].pid);
    } else if (strncmp(buffer, "STATUS", 6) == 0) {
        char status[32];
        int time_left = 0;
        if (sscanf(buffer, "STATUS %31s %d", status, &time_left) >= 1) {
            sync_driver_state(driver_idx);
            if (drivers[driver_idx].busy && time_left > 0) {
                drivers[driver_idx].task_timer = time_left;
                drivers[driver_idx].start_time = time(NULL) - (drivers[driver_idx].task_timer - time_left);
                snprintf(drivers[driver_idx].status, sizeof(drivers[driver_idx].status), "Busy %d", time_left);
            }
        }
    }
}

void handle_command(char *cmd) {
    cmd[strcspn(cmd, "\r\n")] = '\0';
    if (strlen(cmd) == 0) return;

    char *args[4];
    int argc = 0;
    char *token = strtok(cmd, " ");
    while (token && argc < 4) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return;

    if (strcmp(args[0], "create_driver") == 0) {
        create_driver();
    } else if (strcmp(args[0], "send_task") == 0) {
        if (argc != 3) {
            printf("Usage: send_task <pid> <task_timer>\n");
            return;
        }
        send_task(atoi(args[1]), atoi(args[2]));
    } else if (strcmp(args[0], "get_status") == 0) {
        if (argc != 2) {
            printf("Usage: get_status <pid>\n");
            return;
        }
        get_status(atoi(args[1]));
    } else if (strcmp(args[0], "get_drivers") == 0) {
        get_drivers();
    } else if (strcmp(args[0], "quit") == 0 || strcmp(args[0], "exit") == 0) {
        signal_handler(SIGINT);
    } else {
        printf("Unknown command. Available:\n");
        printf("  create_driver\n");
        printf("  send_task <pid> <task_timer>\n");
        printf("  get_status <pid>\n");
        printf("  get_drivers\n");
        printf("  quit\n");
    }
}

void setup_timer() {
    timer_fd = timerfd_create(CLOCK_REALTIME, 0);
    if (timer_fd < 0) {
        perror("timerfd_create");
        return;
    }

    struct itimerspec its;
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 1;
    its.it_interval.tv_nsec = 0;

    if (timerfd_settime(timer_fd, 0, &its, NULL) < 0) {
        perror("timerfd_settime");
        close(timer_fd);
        return;
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = timer_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev) < 0) {
        perror("epoll_ctl add timer");
        close(timer_fd);
    }
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGCHLD, SIG_IGN);

    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
        perror("epoll_ctl add stdin");
        close(epoll_fd);
        exit(1);
    }

    setup_timer();

    printf("\n=== Taxi Manager ===\n");
    printf("Available commands:\n");
    printf("  create_driver\n");
    printf("  send_task <pid> <task_timer>\n");
    printf("  get_status <pid>\n");
    printf("  get_drivers\n");
    printf("  quit\n\n> ");
    fflush(stdout);

    struct epoll_event events[MAX_EVENTS];
    char cmd[MAX_CMD_LEN];

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            if (fd == STDIN_FILENO) {
                if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
                    printf("\nEOF detected\n");
                    signal_handler(SIGINT);
                }
                handle_command(cmd);
                printf("> ");
                fflush(stdout);
            } else if (fd == timer_fd) {
                uint64_t expirations;
                ssize_t r = read(timer_fd, &expirations, sizeof(expirations));
                (void)r;
                
                for (int j = 0; j < drivers_count; j++) {
                    sync_driver_state(j);
                }
            } else {
                for (int j = 0; j < drivers_count; j++) {
                    if (drivers[j].pipe_fd == fd) {
                        handle_driver_message(j);
                        break;
                    }
                }
            }
        }
    }

    close(epoll_fd);
    return 0;
}