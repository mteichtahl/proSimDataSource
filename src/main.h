#define DEFAULT_SIM_PORT 8091
#define BUFFER_LEN 8192
#define MAX_ELEMENTS_PER_UPDATE 512
#define GAUGE_IDENTIFIER 'G'
#define NUMBER_IDENTIFIER 'N'
#define INDICATOR_IDENTIFIER 'I'
#define VALUE_IDENTIFIER 'V'
#define ANALOG_IDENTIFIER 'A'
#define ROTARY_IDENTIFIER 'R'
#define BOOLEAN_IDENTIFIER 'B'

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

unsigned long int elementsProcessed;
pthread_rwlock_t elementLock;
uv_tcp_t client;
uv_connect_t connect_req;

typedef struct simElements
{
    char id[64]; /* key */
    char value[128];
    char previousValue[128];
    char type[32];
    UT_hash_handle hh; /* makes this structure hashable */
} simElements;

extern int initSimConnection(char *IPAddress, int port);
extern void startSimLoop();
extern void stopSimLoop();


void on_connect(uv_connect_t *req, int status);
void on_read(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf);
inline static void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf);
void on_close(uv_handle_t *handle);
static void on_signal(uv_signal_t *handle, int signum);
char *getElementDataType(char identifier);
void addElement(char *id, char *value, char *type);