#include "ui.h"
#include <stdio.h>
#include <string.h>

static void print_line() {
    puts("------------------------------------------------------------");
}

static const char* status_text(int available) {
    return (available > 0) ? "Open" : "Full";
}

static void render_welcome(const EventStore* store) {
    print_line();
    puts("Welcome to the Event List!");
    puts("");
    puts("Featured Events:");

    puts(" ______________________________________________________________________________");
    puts("|        Event                 |      Date           |    Venue        | Status|");
    puts("|______________________________|_____________________|_________________|_______|");

    int shown = 0;
    for (size_t i = 0; i < store->count; i++) {
        const Event* e = &store->events[i];
        if (e->featured != 'Y' && e->featured != 'y') continue;

        // two-line row 
        printf("| %-28s | %-19s | %-15s | %-5s |\n",
               e->name, e->datetime, e->venue, status_text(e->available));
        printf("| %-28s  | %-19s | %-15s | %-5s |\n",
               "", "", "", "");
        puts("|______________________________|_____________________|_________________|_______|");

        shown++;
        if (shown >= 3) break; // cap the welcome screen to 3 featured events
    }

    if (shown == 0) {
        printf("| %-28s | %-19s | %-15s | %-5s|\n",
               "(none)", "", "", "");
        printf("| %-28s  | %-19s | %-15s | %-5s|\n",
               "", "", "", "");
        puts("|______________________________|_____________________|_________________|_______|");
    }

    puts("");
    puts("Check EL for the full Event List, or DETAILS <EventID> for more information:");
    puts("Type QUIT to exit.");
    print_line();
}


static void render_event_list(const EventStore* store) {
    print_line();
    puts("All Events:");

    puts(" ____________________________________________________________________________________");
    puts("|  ID   |      Event                 |      Date           |    Venue        | Status|");
    puts("|_______|____________________________|_____________________|_________________|_______|");

    for (size_t i = 0; i < store->count; i++) {
        const Event* e = &store->events[i];

        int sold_out = (e->available <= 0);

        // Status is shown on two lines. For sold out: "sold" then "out".
        // For available: blank then "avail" (matches your sample’s vibe).
        const char* s1 = sold_out ? "sold" : "";
        const char* s2 = sold_out ? "out"  : "avail";

        // Spacer row (blank ID/Event/Date/Venue). Status top half shows s1 if any.
        printf("|       |                            |                     |                 | %-5s|\n", s1);

        // Data row
        printf("| %-4s  |  %-26s | %-19s | %-15s | %-5s|\n",
               e->id, e->name, e->datetime, e->venue, s2);

        // Separator
        puts("|_______|____________________________|_____________________|_________________|_______|");
    }

    puts("");
    puts("Check DETAILS <EventID> for more information or RESERVE <EventID> <# tickets>.");
    print_line();
}


static void render_event_details(const Event* e) {
    print_line();
    printf("%s %s Details:\n", e->id, e->name);

    puts(" ______________________________________________________________________________");
    puts("|   Details                    | Date / Time         |  Category               |");
    puts("|______________________________|_____________________|_________________________|");

    // Layout is fixed-height to match your wireframe.
    // Sprint 1: we print details as wrapped lines in left column.
    // Date/time: first two lines (date then time) if present; else show full datetime on one line.
    // Category: show one or two lines if user encoded it with ';' (optional).

    // --- Prepare date/time lines ---
    char dt1[32] = {0}, dt2[32] = {0};
    // crude split: if datetime contains a space, show date on line1 and rest on line2
    const char* sp = strchr(e->datetime, ' ');
    if (sp) {
        snprintf(dt1, sizeof(dt1), "%.*s", (int)(sp - e->datetime), e->datetime);
        snprintf(dt2, sizeof(dt2), "%s", sp + 1);
    } else {
        snprintf(dt1, sizeof(dt1), "%s", e->datetime);
        dt2[0] = '\0';
    }

    // --- Prepare category lines (optional ';' split) ---
    char cat1[64] = {0}, cat2[64] = {0};
    const char* semi = strchr(e->category, ';');
    if (semi) {
        snprintf(cat1, sizeof(cat1), "%.*s", (int)(semi - e->category), e->category);
        snprintf(cat2, sizeof(cat2), "%s", semi + 1);
    } else {
        snprintf(cat1, sizeof(cat1), "%s", e->category);
        cat2[0] = '\0';
    }

    // --- Wrap details into up to 8 lines of width 28 ---
    // Column widths: Details=28, Date/Time=19, Category=25 (based on the box drawing)
    enum {DW = 28};
    char dlines[8][DW + 1];
    for (int i = 0; i < 8; i++) dlines[i][0] = '\0';

    const char* d = e->details;
    int li = 0;
    while (*d && li < 8) {
        while (*d == ' ') d++;
        if (!*d) break;

        int len = 0;
        int last_space = -1;
        while (d[len] && len < DW) {
            if (d[len] == ' ') last_space = len;
            len++;
        }

        int take = len;
        if (d[len] && last_space > 0) take = last_space; // wrap at space

        snprintf(dlines[li], sizeof(dlines[li]), "%.*s", take, d);
        // advance
        d += take;
        while (*d == ' ') d++;
        li++;
    }

    // Top block (7-ish rows like your mock)
    printf("| %-28s  | %-19s | %-25s|\n", "", "", "");
    printf("| %-28s  | %-19s | %-25s|\n", dlines[0][0] ? dlines[0] : "", dt1, cat1);
    printf("| %-28s  | %-19s | %-25s|\n", dlines[1][0] ? dlines[1] : "", dt2, cat2);
    printf("| %-28s  | %-19s | %-25s|\n", dlines[2][0] ? dlines[2] : "", "", "");
    printf("| %-28s | %-19s | %-25s|\n", dlines[3][0] ? dlines[3] : "", "", "");
    printf("| %-28s | %-19s | %-25s|\n", dlines[4][0] ? dlines[4] : "", "", "");

    // Divider row that closes the date/time + category top section
    puts("|------------------------------|_____________________|_________________________|");

    // Middle block headers
    printf("| %-28s | %-19s | %-25s|\n",
           dlines[5][0] ? dlines[5] : "", "Location Details", "Tickets Remaining");
    puts("|------------------------------|_____________________|_________________________|");

    // Location + tickets section (fixed lines)
    printf("| %-28s | %-19s | %-25s|\n", dlines[6][0] ? dlines[6] : "", "", "");
    printf("| %-28s | %-19s | %-25s|\n", dlines[7][0] ? dlines[7] : "", e->venue, "");
    printf("| %-28s | %-19s | %-25s|\n", "", "", "");
    printf("| %-28s | %-19s | %-25d|\n", "", "", e->available);
    printf("| %-28s | %-19s | %-25s|\n", "", "", "");
    printf("| %-28s | %-19s | %-25s|\n", "", "", "");

    puts("|______________________________|_____________________|_________________________|");
    puts("");
    puts("RESERVE <# tickets> or EL to return to Event List:");
    print_line();
}

