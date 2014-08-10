#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "ws.h"

//Defined in Makefile
//#define LOGGING 1 //Comment for no log + console, set to 1 for log + console, 2 for no log + no console

#ifndef LOGGING

#define LOG(x,...) do { if(setupFlag) printf(x, ##__VA_ARGS__); printf("\n"); } while(false)
#define LOGn(x,...) do { if(setupFlag) printf(x, ##__VA_ARGS__); } while(false) //Log without newline

#elif LOGGING == 1

static FILE *logfile = fopen("logfile.txt","a+");

#define LOG(x,...) do { if(setupFlag) { fprintf(logfile,x, ##__VA_ARGS__); } fprintf(logfile,"\n"); fflush(logfile); } while(false)
#define LOGn(x,...) do { if(setupFlag) { fprintf(logfile,x, ##__VA_ARGS__); } fflush(logfile); } while(false)

#else

#define LOG(x,...)
#define LOGn(x,...)

#endif

#endif //LOG_H
