# 01 — Simulation Model

## Overview

The case generator runs a **30-day city simulation** at 15-minute tick resolution. Actors live their lives — going to work, socializing, accumulating pressure — and crimes emerge organically from that pressure. The simulation logs everything to SQLite, and we mine the log for compelling cases afterward.

---

## Time System

```
typedef i32 sim_time;  // minutes since Day 0, 00:00
```

| Constant | Value |
|----------|-------|
| Tick interval | 15 minutes |
| Ticks per day | 96 |
| Simulation length | 30 days = 43,200 minutes = 2,880 ticks |
| Case window (typical) | 8-16 hours extracted from the full 30 days |

**Helpers:**
```cpp
inline i32 sim_day(sim_time t)    { return t / 1440; }
inline i32 sim_hour(sim_time t)   { return (t / 60) % 24; }
inline i32 sim_minute(sim_time t) { return t % 60; }
inline bool is_night(sim_time t)  { i32 h = sim_hour(t); return h >= 22 || h < 6; }
inline bool is_work_hours(sim_time t) { i32 h = sim_hour(t); return h >= 8 && h < 17; }
```

**Why 15-minute ticks?** This matches real-world alibi granularity ("I was at the bar from 9 to 11"). Finer resolution wastes compute without adding gameplay value. Coarser resolution (1 hour) loses the ability to create tight alibi windows.

**Why 30 days?** A month gives enough time for multiple independent pressure arcs to develop across different actors. With ~70 actors, we expect 5-6 viable crime incidents to emerge, giving us the luxury of picking the most interesting case.

---

## City Structure

The city is composed of **districts**, each containing **landmarks** (locations). Landmarks contain **rooms**.

### Districts

```cpp
enum district_type : u8 {
    DISTRICT_RESIDENTIAL,
    DISTRICT_COMMERCIAL,
    DISTRICT_INDUSTRIAL,
    DISTRICT_NIGHTLIFE,
    DISTRICT_DOCKS,
    DISTRICT_FINANCIAL
};

struct district {
    i64 id;
    const char *name;           // Markov-generated
    district_type type;
    i32 wealth;                 // 1-5, affects desperation of low-wealth residents
    i32 roughness;              // 1-5, affects crime_factor of locations
    i32 police_response_time;   // minutes, derived from roughness (rough = slow response)
};
```

**Generation:** 4-8 districts depending on city size. Distribution weighted toward residential and commercial. Wealth and roughness inversely correlated (rough districts tend to be poorer).

### Landmarks (Locations)

```cpp
enum landmark_type : u8 {
    LM_APARTMENT,       // residential
    LM_HOUSE,           // residential
    LM_OFFICE,          // commercial
    LM_SHOP,            // commercial
    LM_WAREHOUSE,       // industrial
    LM_FACTORY,         // industrial
    LM_BAR,             // nightlife
    LM_CLUB,            // nightlife
    LM_RESTAURANT,      // commercial/nightlife
    LM_DOCK,            // docks
    LM_PARK,            // public outdoor
    LM_PARKING_LOT,     // public outdoor
    LM_BANK,            // financial
    LM_PAWN_SHOP,       // commercial (fence for stolen goods)
    LM_HOTEL,           // commercial
    LM_HOSPITAL,        // public service
    LM_GAS_STATION,     // commercial
    LM_CHURCH           // public
};

enum access_control : u8 {
    ACCESS_NONE,            // anyone can enter (park, gas station)
    ACCESS_STANDARD_LOCK,   // door with key (apartments, houses)
    ACCESS_KEYPAD,          // code entry (offices, warehouses)
    ACCESS_SECURITY_STAFF,  // guard on premises (bank, club)
    ACCESS_RESTRICTED       // employees only (factory floor, dock area)
};

enum operation_hours : u8 {
    HOURS_24_7,
    HOURS_BUSINESS,     // 8:00 - 18:00
    HOURS_EVENING,      // 18:00 - 02:00
    HOURS_MORNING       // 06:00 - 12:00
};

struct landmark {
    i64 id;
    i64 district_id;
    const char *name;
    landmark_type type;
    access_control access;
    operation_hours hours;
    i32 crime_factor;       // 0-100, increases escalation rate of conflicts here
    bool has_cameras;       // CCTV present — generates documentary evidence
    i64 owner_id;           // actor who owns this place, -1 if public
};
```

### Rooms

Rooms give spatial depth to landmarks. Evidence is placed in specific rooms. Crimes happen in specific rooms. Players explore rooms within a location.

```cpp
struct room {
    i64 id;
    i64 landmark_id;
    const char *name;       // "Kitchen", "Back Alley", "Office", "Restroom"
    i32 capacity;           // max actors simultaneously
    bool is_exterior;       // outdoor/alley — affects lighting for witness accuracy
    bool has_camera;        // room-level CCTV (inherits from landmark or has its own)
    bool is_private;        // requires access (bedroom, back office) vs common areas
};
```