static void render_reserve(const AppState* st, const Event* e) {
    print_line();
    printf("Reserve Tickets for %s %s:\n", e->id, e->name);

    puts(" ______________________________________________________________________");
    puts("|        Event                 |      Date           |    Venue        |");
    puts("|______________________________|_____________________|_________________|");

    // spacer row
    puts("|                              |                     |                 |");

    // data row
    // Note: uses e->datetime as-is. If you want 3/1/26 formatting, we can format later.
    printf("| %-28s | %-19s | %-15s|\n", e->name, e->datetime, e->venue);

    puts("|______________________________|_____________________|_________________|");
    puts(" ");

    printf("Reserving %d Tickets for %s.\n", st->ticket_count, e->name);
    puts("");
    puts("Type CONTINUE to enter ticketholder info, or CANCEL to return to Event List.");
    print_line();
}

static void render_enter_user_data(const AppState* st, const Event* e) {
    print_line();
    printf("Reserve Tickets for %s:\n", e ? e->name : "Event");

    puts(" ______________________________________________________________________");
    puts("| Ticket |    Name                     |   Email                       |");
    puts("|________|_____________________________|_______________________________|");

    for (int i = 0; i < st->ticket_count; i++) {
        const char* nm = (st->holders && st->holders[i].name[0])  ? st->holders[i].name  : "";
        const char* em = (st->holders && st->holders[i].email[0]) ? st->holders[i].email : "";

        // spacer row
        puts("|        |                             |                               |");

        // data row
        printf("|  %-5d|  %-27s|  %-29s|\n", i + 1, nm, em);

        // separator
        puts("|________|_____________________________|_______________________________|");
    }

    puts("");

    int t = st->current_ticket + 1; // display 1-based
    printf("Enter Ticket %d Name:\n\n", t);
    printf("Enter Ticket %d Email:\n", t);

    print_line();
}

static void render_order_summary(const AppState* st, const Event* e) {
    print_line();
    printf("Confirm tickets for %s:\n", e ? e->name : "Event");

    puts(" ______________________________________________________________________");
    puts("| Ticket |    Name                     |   Email                       |");
    puts("|________|_____________________________|_______________________________|");

    for (int i = 0; i < st->ticket_count; i++) {
        const char* nm = (st->holders && st->holders[i].name[0])  ? st->holders[i].name  : "";
        const char* em = (st->holders && st->holders[i].email[0]) ? st->holders[i].email : "";

        puts("|        |                             |                               |");
        printf("|  %-5d|  %-27s|  %-29s|\n", i + 1, nm, em);
        puts("|________|_____________________________|_______________________________|");
    }

    puts("");
    puts("Type CONFIRM to reserve your tickets, or CANCEL to return to the main menu.");
    print_line();
}


void ui_render(const AppState* st, const EventStore* store) {
    if (!st || !store) return;

    if (st->screen == SCREEN_WELCOME) {
        render_welcome(store);
        return;
    }

    if (st->screen == SCREEN_EVENT_LIST) {
        render_event_list(store);
        return;
    }

    const Event* e = events_find(store, st->selected_event_id);

    switch (st->screen) {
        case SCREEN_EVENT_DETAILS:
            if (e) render_event_details(e);
            else puts("No event selected.");
            break;
        case SCREEN_RESERVE_TICKETS:
            if (e) render_reserve(st, e);
            else puts("No event selected.");
            break;
        case SCREEN_ENTER_USER_DATA:
            render_enter_user_data(st, e);
            break;
        case SCREEN_ORDER_SUMMARY:
            if (e) render_order_summary(st, e);
            else puts("No event selected.");
            break;
        default:
            break;
    }
}
