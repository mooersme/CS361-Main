#include "events.h"
#include "csvutil.h"
#include <stdio.h>
#include <string.h>

static void safe_copy(char* dst, size_t cap, const char* src) {
    if (!dst || cap == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    snprintf(dst, cap, "%s", src);
}

int events_load_csv(const char* path, EventStore* store) {
    if (!store) return 0;
    store->count = 0;

    FILE* f = fopen(path, "r");
    if (!f) return 0;

    char line[2048];
    // header
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }

    while (fgets(line, sizeof(line), f)) {
        if (store->count >= MAX_EVENTS) break;
        trim_newline(line);

        char* fields[16] = {0};
        int n = split_csv_line(line, fields, 16);
        if (n < 7) continue;

        Event* e = &store->events[store->count++];
        safe_copy(e->id, sizeof(e->id), fields[0]);
        safe_copy(e->name, sizeof(e->name), fields[1]);
        safe_copy(e->datetime, sizeof(e->datetime), fields[2]);
        safe_copy(e->venue, sizeof(e->venue), fields[3]);
        safe_copy(e->category, sizeof(e->category), fields[4]);
        e->available = (int)strtok(fields[5], NULL, 10);
        safe_copy(e->details, sizeof(e->details), fields[6]);
    }

    fclose(f);
    return 1;
}

const Event* events_find(const EventStore* store, const char* event_id) {
    if (!store || !event_id) return NULL;
    for (size_t i = 0; i < store->count; i++) {
        if (strcmp(store->events[i].id, event_id) == 0) return &store->events[i];
    }
    return NULL;
}

Event* events_find_mut(EventStore* store, const char* event_id) {
    if (!store || !event_id) return NULL;
    for (size_t i = 0; i < store->count; i++) {
        if (strcmp(store->events[i].id, event_id) == 0) return &store->events[i];
    }
    return NULL;
}

int events_save_csv_atomic(const char* path, const EventStore* store) {
    if (!path || !store) return 0;

    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);

    FILE* f = fopen(tmp_path, "w");
    if (!f) return 0;

    fprintf(f, "id,name,datetime,venue,category,available,details\n");
    for (size_t i = 0; i < store->count; i++) {
        const Event* e = &store->events[i];
        // Sprint 1 assumption: no commas in fields.
        fprintf(f, "%s,%s,%s,%s,%s,%d,%s\n",
                e->id, e->name, e->datetime, e->venue, e->category, e->available, e->details);
    }

    fclose(f);

    // atomic-ish replace (works well on Unix; on Windows rename rules differ)
    if (remove(path) != 0) {
        // if file didn't exist, that's fine; ignore
    }
    if (rename(tmp_path, path) != 0) {
        remove(tmp_path);
        return 0;
    }
    return 1;
}

