/* logger.c
 * Authors: Gabriel H. Jadderson
 * Created: 15/01/2019
 */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void log_info(const char* s)
{
	printf(ANSI_COLOR_BLUE "INFO: %s" ANSI_COLOR_RESET "\n", s);
}

void log_infoc(const char* s)
{
	printf(ANSI_COLOR_CYAN "INFO: %s" ANSI_COLOR_RESET "\n", s);
}


void log_warning(const char* format)
{
	printf(ANSI_COLOR_YELLOW "WARNING: %s" ANSI_COLOR_RESET "\n", format);
}

void log_warn(char* format, int line)
{
	printf(ANSI_COLOR_YELLOW "WARNING[%d]: %s" ANSI_COLOR_RESET "\n", line, format);
}

void log_error(char* format, int line)
{
	fprintf(stderr, ANSI_COLOR_RED "-ERROR-[%d]: %s"  ANSI_COLOR_RESET "\n", line, format);
}



void log_internal_warn(char* filename, const char* function, int line, char* format)
{
	printf(ANSI_COLOR_YELLOW "INTERNAL WARNING[%s:%s]:%d -> %s" ANSI_COLOR_RESET "\n", filename, function, line, format);
}

void log_internal_error(char* filename, const char* function, int line, char* format)
{
	printf(ANSI_COLOR_RED "-INTERNAL ERROR-[%s:%s]:%d -> %s"  ANSI_COLOR_RESET "\n", filename, function, line, format);
}

void log_internal_errno(char* filename, const char* function, int line)
{
	printf(ANSI_COLOR_RED "-ERROR-[%s:%s]:%d -> %s"  ANSI_COLOR_RESET "\n", filename, function, line, strerror(errno));
}