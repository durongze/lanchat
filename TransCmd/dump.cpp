#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "mysys.h"
#include "dump.h"

#define MAX_PRINT_LEN	4096

DYZLogLevel DYZdebuglevel = DYZLOGALL;

static int neednl;

static FILE *fmsg;

void LogDefault(int level, const char *format, va_list vl);

static DYZLogCallback *cb = LogDefault;

static const char *levels[] = {
  "CRIT", 
  "ERROR", 
  "WARNING", 
  "INFO",
  "DEBUG",
  "DEBUG2"
};

void LogDefault(int level, const char *format, va_list vl)
{
	char str[MAX_PRINT_LEN]="";

	vsnprintf(str, MAX_PRINT_LEN-1, format, vl);

	/* Filter out 'no-name' */
	if ( DYZdebuglevel<DYZLOGALL && strstr(str, "no-name" ) != NULL )
		return;

	if ( !fmsg ) fmsg = stderr;

	if ( level <= DYZdebuglevel ) {
		fprintf(fmsg, "%s: %s\n", levels[level], str);
		fflush(fmsg);
	}
}

void DYZLogSetOutput(FILE *file)
{
	fmsg = file;
}

void DYZLogSetLevel(DYZLogLevel level)
{
	DYZdebuglevel = level;
}

void DYZLogSetCallback(DYZLogCallback *cbp)
{
	cb = cbp;
}

DYZLogLevel DYZLogGetLevel()
{
	return DYZdebuglevel;
}

void DYZLog(int level, const char *format, ...)
{
	va_list args;

	if ( level > DYZdebuglevel )
		return;

	va_start(args, format);
	cb(level, format, args);
	va_end(args);
}

static const char hexdig[] = "0123456789abcdef";

void DYZLogHex(int level, const uint8_t *data, unsigned long len)
{
	unsigned long i;
	char line[50], *ptr;

	if ( level > DYZdebuglevel )
		return;

	ptr = line;

	for(i=0; i<len; i++) {
		*ptr++ = hexdig[0x0f & (data[i] >> 4)];
		*ptr++ = hexdig[0x0f & data[i]];
		if ((i & 0x0f) == 0x0f) {
			*ptr = '\0';
			ptr = line;
			DYZLog(level, "%s", line);
		} else {
			*ptr++ = ' ';
		}
	}
	if (i & 0x0f) {
		*ptr = '\0';
		DYZLog(level, "%s", line);
	}
}

void DYZLogHexString(int level, const uint8_t *data, unsigned long len)
{
#define BP_OFFSET 9
#define BP_GRAPH 60
#define BP_LEN	80
	char	line[BP_LEN];
	unsigned long i;

	if ( !data || level > DYZdebuglevel )
		return;

	/* in case len is zero */
	line[0] = '\0';

	for ( i = 0 ; i < len ; i++ ) {
		int n = i % 16;
		unsigned off;

		if( !n ) {
			if( i ) DYZLog( level, "%s", line );
			memset( line, ' ', sizeof(line)-2 );
			line[sizeof(line)-2] = '\0';

			off = i % 0x0ffffU;

			line[2] = hexdig[0x0f & (off >> 12)];
			line[3] = hexdig[0x0f & (off >>  8)];
			line[4] = hexdig[0x0f & (off >>  4)];
			line[5] = hexdig[0x0f & off];
			line[6] = ':';
		}

		off = BP_OFFSET + n*3 + ((n >= 8)?1:0);
		line[off] = hexdig[0x0f & ( data[i] >> 4 )];
		line[off+1] = hexdig[0x0f & data[i]];

		off = BP_GRAPH + n + ((n >= 8)?1:0);

		if ( isprint( data[i] )) {
			line[BP_GRAPH + n] = data[i];
		} else {
			line[BP_GRAPH + n] = '.';
		}
	}

	DYZLog( level, "%s", line );
}

/* These should only be used by apps, never by the library itself */
void DYZLogPrintf(const char *format, ...)
{
	char str[MAX_PRINT_LEN]="";
	memset(str, 0, sizeof(str));

	va_list args;
	va_start(args, format);
	vsnprintf(str, MAX_PRINT_LEN-4, format, args);
	va_end(args);

	if ( DYZdebuglevel==DYZLOGCRIT )
		return;

	if ( !fmsg ) fmsg = stderr;

	if (neednl) {
		putc('\n', fmsg);
		neednl = 0;
	}

	fprintf(fmsg, "%s", str);
	fflush(fmsg);
}

void DYZLogStatus(const char *format, ...)
{
	char str[MAX_PRINT_LEN]="";
	va_list args;
	va_start(args, format);
	vsnprintf(str, MAX_PRINT_LEN-1, format, args);
	va_end(args);

	if ( DYZdebuglevel==DYZLOGCRIT )
		return;

	if ( !fmsg ) fmsg = stderr;

	fprintf(fmsg, "%s", str);
	fflush(fmsg);
	neednl = 1;
}
