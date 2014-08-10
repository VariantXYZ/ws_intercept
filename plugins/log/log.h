#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "../../ws.h"

#define LOG(x,...) do { if(setupFlag) printf(x, ##__VA_ARGS__); printf("\n"); } while(false)
#define LOGn(x,...) do { if(setupFlag) printf(x, ##__VA_ARGS__); } while(false) //Log without newline

#endif //LOG_H
