# 02 — Event System

## Overview

The event system is the backbone of case generation. Every meaningful thing that happens in the simulation is logged as an **event** with an explicit **causal link** to the event that caused it. This forms a directed acyclic graph (DAG) that we traverse to extract cases.

There are two distinct concepts:
- **Actions**: What an actor *decides* to do (~8 choices per tick)
- **Events**: What gets *logged* as a result (~20 types, one action can produce multiple events)

---

## Event Types

```cpp
enum sim_event_type : u16 {
    // === Movement ===
    EVT_DEPART = 0,             // actor leaves a landmark
    EVT_ARRIVE,                 // actor arrives at a landmark
    EVT_ENTER_ROOM,             // actor moves to a specific room within a landmark

    // === Social ===
    EVT_MEET,                   // two actors are at the same landmark at the same time
    EVT_ARGUMENT,               // verbal confrontation between two actors
    EVT_PHONE_CALL,             // actor calls another actor (creates phone record evidence)
    EVT_CONVERSATION,           // neutral/positive social interaction

    // === Economic ===
    EVT_TRANSACTION,            // legal money exchange (purchase, payment)
    EVT_DEBT_CREATED,           // actor borrows money / fails to pay
    EVT_DEBT_COLLECTED,         // creditor receives payment

    // === Escalation ===
    EVT_THREAT,                 // actor threatens another (verbal or physical intimidation)
    EVT_ACQUIRE_WEAPON,         // actor obtains a weapon (picks up knife, buys gun)
    EVT_INTOXICATED,            // actor becomes drunk/high at a bar or club

    // === Crime (initial scope: murder + theft) ===
    EVT_MURDER,                 // actor kills another actor
    EVT_THEFT,                  // actor steals an object from a location or person
    EVT_BREAK_IN,               // actor forces entry to a locked location

    // === Aftermath ===
    EVT_FLEE,                   // actor leaves a location rapidly after a crime
    EVT_HIDE_OBJECT,            // actor conceals an object (weapon, stolen goods)
    EVT_DISPOSE_EVIDENCE,       // actor destroys or discards evidence
    EVT_BODY_DISCOVERED,        // actor finds a dead body at a location
    EVT_CALL_POLICE,            // actor reports a crime to authorities
    EVT_WITNESS,                // actor observes another event (reactive, not decided)

    EVT_COUNT
};
```

### Event Categories

| Category | Types | Triggered by |
|----------|-------|-------------|
| **Movement** | DEPART, ARRIVE, ENTER_ROOM | Any action that changes location |
| **Social** | MEET, ARGUMENT, PHONE_CALL, CONVERSATION | Actors sharing space, or deliberate outreach |
| **Economic** | TRANSACTION, DEBT_CREATED, DEBT_COLLECTED | Routine interactions and financial pressure |
| **Escalation** | THREAT, ACQUIRE_WEAPON, INTOXICATED | Drive thresholds crossed |
| **Crime** | MURDER, THEFT, BREAK_IN | High drive scores push actor to criminal action |
| **Aftermath** | FLEE, HIDE_OBJECT, DISPOSE_EVIDENCE, BODY_DISCOVERED, CALL_POLICE, WITNESS | Reactive consequences of crime events |

---

## Event Storage (SQLite)

```sql
CREATE TABLE sim_events (
    event_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    tick            INTEGER NOT NULL,           -- simulation tick number (0-2879)
    sim_time        INTEGER NOT NULL,           -- minutes since Day 0, 00:00
    event_type      INTEGER NOT NULL,           -- sim_event_type enum value
    actor_id        INTEGER REFERENCES actors(id),      -- who performed/experienced this
    target_id       INTEGER,                    -- other actor involved (context-dependent)
    landmark_id     INTEGER REFERENCES landmarks(id),   -- where it happened
    room_id         INTEGER REFERENCES rooms(id),       -- specific room (nullable)
    object_id       INTEGER REFERENCES objects(id),     -- object involved (nullable)
    detail_int      INTEGER DEFAULT 0,          -- type-specific integer payload
    detail_text     TEXT,                        -- type-specific string payload
    cause_event_id  INTEGER REFERENCES sim_events(event_id)  -- what caused this event
);

-- Performance indices
CREATE INDEX idx_evt_tick       ON sim_events(tick);
CREATE INDEX idx_evt_actor      ON sim_events(actor_id);
CREATE INDEX idx_evt_target     ON sim_events(target_id);
CREATE INDEX idx_evt_landmark   ON sim_events(landmark_id);
CREATE INDEX idx_evt_type       ON sim_events(event_type);
CREATE INDEX idx_evt_cause      ON sim_events(cause_event_id);
```

