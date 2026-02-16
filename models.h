#ifndef MODELS_H
#define MODELS_H

#include <stddef.h>
#include <time.h>

#define MAX_EVENTS 256
#define MAX_ID     32
#define MAX_NAME   128
#define MAX_DT     32
#define MAX_VENUE  128
#define MAX_CAT    64
#define MAX_DETAILS 512
#define MAX_EMAIL  128

typedef struct {
    char id[MAX_ID];
    char name[MAX_NAME];
    char datetime[MAX_DT];
    char venue[MAX_VENUE];
    char category[MAX_CAT];
    int  available;
    char featured;
    char details[MAX_DETAILS];
} Event;

typedef struct {
    char name[MAX_NAME];
    char email[MAX_EMAIL];
} TicketHolder;

typedef enum {
    SCREEN_WELCOME = 0,
    SCREEN_EVENT_LIST,
    SCREEN_EVENT_DETAILS,
    SCREEN_RESERVE_TICKETS,
    SCREEN_ENTER_USER_DATA,
    SCREEN_ORDER_SUMMARY
} Screen;

typedef struct {
    // current navigation
    Screen screen;
    char selected_event_id[MAX_ID];

    // reservation in progress
    int ticket_count;
    TicketHolder* holders;   // dynamically allocated array sized ticket_count
    char order_id[MAX_ID];   // simple generated id
} AppState;

#endif
