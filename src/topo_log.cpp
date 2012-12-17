#include "topo_log.h"

#define MAX_STRING_LEN	1024


void _duker_log(const char *filename, const int line, const char *format, ...)
{
	cout << flush;

	va_list arg;
	char buf[MAX_STRING_LEN];	
	int len =0;

	len = sprintf(buf, "file:%16s, line:%3d ", filename, line);
		
	va_start(arg,format);

	vsprintf(buf + len,format,arg);

	va_end(arg);
	
	cout << buf << endl;
}
