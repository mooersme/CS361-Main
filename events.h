#ifndef EVENTS_H
#define EVENTS_H

#include "models.h"
#include <stddef.h>

typedef struct {
    Event events[MAX_EVENTS];
    size_t count;
} EventStore;

int events_load_csv(const char* path, EventStore* store);
const Event* events_find(const EventStore* store, const char* event_id);
Event* events_find_mut(EventStore* store, const char* event_id);

// Persist entire store back to CSV using atomic replace (temp + rename)
int events_save_csv_atomic(const char* path, const EventStore* store);

#endif
