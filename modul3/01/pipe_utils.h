#ifndef PIPE_UTILS_H
#define PIPE_UTILS_H

#define MODE_UNNAMED 0
#define MODE_NAMED 1

typedef struct {
    int fd_read;
    int fd_write;
    int mode;
    char name_to_parent[256];  // Имя FIFO для передачи от child к parent
    char name_to_child[256];   // Имя FIFO для передачи от parent к child
    
    // Для неименованных каналов нужны два pipe
    int p2c[2]; // parent to child
    int c2p[2]; // child to parent
} PipeChannel;

void init_channel(PipeChannel *ch, const char *name);
void open_channel_parent(PipeChannel *ch);
void open_channel_child(PipeChannel *ch);
void close_channel(PipeChannel *ch);
void cleanup_named_pipe(PipeChannel *ch);

#endif