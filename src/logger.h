#pragma once

void log_info(const char* s);
void log_infoc(const char* s);
void log_warning(const char* format);
void log_internal_warn(char* filename, const char* function, int line, char* format);
void log_internal_error(char* filename, const char* function, int line, char* format);
void log_internal_errno(char* filename, const char* function, int line);
