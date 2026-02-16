/* Megan Mooers
    CS361
    Main Project
*/

#include "models.h"
#include "events.h"
#include "orders.h"
#include "ui.h"
#include "csvutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define EVENTS_PATH "data/events.csv"
#define RES_PATH    "data/reservations.csv"

static void state_reset_order(AppState* st) {
    if (!st) return;
    st->ticket_count = 0;
    if (st->holders) {
        free(st->holders);
        st->holders = NULL;
    }
    st->order_id[0] = '\0';
}

static void to_upper_str(char* s) {
    for (; s && *s; s++) *s = (char)toupper((unsigned char)*s);
}

static int parse_tokens(char* line, char** toks, int max) {
    int n = 0;
    char* p = line;
    while (*p && n < max) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;
        toks[n++] = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (*p) { *p = '\0'; p++; }
    }
    return n;
}

static int read_line(char* buf, size_t cap) {
    if (!fgets(buf, cap, stdin)) return 0;
    trim_newline(buf);
    return 1;
}

static int prompt_field(const char* label, char* out, size_t cap) {
    printf("%s: ", label);
    fflush(stdout);
    if (!read_line(out, cap)) return 0;
    if (strcmp(out, "X") == 0 || strcmp(out, "x") == 0) return -1;
    return 1;
}

static int collect_ticket_holders(AppState* st) {
    st->holders = (TicketHolder*)calloc((size_t)st->ticket_count, sizeof(TicketHolder));
    if (!st->holders) return 0;

    for (int i = 0; i < st->ticket_count; i++) {
        printf("Ticket %d\n", i + 1);
        int r1 = prompt_field("  Name", st->holders[i].name, sizeof(st->holders[i].name));
        if (r1 == -1) return -1;
        if (r1 == 0) return 0;

        int r2 = prompt_field("  Email", st->holders[i].email, sizeof(st->holders[i].email));
        if (r2 == -1) return -1;
        if (r2 == 0) return 0;

        // Sprint 1: reject commas to keep CSV simple
        if (strchr(st->holders[i].name, ',') || strchr(st->holders[i].email, ',')) {
            puts("Commas are not allowed in this sprint's CSV fields. Try again.");
            i--;
            continue;
        }
    }
    return 1;
}

static void goto_list(AppState* st) {
    st->screen = SCREEN_EVENT_LIST;
    st->selected_event_id[0] = '\0';
    state_reset_order(st);
}

