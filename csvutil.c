#include "csvutil.h"
#include <string.h>

void trim_newline(char* s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

int split_csv_line(char* line, char** out_fields, int max_fields) {
    // Very simple splitter: commas only, no quotes
    int count = 0;
    char* p = line;
    while (p && count < max_fields) {
        out_fields[count++] = p;
        char* comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }
    return count;
}
