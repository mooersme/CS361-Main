#ifndef CSVUTIL_H
#define CSVUTIL_H

#include <stddef.h>

int split_csv_line(char* line, char** out_fields, int max_fields);
// Note: Sprint 1 assumes no quoted commas in fields.
char* trim_ws(char* s);

void trim_newline(char* s);

#endif
