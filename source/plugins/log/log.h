#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "../../ws.h"

#define LOG(x,...) do { printf(x, ##__VA_ARGS__); printf("\n"); } while(0)
#define LOGn(x,...) do { printf(x, ##__VA_ARGS__); } while(0) //Log without newline

#endif //LOG_H