int main(void) {
    EventStore store;
    if (!events_load_csv(EVENTS_PATH, &store)) {
        fprintf(stderr, "Failed to load %s\n", EVENTS_PATH);
        return 1;
    }

    AppState st = {0};
    st.screen = SCREEN_WELCOME;
    st.holders = NULL;

    char line[512];

    while (1) {
        ui_render(&st, &store);

        printf("> ");
        fflush(stdout);
        if (!read_line(line, sizeof(line))) break;
        if (line[0] == '\0') continue;

        // Keep a copy for tokenization
        char buf[512];
        snprintf(buf, sizeof(buf), "%s", line);

        char* toks[8] = {0};
        int nt = parse_tokens(buf, toks, 8);

        // Command keyword uppercased
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "%s", toks[0]);
        to_upper_str(cmd);

        if (strcmp(cmd, "QUIT") == 0 || strcmp(cmd, "EXIT") == 0) {
            break;
        }

        if (strcmp(cmd, "EL") == 0) {
            goto_list(&st);
            continue;
        }

        if (st.screen == SCREEN_WELCOME) {
            if (strcmp(cmd, "EL") == 0) {
                goto_list(&st);
            } else {
                puts("Type EL to view Event List, or QUIT.");
            }
            continue;
        }

        if (strcmp(cmd, "CANCEL") == 0) {
            // per your heuristics: cancel returns to previous safe screen
            if (st.screen == SCREEN_EVENT_DETAILS || st.screen == SCREEN_RESERVE_TICKETS ||
                st.screen == SCREEN_ENTER_USER_DATA || st.screen == SCREEN_ORDER_SUMMARY) {
                goto_list(&st);
            } else {
                goto_list(&st);
            }
            continue;
        }

        if (strcmp(cmd, "DETAILS") == 0) {
            if (nt < 2) { puts("Usage: DETAILS <EVENT_ID>"); continue; }
            const Event* e = events_find(&store, toks[1]);
            if (!e) { puts("Event not found."); continue; }
            snprintf(st.selected_event_id, sizeof(st.selected_event_id), "%s", toks[1]);
            st.screen = SCREEN_EVENT_DETAILS;
            state_reset_order(&st);
            continue;
        }

        if (strcmp(cmd, "RESERVE") == 0) {
            if (st.screen == SCREEN_EVENT_DETAILS) {
                // RESERVE <#>
                if (nt < 2) { puts("Usage: RESERVE <#>"); continue; }
                const Event* e = events_find(&store, st.selected_event_id);
                if (!e) { puts("No event selected."); continue; }
                int n = (int)strtol(toks[1], NULL, 10);
                if (n <= 0) { puts("Ticket count must be > 0."); continue; }
                if (n > e->available) { puts("Not enough tickets available."); continue; }
                st.ticket_count = n;
                st.screen = SCREEN_RESERVE_TICKETS;
                continue;
            } else {
                // RESERVE <EVENT_ID> <#>
                if (nt < 3) { puts("Usage: RESERVE <EVENT_ID> <#>"); continue; }
                const Event* e = events_find(&store, toks[1]);
                if (!e) { puts("Event not found."); continue; }
                int n = (int)strtol(toks[2], NULL, 10);
                if (n <= 0) { puts("Ticket count must be > 0."); continue; }
                if (n > e->available) { puts("Not enough tickets available."); continue; }
                snprintf(st.selected_event_id, sizeof(st.selected_event_id), "%s", toks[1]);
                st.ticket_count = n;
                st.screen = SCREEN_RESERVE_TICKETS;
                continue;
            }
        }

        if (strcmp(cmd, "CONTINUE") == 0) {
            if (st.screen != SCREEN_RESERVE_TICKETS) {
                puts("CONTINUE is only valid after RESERVE.");
                continue;
            }
            st.screen = SCREEN_ENTER_USER_DATA;
            ui_render(&st, &store);

            int r = collect_ticket_holders(&st);
            if (r == -1) {
                puts("Aborted.");
                goto_list(&st);
                continue;
            } else if (r == 0) {
                puts("Input error.");
                goto_list(&st);
                continue;
            }

            orders_generate_order_id(st.order_id, sizeof(st.order_id));
            st.screen = SCREEN_ORDER_SUMMARY;
            continue;
        }

        if (strcmp(cmd, "CONFIRM") == 0) {
            if (st.screen != SCREEN_ORDER_SUMMARY) {
                puts("CONFIRM is only valid on Order Summary.");
                continue;
            }

            Event* e = events_find_mut(&store, st.selected_event_id);
            if (!e) { puts("No event selected."); goto_list(&st); continue; }

            if (st.ticket_count > e->available) {
                puts("Tickets no longer available (state mismatch).");
                goto_list(&st);
                continue;
            }

            // 1) decrement in memory
            e->available -= st.ticket_count;

            // 2) persist events.csv (atomic)
            if (!events_save_csv_atomic(EVENTS_PATH, &store)) {
                puts("Failed to write events.csv. Reservation not completed.");
                // rollback in memory
                e->available += st.ticket_count;
                goto_list(&st);
                continue;
            }

            // 3) append reservations.csv
            if (!orders_append_reservations(RES_PATH, st.order_id, st.selected_event_id,
                                            st.holders, st.ticket_count)) {
                puts("Failed to append reservations.csv. Events already decremented.");
                puts("In a DB sprint we'd fix this with transactions; for now, note the mismatch.");
                goto_list(&st);
                continue;
            }

            puts("Reservation confirmed.");
            goto_list(&st);
            continue;
        }

        puts("Unknown command for this screen. Try EL, DETAILS, RESERVE, CONTINUE, CONFIRM, CANCEL, QUIT.");
    }

    state_reset_order(&st);
    return 0;
}
