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
#include "uthash.h"
#include "main.h"

struct simElements *elements = NULL; /* important! initialize to NULL */

simElements *findElement(char *id)
{
    struct simElements *s;

    if (pthread_rwlock_rdlock(&elementLock) != 0)
        printf("can't get rdlock");

    HASH_FIND_STR(elements, id, s); /* id already in the hash? */
    pthread_rwlock_unlock(&elementLock);
    return s;
}

void addElement(char *id, char *value, char *type)
{
    struct simElements *s = findElement(id);

    if (s == NULL)
    {
        s = (simElements *)malloc(sizeof(simElements));
        strcpy(s->id, id);
        strcpy(s->previousValue, "0");
        strcpy(s->type, type);
        if (pthread_rwlock_wrlock(&elementLock) != 0)
            printf("can't get wrlock");
        HASH_ADD_STR(elements, id, s); /* id: name of key field */
        pthread_rwlock_unlock(&elementLock);
    }
    else
    {
        strcpy(s->previousValue, s->value);
    }
    strcpy(s->value, value);
    printf("%s %s %s %s\n", s->id, s->value, s->previousValue, s->type);
}

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
       stopSimLoop();
    }
}

// Signal handling -- just stop the main loop
static void on_signal(uv_signal_t *handle, int signum)
{

    if (!simLoop->active_handles)
    {
       stopSimLoop();
    }
}

char *getElementDataType(char identifier)
{

    switch (identifier)
    {
    case GAUGE_IDENTIFIER:
        return "float";
        break;
    case NUMBER_IDENTIFIER:
        return "float";
        break;
    case INDICATOR_IDENTIFIER:
        return "bool";
        break;
    case VALUE_IDENTIFIER:
        return "uint";
        break;
    case ANALOG_IDENTIFIER:
        return "char";
        break;
    case ROTARY_IDENTIFIER:
        return "char";
        break;
    case BOOLEAN_IDENTIFIER:
        return "bool";
        break;
    default:
        printf("oops");
    }

    return "float";
}

void processElement(int index, char *element)
{

    char *name = strtok(element, "=");
    char *value = strtok(NULL, "=");

    if (value == NULL)
        return;

    char *type = getElementDataType(name[0]);

    addElement(name, value, type);

    elementsProcessed++;
}

void processData(char *data, int len)
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
        processElement(i, array[i]);

    //free(data);
}

void on_read(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf)
{
    // printf("R len: %d -> 0x%x\n", nread, buf->base);

    if (nread > 0)
    {
        uv_buf_t buffer = uv_buf_init((char *)malloc(nread), nread);
        memcpy(buffer.base, buf->base, nread);
        processData(buffer.base, nread);
    }
    else if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            printf("STOPPING LOOP\n");
            stopSimLoop();
        }
        else
        {
            printf("%s\n", uv_strerror(nread));
            uv_close((uv_handle_t *)server, on_close);
        }
    }
}

extern void startSimLoop()
{
    check_uv(uv_run(simLoop, UV_RUN_DEFAULT));
}

extern void stopSimLoop()
{
    printf("Stopping\n");
    uv_stop(simLoop);
}

extern int initSimConnection(char *ipAddress, int port)
{

    struct sockaddr_in req_addr;

    printf("Initialising Sim connection\n");

    if (pthread_rwlock_init(&elementLock, NULL) != 0)
        printf("can't create rwlock");

    check_uv(uv_loop_init(&simLoop));
    check_uv(uv_signal_init(&simLoop, &sigterm));
    check_uv(uv_signal_start(&sigterm, on_signal, SIGTERM));

    //Buffer allocation for TCP reading
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
        printf("Error");
        return 1;
    }
    return 0;
}

int main()
{

    initSimConnection("192.168.2.2", 8091);
    startSimLoop();

    // if (read_buffer.base)
    //     free(read_buffer.base);

    // struct simElements *currentElement, *tmp;

    // HASH_ITER(hh, elements, currentElement, tmp)
    // {
    //     HASH_DEL(elements, currentElement); /* delete it (users advances to next) */
    //     free(currentElement);               /* free it */
    // }
    return 0;
}
