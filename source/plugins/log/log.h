#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "../../ws.h"

#define LOGGING 1 //TODO: Just add a makefile option for this

#ifndef LOGGING

#define LOG(x,...) do { printf(x, ##__VA_ARGS__); printf("\n"); } while(0)
#define LOGn(x,...) do { printf(x, ##__VA_ARGS__); } while(0) //Log without newline

#elif LOGGING == 1

#include <time.h>
FILE *logfile = NULL;

#define LOG(x,y,z) do { fwrite(x,y,z,logfile); fflush(logfile); } while(0)

#endif

#endif //LOG_H
