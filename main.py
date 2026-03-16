import csv
import os
import requests
from datetime import datetime

# ----------------------------
# CONFIG
# ----------------------------

NAME_URL = "http://127.0.0.1:8070/validate-name"
EMAIL_URL = "http://127.0.0.1:8060/validate-email"
CALENDAR_URL = "http://127.0.0.1:8050/ics"
EMAILER_URL = "http://127.0.0.1:8080/send-file"

EVENTS_FILE = "data/events.csv"
RESERVATIONS_FILE = "data/reservations.csv"

WIDTH = 80

# ----------------------------
# UTILITIES
# ----------------------------


def clear():
    os.system("cls" if os.name == "nt" else "clear")


def line(char="-"):
    print(char * WIDTH)


def header(title):
    line("=")
    print(title.center(WIDTH))
    line("=")


def format_datetime_for_ics(dt_string):
    dt = datetime.strptime(dt_string, "%Y-%m-%d %H:%M")
    return dt.strftime("%Y%m%dT%H%M%SZ")


def generate_order_id():
    return datetime.now().strftime("%Y%m%d%H%M%S")

# ----------------------------
# CSV
# ----------------------------


def load_events():
    with open(EVENTS_FILE, newline="", encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def save_events(events):
    with open(EVENTS_FILE, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=events[0].keys())
        writer.writeheader()
        writer.writerows(events)


def append_reservation(order_id, event_id, ticket_no, name, email):
    with open(RESERVATIONS_FILE, "a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow([
            order_id,
            event_id,
            ticket_no,
            name,
            email,
            datetime.now().isoformat()
        ])

# ----------------------------
# MICROSERVICE CALLS
# ----------------------------


def validate_name(first, last):
    r = requests.post(NAME_URL, json={
        "first_name": first,
        "last_name": last
    })
    if r.status_code != 200:
        return None
    return r.json()


def validate_email(email):
    r = requests.post(EMAIL_URL, json={"email": email})
    if r.status_code != 200:
        return None
    return r.json()["email"]


def generate_ics(event):
    start = format_datetime_for_ics(event["datetime"])

    payload = {
        "event_name": event["name"],
        "start": start,
        "end": start,
        "location": event["venue"],
        "description": event["details"],
        "uid": generate_order_id()
    }

    r = requests.post(CALENDAR_URL, json=payload)

    if r.status_code != 200:
        print("Calendar MS error:", r.text)
        return None

    return r.content


def send_email(recipient, subject, body, file_path):
    r = requests.post(EMAILER_URL, json={
        "recipient": recipient,
        "subject": subject,
        "body": body,
        "file_path": os.path.abspath(file_path)
    })
    return r.status_code == 200

# ----------------------------
# RESERVATION FLOW
# ----------------------------


def reserve(event):
    available = int(event["available"])

    if available <= 0:
        print("\nThis event is sold out.")
        input("Press Enter to continue...")
        return

    try:
        tickets_requested = int(input("\nHow many tickets? "))
    except:
        return

    if tickets_requested <= 0 or tickets_requested > available:
        print("\nInvalid ticket quantity.")
        input("Press Enter to continue...")
        return

    order_id = generate_order_id()
    events = load_events()

    for i in range(1, tickets_requested + 1):

        # --- NAME ---
        while True:
            first = input("\nFirst Name: ")
            last = input("Last Name: ")
            name_valid = validate_name(first, last)
            if name_valid:
                break
            print("Invalid name. Try again.")

        # --- EMAIL ---
        while True:
            email = input("Email: ")
            email_valid = validate_email(email)
            if email_valid:
                break
            print("Invalid email. Try again.")

        # --- ICS ---
        ics_bytes = generate_ics(event)
        if not ics_bytes:
            print("Failed to generate calendar file.")
            return

        filename = f"{order_id}_ticket_{i}.ics"
        with open(filename, "wb") as f:
            f.write(ics_bytes)

        # --- EMAIL SEND ---
        success = send_email(
            email_valid,
            f"Your ticket for {event['name']}",
            "Attached is your event ticket.",
            filename
        )

        os.remove(filename)

        if not success:
            print("Failed to send email.")
            return

        full_name = f"{name_valid['first_name']} {name_valid['last_name']}"

        append_reservation(
            order_id,
            event["id"],
            i,
            full_name,
            email_valid
        )

        event["available"] = str(int(event["available"]) - 1)

    # Update events.csv
    for e in events:
        if e["id"] == event["id"]:
            e["available"] = event["available"]

    save_events(events)

    print("\nReservation complete.")
    print(f"Order ID: {order_id}")
    input("Press Enter to continue...")

# ----------------------------
# MAIN LOOP
# ----------------------------


def main():
    while True:
        header("EVENT LIST")

        events = load_events()

        print(f"{'ID':<5}{'Name':<30}{'Date/Time':<20}{'Avail':<8}")
        line()

        for e in events:
            print(f"{e['id']:<5}{e['name']:<30}{e['datetime']:<20}{e['available']:<8}")

        print("\nEnter Event ID to view details.")
        print("Type X to exit.")

        choice = input("Choice: ").strip()

        if choice.lower() == "x":
            break

        selected = next((e for e in events if e["id"] == choice), None)

        if not selected:
            continue

        # --- EVENT DETAILS SCREEN ---
        while True:
            header("EVENT DETAILS")
            print(f"Name: {selected['name']}")
            print(f"Date/Time: {selected['datetime']}")
            print(f"Venue: {selected['venue']}")
            print(f"Category: {selected['category']}")
            print(f"Available: {selected['available']}")
            print("\nDetails:")
            print(selected['details'])

            print("\n1. Reserve")
            print("2. Back")

            action = input("Choice: ").strip()

            if action == "1":
                reserve(selected)
                break
            elif action == "2":
                break


if __name__ == "__main__":
    main()