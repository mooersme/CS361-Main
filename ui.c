#include "ui.h"
#include <stdio.h>
#include <string.h>

static void print_line() {
    puts("------------------------------------------------------------");
}

static void render_welcome() {
    print_line();
    puts("WELCOME");
    puts("Type EL to view the Event List.");
    puts("Type QUIT to exit.");
    print_line();
}

static void render_event_list(const EventStore* store) {
    print_line();
    puts("EVENT LIST");
    puts("ID    | Event                | Date/Time           | Venue            | Avail");
    print_line();
    for (size_t i = 0; i < store->count; i++) {
        const Event* e = &store->events[i];
        printf("%-5s | %-20s | %-18s | %-15s | %d\n",
               e->id, e->name, e->datetime, e->venue, e->available);
    }
    print_line();
    puts("Commands:");
    puts("  DETAILS <EVENT_ID>");
    puts("  RESERVE <EVENT_ID> <#>");
    puts("  EL   (reprint list)");
    puts("  QUIT");
    print_line();
}

static void render_event_details(const Event* e) {
    print_line();
    puts("EVENT DETAILS");
    printf("Event: %s (%s)\n", e->name, e->id);
    printf("When:  %s\n", e->datetime);
    printf("Where: %s\n", e->venue);
    printf("Category/Restrictions: %s\n", e->category);
    printf("Tickets Remaining: %d\n", e->available);
    puts("");
    puts("Details:");
    puts(e->details);
    print_line();
    puts("Commands:");
    puts("  RESERVE <#>");
    puts("  EL");
    puts("  CANCEL  (back to list)");
    puts("  QUIT");
    print_line();
}

static void render_reserve(const AppState* st, const Event* e) {
    print_line();
    puts("RESERVE TICKETS");
    printf("Event: %s (%s)\n", e->name, e->id);
    printf("Tickets requested: %d\n", st->ticket_count);
    puts("");
    puts("Commands:");
    puts("  CONTINUE  (enter ticket-holder info)");
    puts("  CANCEL    (abort reservation)");
    puts("  EL        (back to list)");
    puts("  QUIT");
    print_line();
}

static void render_enter_user_data(const AppState* st) {
    print_line();
    puts("ENTER USER DATA");
    printf("Tickets: %d\n", st->ticket_count);
    puts("You will be prompted for each ticket-holder Name + Email.");
    puts("Type X at any prompt to abort.");
    print_line();
}

static void render_order_summary(const AppState* st, const Event* e) {
    print_line();
    puts("ORDER SUMMARY");
    printf("Order: %s\n", st->order_id);
    printf("Event: %s (%s)\n", e->name, e->id);
    printf("Reserved %d seat(s)\n", st->ticket_count);
    print_line();
    for (int i = 0; i < st->ticket_count; i++) {
        printf("Ticket %d: %s <%s>\n", i + 1, st->holders[i].name, st->holders[i].email);
    }
    print_line();
    puts("Commands:");
    puts("  CONFIRM  (finalize; writes reservations.csv and updates events.csv)");
    puts("  CANCEL   (abort)");
    puts("  EL       (back to list)");
    puts("  QUIT");
    print_line();
}

void ui_render(const AppState* st, const EventStore* store) {
    if (!st || !store) return;

    if (st->screen == SCREEN_WELCOME) {
        render_welcome();
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
            render_enter_user_data(st);
            break;
        case SCREEN_ORDER_SUMMARY:
            if (e) render_order_summary(st, e);
            else puts("No event selected.");
            break;
        default:
            break;
    }
}
