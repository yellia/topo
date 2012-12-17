#ifndef LOG_H

#define LOG_H
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <errno.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>

using namespace std;

#define MAX_STRING_LEN 1024
#define MAX_BUF_LEN	4096 

#define duker_debug

#ifdef duker_debug
	#define duker_log(format, ...) _duker_log(__FILE__, __LINE__,format, ##__VA_ARGS__) 
#else
	#define duker_log(format, ...)	 
#endif

void _duker_log(const char *filename, const int line,const char* format, ...);

#endif
