#include "elements.h"
sim_elements_t *elements = NULL; /* important! initialize to NULL */

sim_elements_t *element_find(char *id)
{
    sim_elements_t *s = NULL;

    if (pthread_rwlock_rdlock(&elementLock) != 0)
    {
        zlog_info(simLogHandler, "can't get rdlock");
    }
    else
    {
        HASH_FIND_STR(elements, id, s); /* id already in the hash? */
        pthread_rwlock_unlock(&elementLock);
    }

    return s;
}

void element_add(char *id, char *value, char *type)
{
    sim_elements_t *s = element_find(id);

    if (s == NULL)
    {
        s = (sim_elements_t*)malloc(sizeof(sim_elements_t));
        strcpy(s->id, id);
        strcpy(s->previousValue, "0");
        strcpy(s->type, type);
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
    // zlog_info(simLogHandler, "%s %s %s %s", s->id, s->value, s->previousValue, s->type);
}

char *element_get_data_type(char identifier)
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

void element_process(int index, char *element)
{
    char *name = strtok(element, "=");
    char *value = strtok(NULL, "=");

    if (value == NULL)
        return;

    char *type = element_get_data_type(name[0]);

    if (type != NULL)
        element_add(name, value, type);

    dataSourceStats->elementsProcessed = elementsProcessed++;
}

void element_stats_time_callback(uv_async_t *handle)
{
    unsigned long runtime = statsCallbackCounter * (STATUS_REPORT_FREQUENCY / 1000);
    zlog_info(simLogHandler, 
              "%d elements / %d updates / %2.2f elements/sec", 
              HASH_COUNT(elements), 
              dataSourceStats->elementsProcessed, 
              (float)(dataSourceStats->elementsProcessed / runtime));

    statsCallbackCounter++;
}

void *element_child_thread(void *data)
{
    uv_loop_t *thread_loop = (uv_loop_t *)data;
    zlog_info(simLogHandler, "Starting simulator event loop");

    // start this loop
    uv_run(thread_loop, UV_RUN_DEFAULT);
    pthread_exit(NULL);
}

void element_timer_callback(uv_timer_t *handle)
{
    uv_async_t *other_thread_notifier = (uv_async_t *)handle->data;
    // notify the other thread
    uv_async_send(other_thread_notifier);
}

void element_sim_start_stats_loop()
{
    pthread_t thread;
    uv_async_t async;
    dataSourceStats = malloc(sizeof(datasource_stats_t));

    statsLoop = uv_loop_new();
    uv_async_init(statsLoop, &async, element_stats_time_callback);
    pthread_create(&thread, NULL, element_child_thread, statsLoop);

    uv_loop_t *main_loop = uv_default_loop();
    uv_timer_t reportingLoopTimer;
    uv_timer_init(main_loop, &reportingLoopTimer);
    reportingLoopTimer.data = &async;

    int ret = uv_timer_start(&reportingLoopTimer, &element_timer_callback, 1000, STATUS_REPORT_FREQUENCY);

    uv_run(main_loop, UV_RUN_DEFAULT);
}
