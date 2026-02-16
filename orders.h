#ifndef ORDERS_H
#define ORDERS_H

#include "models.h"

// Append one line per ticket to reservations.csv
int orders_append_reservations(const char* path, const char* order_id, const char* event_id,
                               const TicketHolder* holders, int ticket_count);

void orders_generate_order_id(char out_order_id[], size_t cap);

#endif
