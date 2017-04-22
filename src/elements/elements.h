#ifndef __ELEMENTS_H
#define __ELEMENTS_H

#include <uv.h>
#include "../libProSimDataSource.h"
#include "uthash.h"

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

#define STATUS_REPORT_FREQUENCY     10000


typedef struct simElements
{
    char id[64]; /* key */
    char value[128];
    char previousValue[128];
    char type[32];
    void *(*onElementUpdate)(void *);
    UT_hash_handle hh; /* makes this structure hashable */
} simElements;

unsigned long int elementsProcessed;
unsigned long int statsCallbackCounter;

pthread_rwlock_t elementLock;
uv_loop_t* statsLoop;

char *getElementDataType(char identifier);
simElements *findElement(char *id);
void addElement(char *id, char *value, char *type);
void processElement(int index, char *element);

#endif