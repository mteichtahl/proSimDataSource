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
#include "elements/elements.h"

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

uv_loop_t *simLoop;
uv_buf_t read_buffer; // TCP read buffer
uv_signal_t sigterm;  // SIGTERM handle
uv_signal_t sigint;   // SIGINT handle
uv_tcp_t client;
uv_connect_t connect_req;



extern int initSimConnection(char *IPAddress, int port);
extern void startSimLoop();
extern void stopSimLoop();


void on_connect(uv_connect_t *req, int status);
void on_read(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf);
inline static void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf);
void on_close(uv_handle_t *handle);
static void on_signal(uv_signal_t *handle, int signum);
void processData(char *data, int len);