### Field Usage by Event Type

| Event Type | actor_id | target_id | landmark_id | room_id | object_id | detail_int | detail_text |
|------------|----------|-----------|-------------|---------|-----------|------------|-------------|
| EVT_DEPART | who left | — | from where | from room | — | — | — |
| EVT_ARRIVE | who arrived | — | to where | to room | — | — | — |
| EVT_ARGUMENT | instigator | other party | where | which room | — | anger_gained | — |
| EVT_PHONE_CALL | caller | callee | caller's loc | — | caller's phone | duration_min | — |
| EVT_MURDER | killer | victim | where | which room | murder weapon | — | method description |
| EVT_THEFT | thief | owner | where | which room | stolen object | value | — |
| EVT_BREAK_IN | intruder | — | target loc | entry room | tool used | — | entry method |
| EVT_FLEE | who fled | — | from where | — | — | — | — |
| EVT_HIDE_OBJECT | who hid it | — | where hidden | which room | object hidden | — | — |
| EVT_WITNESS | who saw it | — | where | which room | — | witnessed_evt_id | accuracy level |
| EVT_BODY_DISCOVERED | discoverer | victim | where | which room | — | — | — |
| EVT_CALL_POLICE | caller | — | caller's loc | — | — | reported_evt_id | — |

---

## Causal Links

The `cause_event_id` column is the single most important field in the system. It forms a DAG:

```
                              ┌─ EVT_FLEE (cause=131)
                              │
EVT_ARGUMENT (47)             ├─ EVT_HIDE_OBJECT (cause=131)
    │                         │
    └─ EVT_THREAT (cause=47)  └─ EVT_WITNESS (cause=131)
           │                          │
           └─ EVT_ACQUIRE_WEAPON      └─ EVT_BODY_DISCOVERED (cause=134)
                  (cause=112)                 │
                  │                           └─ EVT_CALL_POLICE (cause=140)
                  └─ EVT_MURDER (cause=118)
                         (id=131)
```

**Rules for setting cause_event_id:**

1. **Direct causation**: The action that led to this event. If actor A threatens actor B (EVT_THREAT, id=112) and later actor A acquires a weapon to follow through, EVT_ACQUIRE_WEAPON gets `cause_event_id = 112`.

2. **Reactive events**: When an event triggers another automatically. If EVT_MURDER happens and actor C is in the same room, EVT_WITNESS is logged with `cause_event_id = murder_event_id`.

3. **Root events**: Some events have no cause — they start a new chain. An argument that happens because two actors with negative relationship are at the same bar has `cause_event_id = NULL`. This is a root node in the DAG.

4. **One cause per event**: Each event has exactly one causal parent (or NULL). If multiple factors contributed, pick the most proximate. The full context is recoverable by walking the tree.

---

## Actions → Events Expansion

### Action: GO_TO_WORK / GO_HOME / VISIT_BAR

Routine movement. Generates:
1. `EVT_DEPART` from current landmark (cause = NULL, routine)
2. After `travel_minutes` ticks: `EVT_ARRIVE` at destination (cause = the DEPART event)
3. `EVT_ENTER_ROOM` for default room (cause = ARRIVE event)
4. If another actor is in the same room: `EVT_MEET` (cause = ARRIVE event)

If destination is a bar/club and actor stays for 2+ ticks:
5. `EVT_INTOXICATED` if cumulative bar time exceeds threshold (cause = ARRIVE)

### Action: CONFRONT_GRUDGE

Actor travels to grudge target's location and confronts them. Generates:
1. `EVT_DEPART` from current landmark
2. `EVT_ARRIVE` at grudge target's landmark
3. `EVT_ENTER_ROOM` (moves to the room the target is in)
4. `EVT_ARGUMENT` between actor and target (cause = the ARRIVE event or the original grievance event if trackable)
5. For each bystander in the same room: `EVT_WITNESS` (cause = ARGUMENT event)
6. If anger exceeds violence threshold after argument:
   - `EVT_THREAT` (cause = ARGUMENT) — and possibly further escalation in a later tick