**Room templates by landmark type:**

| Landmark Type | Rooms |
|---------------|-------|
| LM_APARTMENT | Living Room, Kitchen, Bedroom, Bathroom, Hallway, Stairwell (shared) |
| LM_BAR | Entrance, Bar Counter, Back Room, Restroom, Alley (exterior) |
| LM_WAREHOUSE | Loading Dock (exterior), Main Floor, Office, Storage Room |
| LM_OFFICE | Lobby, Open Floor, Conference Room, Break Room, Restroom, Parking (exterior) |
| LM_PARK | Entrance Path, Main Area, Playground, Wooded Path, Parking Lot |
| LM_HOTEL | Lobby, Room (per occupant), Restaurant, Parking (exterior), Hallway |

Room templates are stored in a config file. At generation time, rooms are instantiated from templates with slight variation (some apartments have balconies, some don't).

---

## Actor Model

### Base Actor (persisted, immutable after generation)

```cpp
enum personality_archetype : u8 {
    ARCHETYPE_VOLATILE,     // high impulsivity, high aggression
    ARCHETYPE_GREEDY,       // high greed, moderate morality
    ARCHETYPE_LOYAL,        // high loyalty, low aggression
    ARCHETYPE_AVERAGE       // balanced traits
};

struct actor {
    i64 id;
    const char *name;
    const char *alias;          // nickname, nullable (~30% have one)
    char sex;                   // 'M' or 'F'
    i32 age;

    // Physical description (for witness testimony accuracy)
    u8 height;                  // SHORT, AVERAGE, TALL
    u8 build;                   // THIN, AVERAGE, HEAVY, MUSCULAR
    u8 hair_color;              // BLACK, BROWN, BLONDE, RED, GRAY, BALD

    // Life anchors
    i64 home_landmark_id;
    i64 work_landmark_id;
    const char *occupation;
    const char *phone_number;

    // Personality (0-100 each, set at generation based on archetype + noise)
    i8 impulsivity;
    i8 morality;
    i8 greed;
    i8 aggression;
    i8 loyalty;

    personality_archetype archetype;
    i32 wealth;                 // 1-5, compared against district wealth for desperation
};
```

### Simulation Actor (volatile state, updated each tick)

```cpp
struct sim_actor {
    i64 actor_id;               // FK to actors table

    // Current position
    i64 current_landmark;
    i64 current_room;           // which room within the landmark
    sim_time arrived_at;        // when they arrived here

    // Psychological drives (volatile, 0-100)
    i8 desperation;             // financial pressure
    i8 anger;                   // accumulated resentment (has a target)
    i8 fear;                    // rises after witnessing crime or being threatened
    i8 intoxication;            // rises at bars/clubs, decays over time

    // Relational state
    i64 grudge_target;          // actor_id they resent most, -1 if none
    i64 closest_ally;           // actor_id of their best friend/partner, -1 if none

    // Inventory
    i64 carried_objects[4];
    i8 num_carried;

    // Flags
    u32 flags;                  // see below
    bool is_alive;
    bool is_arrested;           // post-crime, if caught
};

// Flag definitions
#define SIM_FLAG_HAS_WEAPON     (1 << 0)
#define SIM_FLAG_INTOXICATED    (1 << 1)
#define SIM_FLAG_PARANOID       (1 << 2)    // after committing crime, avoids public
#define SIM_FLAG_FLEEING        (1 << 3)    // actively leaving the area
#define SIM_FLAG_AT_CRIME_SCENE (1 << 4)    // was present during a crime (potential witness)
```

### Drive Initialization

At simulation start, drives are seeded from actor personality + environment:

```
desperation = max(0, (district_wealth - actor_wealth) * 15) + rand_int_min(-5, 5)
anger       = (aggression / 4) + rand_int_min(-5, 5)
fear        = max(0, (100 - aggression) / 5) + rand_int_min(-3, 3)
intoxication = 0
```

Drives are clamped to [0, 100] after every update.

### Drive Decay and Accumulation

Each tick, before action evaluation:

```
// Daily desperation pressure (only at midnight tick)
if (sim_hour(tick_time) == 0 && sim_minute(tick_time) == 0) {
    desperation += max(0, district_wealth - actor_wealth) * config.desperation_gain_per_day;
}

// Anger decays slowly when away from grudge target
if (current_landmark != grudge_target_landmark) {
    anger = max(0, anger - 1);  // -1 per tick when not near target
}

// Fear decays slowly
fear = max(0, fear - 1);  // -1 per tick

// Intoxication decays
intoxication = max(0, intoxication - 2);  // -2 per tick (~2 hours to sober up)
```

---

## Relationships

Relationships are directed edges between actors. They drive social behavior and provide motive for crimes.

```cpp
enum relationship_type : u8 {
    REL_SPOUSE,
    REL_EX_SPOUSE,
    REL_FAMILY,
    REL_FRIEND,
    REL_COWORKER,
    REL_EMPLOYER,
    REL_EMPLOYEE,
    REL_RIVAL,
    REL_DEBTOR,         // A owes money to B
    REL_CREDITOR,       // A is owed money by B
    REL_LOVER,          // secret affair
    REL_NEIGHBOR
};

struct relationship {
    i64 from_actor;
    i64 to_actor;
    relationship_type type;
    i32 strength;       // -100 (hatred) to +100 (deep bond)
};
```

**Generation rules:**
- Actors at the same `home_landmark_id` → NEIGHBOR (strength 10-40)
- Actors at the same `work_landmark_id` → COWORKER (strength 20-60)
- ~15% of actors get a SPOUSE (same home, strength 50-90)
- ~5% of actors get an EX_SPOUSE (strength -40 to -10)
- ~10% get a DEBTOR/CREDITOR pair (strength -30 to -10 for debtor, 10-30 for creditor)
- RIVAL pairs generated between actors in same workplace or neighborhood with high aggression (strength -60 to -20)
- FRIEND pairs between actors who share a regular hangout location (bar, club, park) — strength 30-70

**Grudge target selection:** At initialization and after negative events, an actor's `grudge_target` is set to the actor they have the most negative total relationship strength with.

---

## Objects

Objects exist in the world and can become evidence. They are tracked for ownership, location, and state.

```cpp
enum object_type : u8 {
    OBJ_PHONE,
    OBJ_WALLET,
    OBJ_KEYS,
    OBJ_WEAPON_KNIFE,
    OBJ_WEAPON_GUN,
    OBJ_WEAPON_BLUNT,      // pipe, bat, etc.
    OBJ_TOOL,              // crowbar, lockpick, etc.
    OBJ_DOCUMENT,          // contract, letter, receipt
    OBJ_VEHICLE,
    OBJ_JEWELRY,
    OBJ_DRUG_PARAPHERNALIA,
    OBJ_PERSONAL_ITEM      // watch, lighter, hat, etc.
};

struct sim_object {
    i64 id;
    const char *name;       // "Kitchen knife", "Nokia phone", "Brown leather wallet"
    object_type type;
    i64 owner_id;           // actor who owns it, -1 for unowned
    i64 location_landmark;  // where it is, -1 if carried
    i64 location_room;      // which room, -1 if carried
    i64 carrier_id;         // who carries it, -1 if placed
    u32 flags;              // OBJ_BLOODY, OBJ_FINGERPRINTED, OBJ_HIDDEN, OBJ_DAMAGED
};

#define OBJ_FLAG_BLOODY         (1 << 0)
#define OBJ_FLAG_FINGERPRINTED  (1 << 1)    // someone touched it without gloves
#define OBJ_FLAG_HIDDEN         (1 << 2)    // deliberately concealed
#define OBJ_FLAG_DAMAGED        (1 << 3)
#define OBJ_FLAG_STOLEN         (1 << 4)
```

**Generation:** Every actor starts with a PHONE, WALLET, and KEYS. ~20% carry a PERSONAL_ITEM. Weapons are at locations (kitchen knives at homes, tools at warehouses) not on actors initially. Actors must travel to acquire weapons — this creates evidence.

---

## Transit System

Travel between landmarks takes real time. A transit table stores travel duration between all landmark pairs:

```sql
CREATE TABLE transit (
    from_landmark INTEGER NOT NULL,
    to_landmark   INTEGER NOT NULL,
    travel_minutes INTEGER NOT NULL,
    PRIMARY KEY (from_landmark, to_landmark)
);
```

Travel time is based on district distance (same district = 5-15 min, adjacent = 15-30 min, far = 30-60 min). When an actor decides to travel, they enter a "traveling" state and arrive after `travel_minutes` ticks have elapsed.

---

## Routines

Each actor has a daily routine that provides their default behavior. This is critical for alibi construction — if an actor deviates from routine, that deviation IS the evidence.

```sql
CREATE TABLE actor_routines (
    actor_id      INTEGER NOT NULL,
    day_type      INTEGER NOT NULL,     -- 0=weekday, 1=weekend
    hour_start    INTEGER NOT NULL,     -- 0-23
    hour_end      INTEGER NOT NULL,     -- 0-23
    landmark_id   INTEGER NOT NULL,     -- where they should be
    PRIMARY KEY (actor_id, day_type, hour_start)
);
```

**Default routine (weekday):**
- 06:00-08:00: home
- 08:00-17:00: work
- 17:00-18:00: travel home
- 18:00-22:00: home (or ~20% chance: bar/restaurant/friend's place)
- 22:00-06:00: home (sleeping)

**Weekend:** No work block. Replace with: home until noon, then 50% chance of going out (shopping, bar, park, visiting friends).

Routines provide the "normal" that makes deviations detectable. If an actor who always goes home at 6 PM instead went to a warehouse at 10 PM, that's suspicious — and there's evidence of it (they weren't home, they were seen elsewhere).
