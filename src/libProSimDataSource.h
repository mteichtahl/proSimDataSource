#ifndef INC_LIBPROSIMDATASOURCE_H
#define INC_LIBPROSIMDATASOURCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <zlog.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <uv.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define check_uv(status)                                                       \
    do                                                                         \
    {                                                                          \
        int code = (status);                                                   \
        if (code < 0)                                                          \
        {                                                                      \
            fprintf(stderr, "%s: %s\n", uv_err_name(code), uv_strerror(code)); \
            exit(code);                                                        \
        }                                                                      \
    } while (0)

#define memory_error(fmt, ...)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        fprintf(stderr, "%s: %s (%d): not enough memory: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define SHMKEY7 123999

typedef struct
{
     int elementsProcessed;
} datasource_stats_t;

int init_sim_connection(char *ipAddress, int port,void *(*onElementUpdate)(void *));
void sim_set_logging_handler(zlog_category_t* handler);
void start_sim_loop();
void stop_sim_loop();
int get_data_source_shmid();
void on_connect(uv_connect_t *req, int status);
void on_read(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf);
inline static void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf);
void on_close(uv_handle_t *handle);
void on_signal(uv_signal_t *handle, int signum);
void process_data(char *data, int len);

#ifdef __cplusplus
}
#endif

#endif