#ifndef INC_LIBPROSIMDATASOURCE_INTERNAL_H
#define INC_LIBPROSIMDATASOURCE_INTERNAL_H

#include <uv.h>
#include <zlog.h>

#include "libProSimDataSource.h"

/**
 * header containing global definitions internal to this library
 */

#ifdef __cplusplus
extern "C"
{
#endif

// -- private prototypes

void sim_start_stats_loop();

// -- forward decl of private global vars

uv_loop_t *simLoop;
uv_buf_t read_buffer; // TCP read buffer
uv_signal_t sigterm;  // SIGTERM handle
uv_signal_t sigint;   // SIGINT handle
uv_tcp_t client;
uv_connect_t connect_req;

int dataSourceShmid;
key_t key;

zlog_category_t *simLogHandler;
datasource_stats_t *dataSourceStats;

#ifdef __cplusplus
extern "C"
}
#endif

#endif

