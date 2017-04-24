
#include "libProSimDataSource.h"
#include "libProSimDataSource_internal.h"
#include "elements/elements.h"

inline static void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
    *buf = read_buffer;
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
    if (!simLoop->active_handles)
    {
        stop_sim_loop();
    }
}

// Signal handling -- just stop the main loop
void on_signal(uv_signal_t *handle, int signum)
{
    if (!simLoop->active_handles)
    {
        stop_sim_loop();
    }
}

void process_data(char *data, int len)
{
    int elementCount = 0;
    char *p = strtok(data, "\n\r");
    char *array[MAX_ELEMENTS_PER_UPDATE];

    while (p != NULL)
    {
        array[elementCount++] = p;
        p = strtok(NULL, "\n\r");
    }

    for (int i = 0; i < elementCount; ++i)
        element_process(i, array[i]);
}

void on_read(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        uv_buf_t buffer = uv_buf_init((char *)malloc(nread), nread);
        memcpy(buffer.base, buf->base, nread);
        process_data(buffer.base, nread);
    }
    else if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            printf("STOPPING LOOP\n");
            stop_sim_loop();
        }
        else
        {
            printf("%s\n", uv_strerror(nread));
            uv_close((uv_handle_t *)server, on_close);
        }
    }
}

void start_sim_loop()
{
    element_sim_start_stats_loop();
    check_uv(uv_run(simLoop, UV_RUN_DEFAULT));
}

void stop_sim_loop()
{
    printf("Stopping\n");
    uv_stop(simLoop);
}

void sim_set_logging_handler(zlog_category_t *handler)
{
    simLogHandler = handler;
}

int get_data_source_shmid()
{
    return dataSourceShmid;
}

int init_sim_connection(char *ipAddress, int port,void *(*onElementUpdate)(void *))
{
    struct sockaddr_in req_addr;
    onElementUpdate(NULL);

    zlog_info(simLogHandler, "Initialising simulator connection");

    if (pthread_rwlock_init(&elementLock, NULL) != 0)
        zlog_info(simLogHandler, "can't create rwlock");

    simLoop = (uv_loop_t*)calloc(1, sizeof(uv_loop_t));

    check_uv(uv_loop_init(simLoop));
    check_uv(uv_signal_init(simLoop, &sigterm));
    check_uv(uv_signal_start(&sigterm, on_signal, SIGTERM));

    // buffer allocation for TCP reading
    char *buffer;
    if (!(buffer = malloc(BUFFER_LEN)))
    {
        memory_error("Unable to allocate buffer of size %d", BUFFER_LEN);
    }

    read_buffer = uv_buf_init(buffer, BUFFER_LEN);

    simLoop = uv_default_loop();

    check_uv(uv_tcp_init(simLoop, &client));
    uv_tcp_keepalive(&client, 1, 60);
    uv_ip4_addr(ipAddress, port, &req_addr);

    if (uv_tcp_connect(&connect_req, &client, (struct sockaddr *)&req_addr, on_connect) != 0)
    {
        zlog_info(simLogHandler, "Error");
        return 1;
    }  
    
    return 0;
}

int main()
{
    return 0;
}
