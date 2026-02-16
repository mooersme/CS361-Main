#include "orders.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void orders_generate_order_id(char out_order_id[], size_t cap) {
    // Simple: ORD-YYYYMMDDHHMMSS
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    snprintf(out_order_id, cap, "ORD-%04d%02d%02d%02d%02d%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
}

static void iso_timestamp(char out[], size_t cap) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    snprintf(out, cap, "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
}

int orders_append_reservations(const char* path, const char* order_id, const char* event_id,
                               const TicketHolder* holders, int ticket_count) {
    if (!path || !order_id || !event_id || !holders || ticket_count <= 0) return 0;

    FILE* f = fopen(path, "a");
    if (!f) return 0;

    char ts[32];
    iso_timestamp(ts, sizeof(ts));

    for (int i = 0; i < ticket_count; i++) {
        // Sprint 1 assumption: no commas in names/emails
        fprintf(f, "%s,%s,%d,%s,%s,%s\n",
                order_id, event_id, i + 1, holders[i].name, holders[i].email, ts);
    }

    fclose(f);
    return 1;
}