### Action: STEAL_FROM

Actor steals an object from a location. Generates:
1. If location is locked and actor lacks access: `EVT_BREAK_IN` (cause = NULL or desperation root)
2. `EVT_THEFT` of the target object (cause = BREAK_IN if forced entry, else NULL)
3. Object ownership transfers, `OBJ_FLAG_STOLEN` set
4. `EVT_FLEE` from the location (cause = THEFT)
5. For each actor in the same landmark during the theft: `EVT_WITNESS` (cause = THEFT)

### Action: ACQUIRE_WEAPON

Actor picks up a weapon from a known location (their home, a hardware store, etc.). Generates:
1. `EVT_DEPART` + `EVT_ARRIVE` (travel to weapon location)
2. `EVT_ACQUIRE_WEAPON` (cause = the THREAT or ARGUMENT that motivated it)
3. Object transfers to actor's inventory, `SIM_FLAG_HAS_WEAPON` set

### Action: MEET_ALLY

Actor visits their closest ally. Calming effect. Generates:
1. `EVT_DEPART` + `EVT_ARRIVE` (travel)
2. `EVT_MEET` with ally (cause = ARRIVE)
3. `EVT_CONVERSATION` (cause = MEET)
4. Actor's anger decreases by 5-15 points

### Action: FLEE_AREA

Post-crime panic. Actor tries to leave the area. Generates:
1. `EVT_FLEE` from current landmark (cause = the crime event)
2. `EVT_DEPART` (cause = FLEE)
3. May trigger `EVT_HIDE_OBJECT` if carrying incriminating items (cause = FLEE)
4. Actor sets `SIM_FLAG_PARANOID`, avoids public landmarks for 24-48 hours

### Reactive Events (system-generated, no action required)

**EVT_BODY_DISCOVERED**: When any actor arrives at a landmark/room where a dead actor's body is, and the body has not yet been discovered:
- `EVT_BODY_DISCOVERED` (cause = the actor's ARRIVE event)
- Discovering actor's fear increases by 30-50
- Discovering actor may then: `EVT_CALL_POLICE` (cause = BODY_DISCOVERED) if morality > 30
- Or: `EVT_FLEE` (cause = BODY_DISCOVERED) if fear > 50

**EVT_WITNESS**: When a crime or notable event occurs and other actors are in the same room or landmark:
- Same room: high-accuracy witness (detail_text = "clear")
- Same landmark, different room: low-accuracy witness (detail_text = "partial")
- `EVT_WITNESS` (cause = the observed event)
- Witness detail_int stores the event_id they witnessed

**EVT_MEET**: When two actors are at the same landmark simultaneously during the same tick:
- Only logged once per pair per visit (not every tick they're co-located)
- Provides the foundation for "who was where when" — alibi and suspicion data

---

## Event Processing Order

Within a single tick, events are processed in this order:

1. **Drive updates** — Decay/accumulate drives for all actors
2. **Routine evaluation** — Check if routine demands movement
3. **Travel resolution** — Actors in transit check if they've arrived
4. **Action selection** — Each non-traveling actor picks an action
5. **Action execution** — Actions generate primary events (DEPART, ARRIVE, ARGUMENT, etc.)
6. **Reactive pass** — Check for reactive events (WITNESS, BODY_DISCOVERED, MEET)
7. **State updates** — Update sim_actor fields based on events this tick

This ensures that reactive events always fire in response to events from the same tick, and state changes reflect the full tick's activity.

---

## Event Volume Estimates

For a 30-day simulation with ~70 actors:

| Event Category | Est. per day | Est. total (30 days) |
|---------------|-------------|---------------------|
| Movement (DEPART/ARRIVE) | ~280 (2 trips per actor avg) | ~8,400 |
| Room changes | ~350 | ~10,500 |
| Social (MEET/CONVERSATION) | ~50 | ~1,500 |
| Economic | ~10 | ~300 |
| Escalation (ARGUMENT/THREAT) | ~5 | ~150 |
| Crime | 0-1 | 5-10 total |
| Aftermath | 0-3 | 15-30 total |
| Witness | ~10 | ~300 |
| **Total** | **~700** | **~21,000** |

21,000 rows in SQLite is trivial — query performance will not be an issue. The indices on actor, landmark, type, and cause ensure fast lookups during case extraction.
