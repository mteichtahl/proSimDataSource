#include "elements.h"
struct simElements *elements = NULL; /* important! initialize to NULL */

simElements *findElement(char *id)
{
    struct simElements *s;

    if (pthread_rwlock_rdlock(&elementLock) != 0)
    {
        zlog_info(simLogHandler, "can't get rdlock");
    }
    else
    {
        HASH_FIND_STR(elements, id, s); /* id already in the hash? */
        pthread_rwlock_unlock(&elementLock);
        return s;
    }
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
        s->onElementUpdateCallback = onUpdate;
        if (pthread_rwlock_wrlock(&elementLock) != 0)
        {
            zlog_info(simLogHandler, "can't get wrlock");
        }
        else
        {
            HASH_ADD_STR(elements, id, s); /* id: name of key field */
        }
        pthread_rwlock_unlock(&elementLock);
    }
    else
    {
        strcpy(s->previousValue, s->value);
    }
    strcpy(s->value, value);
    s->onElementUpdateCallback(s);
    

    // zlog_info(simLogHandler, "%s %s %s %s", s->id, s->value, s->previousValue, s->type);
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
        return NULL;
    }

    return NULL;
}

void processElement(int index, char *element)
{
    char *name = strtok(element, "=");
    char *value = strtok(NULL, "=");

    if (value == NULL)
        return;

    char *type = getElementDataType(name[0]);

    if (type != NULL)
        addElement(name, value, type);

    dataSourceStats->elementsProcessed = elementsProcessed++;
}

void statsTimerCallback(uv_async_t *handle, int status)
{
    unsigned long runtime = statsCallbackCounter * (STATUS_REPORT_FREQUENCY / 1000);
    zlog_info(simLogHandler, "%d elements / %d updates / %2.2f elements/sec", HASH_COUNT(elements), dataSourceStats->elementsProcessed, (float)(dataSourceStats->elementsProcessed / runtime));
    statsCallbackCounter++;
}

void *child_thread(void *data)
{
    uv_loop_t *thread_loop = (uv_loop_t *)data;
    zlog_info(simLogHandler, "Starting simulator event loop");

    //Start this loop
    uv_run(thread_loop, UV_RUN_DEFAULT);
    pthread_exit(NULL);
}

void timer_callback(uv_timer_t *handle, int status)
{
    uv_async_t *other_thread_notifier = (uv_async_t *)handle->data;
    //Notify the other thread
    uv_async_send(other_thread_notifier);
}

void simStartStatsLoop()
{

    pthread_t thread;
    uv_async_t async;
    dataSourceStats = malloc(sizeof(t_stats));

    statsLoop = uv_loop_new();
    uv_async_init(statsLoop, &async, statsTimerCallback);
    pthread_create(&thread, NULL, child_thread, statsLoop);

    uv_loop_t *main_loop = uv_default_loop();
    uv_timer_t reportingLoopTimer;
    uv_timer_init(main_loop, &reportingLoopTimer);
    reportingLoopTimer.data = &async;

    int ret = uv_timer_start(&reportingLoopTimer, timer_callback, 1000, STATUS_REPORT_FREQUENCY);

    uv_run(main_loop, UV_RUN_DEFAULT);
}
