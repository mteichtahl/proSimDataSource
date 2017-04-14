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
#include "PoKeysLib.h"

#define BUFFER_LEN 8192

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

uv_loop_t *loop;
uv_buf_t read_buffer; // TCP read buffer
uv_signal_t sigterm;  // SIGTERM handle
uv_signal_t sigint;   // SIGINT handle

void on_connect(uv_connect_t *req, int status);
void on_read(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf);
static void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf);
void on_close(uv_handle_t *handle);
static void on_signal(uv_signal_t *handle, int signum);

static void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
    *buf = read_buffer;

    //printf("A len: %d -> 0x%x\n", buf->len, buf->base);
}

void on_connect(uv_connect_t *req, int status)
{
    if (uv_is_readable(req->handle))
    {
        uv_read_start(req->handle, alloc_buffer, on_read);
    }
    else
    {
        printf("not readable\n");
    }
}

void on_close(uv_handle_t *handle)
{
    if (!loop->active_handles)
    {
        uv_stop(&loop);
    }
}

void processToken(int index,char *token)
{
 
     printf(" processToken %d %s\n",index, token);

 

   
}

void processData(char *data, int len)
{

    printf("Processing %d bytes\n",len);
    
    char *tempToken = malloc(len);
    memcpy(tempToken,data,len);

   // printf(" - %s\n",tempToken+10);

    // for(int i=0;i<len;i++){
    //     printf("%02X\n",((unsigned char *)tempToken[i]));
    //     if ( ((unsigned char *)tempToken[i])==0x0D && ((unsigned char *)tempToken[i+1])==0x0A ){
    //         printf("----");

    //     }
    // }

    char *token;
    token = strtok(tempToken, "\r\n");
    int i = 0;
    /* walk through other tokens */
    while (token != NULL)
    {
       
        //printf("%d %s\n", i, token);
        processToken(i,token);
        token = strtok(NULL, "\r\n");
        i++;
    }
}

// Signal handling -- just stop the main loop
static void on_signal(uv_signal_t *handle, int signum)
{
    uv_stop(&loop);
}

void on_read(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf)
{
    printf("R len: %d -> 0x%x\n", nread, buf->base);
    if (nread > 0)
    {
        uv_buf_t buffer = uv_buf_init(malloc(nread), nread);
        memcpy(buffer.base, buf->base, nread);

        //printf("read: %d - %s\n", nread, buffer.base);
        processData(buffer.base,nread);
    }
    else if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            printf("STOPPING LOOP\n");
            uv_stop(&loop);
        }
        else
        {
            printf("%s\n", uv_strerror(nread));
            uv_close((uv_handle_t *)server, on_close);
        }
    }
}

int main()
{
    uv_tcp_t client;
    uv_connect_t connect_req;
    struct sockaddr_in req_addr;

    check_uv(uv_loop_init(&loop));

    // check_uv(uv_signal_init(&loop, &sigterm));
    // check_uv(uv_signal_start(&sigterm, on_signal, SIGTERM));
    // check_uv(uv_signal_init(&loop, &sigint));
    // check_uv(uv_signal_start(&sigint, on_signal, SIGINT));

    // Buffer allocation for TCP reading
    char *buffer;
    if (!(buffer = malloc(BUFFER_LEN)))
    {
        memory_error("Unable to allocate buffer of size %d", BUFFER_LEN);
    }
    read_buffer = uv_buf_init(buffer, BUFFER_LEN);

    printf("Starting\n");

    loop = uv_default_loop();

    check_uv(uv_tcp_init(loop, &client));
    uv_tcp_keepalive(&client, 1, 60);
    uv_ip4_addr("192.168.2.2", 8091, &req_addr);

    int rv = uv_tcp_connect(&connect_req, &client, (struct sockaddr *)&req_addr, on_connect);

    if (rv != 0)
    {
        printf("Error");
        return 1;
    }

    if (read_buffer.base)
        free(read_buffer.base);

    check_uv(uv_run(loop, UV_RUN_DEFAULT));
    return 0;
}