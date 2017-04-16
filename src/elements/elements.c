#include "elements.h"

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
