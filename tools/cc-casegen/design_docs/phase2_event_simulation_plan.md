# Phase 2: Event Simulation - Implementation Plan

## Overview

Transform the static world generated in Phase 1 into a living simulation where actors make decisions based on needs and personality, eventually leading to 3-5 crimes with full causation tracking and evidence generation.

---

## New Database Tables

### Core Tables

```sql
-- Events with causation tracking
CREATE TABLE events (
    id INTEGER PRIMARY KEY,
    type INTEGER NOT NULL,           -- event_type enum (34 types)
    class INTEGER NOT NULL,          -- primary_crime, related_incident, background
    start_time INTEGER NOT NULL,     -- simulation tick
    end_time INTEGER,                -- null for instant events
    location_id INTEGER,
    description TEXT,
    parent_event_id INTEGER,         -- causal link
    FOREIGN KEY (location_id) REFERENCES locations(id),
    FOREIGN KEY (parent_event_id) REFERENCES events(id)
);

-- Junction: who participated in each event
CREATE TABLE event_participants (
    event_id INTEGER,
    person_id INTEGER,
    role INTEGER NOT NULL,           -- actor, target, witness, bystander
    PRIMARY KEY (event_id, person_id)
);

-- Junction: objects involved in events
CREATE TABLE event_objects (
    event_id INTEGER,
    object_id INTEGER,
    role INTEGER NOT NULL,           -- weapon, tool, evidence, target, vehicle, document
    PRIMARY KEY (event_id, object_id)
);

-- Actor runtime state (mutable during simulation)
CREATE TABLE actor_states (
    person_id INTEGER PRIMARY KEY,
    state INTEGER NOT NULL,          -- normal, desperate, planning, executing, fugitive, arrested, deceased
    current_location_id INTEGER,
    state_changed_at INTEGER,
    crime_event_id INTEGER           -- if fugitive, which crime they committed
);

-- Actor memory for causation tracking
CREATE TABLE actor_memory (
    id INTEGER PRIMARY KEY,
    person_id INTEGER NOT NULL,
    event_id INTEGER NOT NULL,
    memory_time INTEGER NOT NULL,
    salience INTEGER DEFAULT 50      -- importance 0-100, decays over time
);

-- Observable stat changes (timestamped)
CREATE TABLE stat_changes (
    id INTEGER PRIMARY KEY,
    tick INTEGER NOT NULL,
    entity_type INTEGER NOT NULL,    -- 0=person, 1=relationship, 2=object
    entity_id INTEGER NOT NULL,
    stat_name TEXT NOT NULL,
    old_value INTEGER,
    new_value INTEGER,
    cause_event_id INTEGER
);

-- Evidence items that can be discovered
CREATE TABLE evidence (
    id INTEGER PRIMARY KEY,
    type INTEGER NOT NULL,            -- evidence_type enum (physical, testimonial, documentary, digital, forensic)
    source_event_id INTEGER NOT NULL, -- Event that generated this evidence
    location_id INTEGER,              -- Where to find this evidence (for physical/forensic)
    object_id INTEGER,                -- Associated object (receipt, phone, weapon, etc.)
    person_id INTEGER,                -- Associated person (for testimonial - the witness)
    reliability INTEGER DEFAULT 100,  -- 0-100, how trustworthy this evidence is
    description TEXT,                 -- Human-readable description for gameplay
    FOREIGN KEY (source_event_id) REFERENCES events(id),
    FOREIGN KEY (location_id) REFERENCES locations(id),
    FOREIGN KEY (object_id) REFERENCES objects(id),
    FOREIGN KEY (person_id) REFERENCES people(id)
);

-- What information this evidence unlocks when discovered
CREATE TABLE evidence_reveals (
    id INTEGER PRIMARY KEY,
    evidence_id INTEGER NOT NULL,
    entity_type INTEGER NOT NULL,     -- reveal_entity_type enum (0=person, 1=location, 2=object, 3=event, 4=relationship)
    entity_id INTEGER NOT NULL,       -- ID in the corresponding table
    field_name TEXT NOT NULL,         -- Which field is revealed (e.g., "phone_number", "owner_id", "start_time")
    FOREIGN KEY (evidence_id) REFERENCES evidence(id)
);
```

### Evidence Tables Usage Example

A phone call event (id=42) between person 5 and person 12 generates phone record evidence:

```sql
-- The evidence item itself
INSERT INTO evidence (id, type, source_event_id, location_id, object_id, person_id, reliability, description)
VALUES (1, 3, 42, NULL, NULL, NULL, 95, 'Phone records show call between two parties at 2:15 PM');
-- type=3 is EVID_DIGITAL

-- What this evidence reveals to the player
INSERT INTO evidence_reveals (id, evidence_id, entity_type, entity_id, field_name) VALUES
    (1, 1, 0, 5, 'phone_number'),   -- Reveals person 5's phone number
    (2, 1, 0, 12, 'phone_number'),  -- Reveals person 12's phone number
    (3, 1, 3, 42, 'start_time'),    -- Reveals when the call happened (event field)
    (4, 1, 3, 42, 'location_id');   -- Reveals caller's location via cell tower
-- entity_type: 0=ENTITY_PERSON, 3=ENTITY_EVENT
```

A receipt found at a crime scene:

```sql
-- Physical evidence: the receipt object
INSERT INTO evidence (id, type, source_event_id, location_id, object_id, person_id, reliability, description)
VALUES (2, 0, 38, 15, 99, NULL, 90, 'Receipt for rope and duct tape found at scene');
-- type=0 is EVID_PHYSICAL, object_id=99 is the receipt object, location_id=15 is crime scene

-- What this reveals
INSERT INTO evidence_reveals (id, evidence_id, entity_type, entity_id, field_name) VALUES
    (5, 2, 2, 99, 'owner_id'),      -- Reveals who purchased (object's owner)
    (6, 2, 3, 38, 'start_time'),    -- Reveals when purchased (transaction event time)
    (7, 2, 3, 38, 'location_id');   -- Reveals where purchased (store location)
-- entity_type: 2=ENTITY_OBJECT, 3=ENTITY_EVENT
```

---

## New Enums (event_simulation.h)

### Event Types (37 total)

```cpp
enum event_type : i8 {
    // Crime (6)
    EVT_CRIME_COMMITTED, EVT_CRIME_ATTEMPTED, EVT_CRIME_DISCOVERED,
    EVT_CRIME_PREP, EVT_RECRUITMENT_ATTEMPT, EVT_ALIBI_FABRICATION,

    // Movement (6)
    EVT_ACTOR_ENTERS, EVT_ACTOR_LEAVES, EVT_ACTOR_SEEN_AT,
    EVT_GROUP_GATHERING, EVT_LOITERING, EVT_ROUTINE_DEVIATION,

    // Interaction (7)
    EVT_DIRECT_CONVERSATION, EVT_PHONE_CALL, EVT_MESSAGE_SENT,
    EVT_ARGUMENT, EVT_TRANSACTION, EVT_THREAT, EVT_BETRAYAL,

    // Object (6)
    EVT_OBJECT_CREATED, EVT_OBJECT_MOVED, EVT_OBJECT_USED,
    EVT_OBJECT_DESTROYED, EVT_OBJECT_DISCOVERED, EVT_OBJECT_MODIFIED,

    // Discovery (5)
    EVT_BODY_FOUND, EVT_BREAK_IN_REPORTED, EVT_MISSING_PERSON_REPORTED,
    EVT_ANONYMOUS_TIP, EVT_SUSPICIOUS_ACTIVITY_REPORTED,

    // State Changes (7)
    EVT_FINANCIAL_CHANGE, EVT_RELATIONSHIP_STATUS_CHANGE, EVT_EMPLOYMENT_CHANGE,
    EVT_EMOTIONAL_STATE, EVT_INJURY, EVT_SCHEDULE_CHANGE, EVT_IDENTITY_ASSUMED
};

enum crime_type : i8 {
    CRIME_MURDER, CRIME_ATTEMPTED_MURDER, CRIME_KIDNAPPING,
    CRIME_EXTORTION, CRIME_ROBBERY, CRIME_BURGLARY,
    CRIME_ASSAULT, CRIME_DRUG_TRAFFICKING, CRIME_MONEY_LAUNDERING
};

enum actor_state : i8 {
    STATE_NORMAL,           // Daily activities
    STATE_DESPERATE,        // Low needs, considering crime
    STATE_PLANNING_CRIME,   // Actively preparing
    STATE_EXECUTING_CRIME,  // Committing crime
    STATE_FUGITIVE,         // Post-crime, fleeing/concealing
    STATE_ARRESTED,
    STATE_DECEASED
};

enum evidence_type : i8 {
    EVID_PHYSICAL,
    EVID_TESTIMONIAL,
    EVID_DOCUMENTARY,
    EVID_DIGITAL,
    EVID_FORENSIC
};

// Entity types for evidence_reveals table
enum reveal_entity_type : i8 {
    ENTITY_PERSON,       // 0 - reveals field from people table
    ENTITY_LOCATION,     // 1 - reveals field from locations table
    ENTITY_OBJECT,       // 2 - reveals field from objects table
    ENTITY_EVENT,        // 3 - reveals field from events table
    ENTITY_RELATIONSHIP  // 4 - reveals field from relationships table
};
```

---

## Actor State Machine

```
┌─────────────┐
│   NORMAL    │◄─────────────────────────────┐
└──────┬──────┘                              │
       │ [any need < 30]               [needs satisfied]
       ▼                                     │
┌─────────────┐                              │
│  DESPERATE  │──────────────────────────────┘
└──────┬──────┘
       │ [need < 20 AND (impulsivity > 60 OR morality < 30)]
       ▼
┌─────────────┐
│  PLANNING   │◄──┐
└──────┬──────┘   │ [failed/interrupted]
       │ [prepared]
       ▼          │
┌─────────────┐   │
│  EXECUTING  │───┘
└──────┬──────┘
       │ [crime committed]
       ▼
┌─────────────┐
│  FUGITIVE   │
└──────┬──────┘
       │
   ┌───┼───┐
   ▼   ▼   ▼
ARRESTED / NORMAL(escaped) / DECEASED
```

**State Behaviors:**
- NORMAL: work, social, shopping, routine movement
- DESPERATE: same as normal but with crime consideration
- PLANNING: stalk target, acquire tools, scout locations
- EXECUTING: commit crime, may succeed or fail, failures go back to planning, may create witnesses
- FUGITIVE: flee, hide evidence, destroy evidence, lay low, establish alibi

---

## Actor Memory System

Each actor maintains a circular buffer of 16 recent events they experienced/witnessed.

**Memory Operations:**
- `memory_push(actor, event_id, salience)` - Add event to memory
- `memory_get_relevant(actor, event_type)` - Find most salient matching memory
- `memory_decay(actor, 0.95f)` - Decay salience each tick (5% per tick)

**Salience by Role:**
| Event Type | As Actor | As Target | As Witness |
|------------|----------|-----------|------------|
| Crime      | 100      | 100       | 80         |
| Movement   | 30       | 30        | 10         |
| Interaction| 50       | 50        | 20         |
| Object     | 30       | 30        | 10         |
| Discovery  | 75       | 75        | 50         |
| State Ch.  | 30       | 30        | 10         |

**ParentEventId Assignment:**
When actor takes action, check their memory for the triggering event:
- Conflict actions (argue, threaten, fight) → look for previous conflict memories
- Crime actions → look for planning event in memory
- Evidence tampering → use crime_event_id from actor state

---

## Decision-Making Algorithm

Each tick, every active actor selects an action:

```cpp
action_type select_action(actor, person, world, tick) {
    1. Get base weights from current state
    2. Apply need modifiers (low money → +economic actions)
    3. Apply personality modifiers (high aggression → +conflict)
    4. Apply time modifiers (work hours → +work action)
    5. Apply opportunity modifiers (rival nearby → +conflict)
    6. Weighted random selection
}
```

**Need Modifiers:**
- need_money < 30 → work/sell/collect_debt x2
- need_money < 15 → commit_crime x3 (desperate)
- need_belonging < 30 → social actions x2
- need_security < 30 → go_home x2, acquire_weapon +20

**Personality Modifiers:**
- impulsivity > 70 → argue/fight/commit_crime x2
- morality < 30 → threaten/commit_crime x2-3
- greed > 70 → economic actions x2
- aggression > 70 → conflict actions x2-3

---

## Evidence Generation

Events automatically generate evidence based on rules:

<!-- TODO: Expand this table with more complete event→evidence mappings -->
<!--
    // Interaction (7)
    EVT_DIRECT_CONVERSATION, EVT_PHONE_CALL, EVT_MESSAGE_SENT,
    EVT_ARGUMENT, EVT_TRANSACTION, EVT_THREAT, EVT_BETRAYAL,

    // Object (6)
    EVT_OBJECT_CREATED, EVT_OBJECT_MOVED, EVT_OBJECT_USED,
    EVT_OBJECT_DESTROYED, EVT_OBJECT_DISCOVERED, EVT_OBJECT_MODIFIED,

    // Discovery (5)
    EVT_BODY_FOUND, EVT_BREAK_IN_REPORTED, EVT_MISSING_PERSON_REPORTED,
    EVT_ANONYMOUS_TIP, EVT_SUSPICIOUS_ACTIVITY_REPORTED,

    // State Changes (7)
    EVT_FINANCIAL_CHANGE, EVT_RELATIONSHIP_STATUS_CHANGE, EVT_EMPLOYMENT_CHANGE,
    EVT_EMOTIONAL_STATE, EVT_INJURY, EVT_SCHEDULE_CHANGE, EVT_IDENTITY_ASSUMED

    enum evidence_type : i8 {
        EVID_PHYSICAL,
        EVID_TESTIMONIAL,
        EVID_DOCUMENTARY,
        EVID_DIGITAL,
        EVID_FORENSIC
    };

    enum crime_type : i8 {
        CRIME_MURDER, CRIME_ATTEMPTED_MURDER, CRIME_KIDNAPPING,
        CRIME_EXTORTION, CRIME_ROBBERY, CRIME_BURGLARY,
        CRIME_ASSAULT, CRIME_DRUG_TRAFFICKING, CRIME_MONEY_LAUNDERING
    };
-->

| Event                             | Evidence Type      | Probability | Description Template                                                         |
| --------------------------------- | ------------------ | ----------- | ----------------------------------------------------------------------       |
|                                   |                    |             |                                                                              |
| EVT\_CRIME\_COMMITTED             | Physical           | 90%         | {object} found at crime scene                                                |
| EVT\_CRIME\_COMMITTED             | Forensic           | 60%         | Fingerprints on {object} match {actor}                                       |
| EVT\_CRIME\_COMMITTED             | Forensic           | 40%         | DNA evidence found at scene                                                  |
| EVT\_CRIME\_COMMITTED             | Testimonial        | 20%         | Witness directly observed {actor} committing the crime                       |
| EVT\_CRIME\_COMMITTED             | Documentary        | 20%         | Crime planning document found in {actor}'s possession                        |
| EVT\_CRIME\_COMMITTED             | Forensic           | 50%         | Fiber evidence links {actor} to crime scene                                  |
| EVT\_CRIME\_COMMITTED             | Digital            | 15%         | Security camera footage captured crime in progress                           |
| EVT\_CRIME\_ATTEMPTED             | Physical           | 90%         | {object} found at crime scene                                                |
| EVT\_CRIME\_ATTEMPTED             | Forensic           | 60%         | Fingerprints on {object} match {actor}                                       |
| EVT\_CRIME\_ATTEMPTED             | Forensic           | 40%         | DNA evidence found at scene                                                  |
| EVT\_CRIME\_ATTEMPTED             | Testimonial        | 20%         | Witness directly observed {actor} attempting the crime                       |
| EVT\_CRIME\_ATTEMPTED             | Documentary        | 20%         | Crime planning document found in {actor}'s possession                        |
| EVT\_CRIME\_ATTEMPTED             | Forensic           | 50%         | Fiber evidence links {actor} to crime scene                                  |
| EVT\_CRIME\_ATTEMPTED             | Digital            | 15%         | Security camera footage captured attempted crime in progress                 |
| EVT\_CRIME\_DISCOVERED            | Testimonial        | 90%         | Discoverer {actor} describes finding the scene                               |
| EVT\_CRIME\_DISCOVERED            | Documentary        | 50%         | Discoverer {actor}'s notes/log mention finding the {crime\_type}             |
| EVT\_CRIME\_DISCOVERED            | Digital            | 60%         | Emergency call recording from {actor} reporting discovery                    |
| EVT\_CRIME\_DISCOVERED            | Forensic           | 20%         | Discoverer {actor}'s DNA/fingerprints found at scene (contamination)         |
| EVT\_CRIME\_PREP                  | Digital            | 40%         | Search history shows research on {crime\_type}                               |
| EVT\_CRIME\_PREP                  | Documentary        | 70%         | Purchase records for {tools/supplies}                                        |
| EVT\_CRIME\_PREP                  | Testimonial        | 30%         | Neighbor saw suspicious person watching {target}                             |
| EVT\_CRIME\_PREP                  | Digital            | 60%         | Cell tower places {actor} near victim's location on multiple occasions       |
| EVT\_CRIME\_PREP                  | Documentary        | 50%         | Written plan of crime found in {actor}'s possession                          |
| EVT\_CRIME\_PREP                  | Digital            | 50%         | Social media comments from {actor} on victim's page                          |
| EVT\_CRIME\_PREP                  | Forensic           | 30%         | DNA from {actor} found at victim's home/workplace                            |
| EVT\_CRIME\_PREP                  | Forensic           | 40%         | Fingerprints on victim's {object} match {actor}                              |
| EVT\_RECRUITMENT\_ATTEMPT         | Digital            | 90%         | Phone records show {actor} contacted {target} about joining scheme           |
| EVT\_RECRUITMENT\_ATTEMPT         | Testimonial        | 25%         | {target} refused and reported {actor}'s recruitment attempt                  |
| EVT\_ALIBI\_FABRICATION           | Digital            | 90%         | Phone records show {actor} coordinating false alibi with {accomplice}        |
| EVT\_ALIBI\_FABRICATION           | Testimonial        | 25%         | {accomplice} admits {actor} asked them to lie about alibi                    |
|                                   |                    |             |                                                                              |
| EVT\_ACTOR\_ENTERS                | Digital            | 70%         | Security log shows {actor} entered at {time}                                 |
| EVT\_ACTOR\_ENTERS                | Testimonial        | 40%         | Witness saw {actor} entering                                                 |
| EVT\_ACTOR\_LEAVES                | Digital            | 70%         | Security log shows {actor} left at {time}                                    |
| EVT\_ACTOR\_LEAVES                | Testimonial        | 40%         | Witness saw {actor} leaving                                                  |
| EVT\_ACTOR\_SEEN\_AT              | Digital            | 35%         | Security camera footage shows {actor} at {location}                          |
| EVT\_ACTOR\_SEEN\_AT              | Testimonial        | 60%         | Witness saw {actor} at {location}                                            |
| EVT\_GROUP\_GATHERING             | Digital            | 30%         | Security camera footage shows group meeting at {location}                    |
| EVT\_GROUP\_GATHERING             | Testimonial        | 50%         | Multiple witnesses saw group meeting                                         |
| EVT\_LOITERING                    | Digital            | 30%         | Security camera footage shows {actor} stationary for extended period         |
| EVT\_LOITERING                    | Testimonial        | 40%         | Security guard noticed {actor} lingering suspiciously                        |
| EVT\_ROUTINE\_DEVIATION           | Digital            | 25%         | Phone location data shows {actor} at unusual location during normal schedule |
| EVT\_ROUTINE\_DEVIATION           | Testimonial        | 30%         | Coworker noticed {actor} acting unusual                                      |
|                                   |                    |             |                                                                              |
| EVT\_DIRECT\_CONVERSATION         | Testimonial        | 40%         | Witness saw {actor1} talking to {actor2}                                     |
| EVT\_PHONE\_CALL                  | Digital            | 95%         | Phone records show call from {caller} to {receiver} at {time}                |
| EVT\_PHONE\_CALL                  | Digital (tower)    | 80%         | Cell tower places {caller} near {location} at {time}                         |
| EVT\_MESSAGE\_SENT                | Digital            | 90%         | Text message from {sender} to {receiver}: "{content}"                        |
| EVT\_ARGUMENT                     | Testimonial        | 60%         | Witness heard {actor1} and {actor2} arguing                                  |
| EVT\_TRANSACTION                  | Documentary        | 85%         | Receipt shows {actor} purchased {object} at {location}                       |
| EVT\_TRANSACTION                  | Testimonial        | 50%         | Clerk remembers selling {object} to {actor}                                  |
| EVT\_THREAT                       | Testimonial        | 70%         | {target} told police about threat from {actor}                               |
| EVT\_THREAT                       | Digital            | 50%         | Threatening message from {actor} to {target}                                 |
|                                   |                    |             |                                                                              |
| EVT\_OBJECT\_CREATED              | Documentary        | 85%         | Purchase record for {object}                                                 |
| EVT\_OBJECT\_MOVED                | Forensic           | 50%         | Fingerprints on {object}                                                     |
| EVT\_OBJECT\_DESTROYED            | Physical (partial) | 70%         | Partially destroyed {object} found                                           |
| EVT\_OBJECT\_MODIFIED             | Forensic           | 60%         | Tool marks/modifications on {object}                                         |
|                                   |                    |             |                                                                              |
| EVT\_FINANCIAL\_CHANGE            | Documentary        | 90%         | Bank records show {change\_type}                                             |
| EVT\_RELATIONSHIP\_STATUS\_CHANGE | Testimonial        | 50%         | Friend knew about {relationship\_change}                                     |
| EVT\_EMPLOYMENT\_CHANGE           | Documentary        | 95%         | Employment records show {change\_type}                                       |
| EVT\_INJURY                       | Physical           | 70%         | Blood/DNA at scene matches {actor}                                           |
| EVT\_INJURY                       | Testimonial        | 60%         | Witness noticed {actor} was injured                                          |

### Fugitive-Specific Evidence

| Event | Evidence Type | Discovery | Probability | Description Template |
|-------|---------------|-----------|-------------|---------------------|
| EVT_ACTOR_LEAVES | Digital | RequestRecords | 70% | Cell tower shows {actor} moving away from crime scene |
| EVT_ACTOR_LEAVES | Testimonial | CanvassArea | 40% | Witness saw {actor} running/driving away |
| EVT_OBJECT_MOVED | Physical | SearchScene | 60% (if failed) | Hidden {object} discovered at {location} |
| EVT_OBJECT_DESTROYED | Physical | SearchScene | 70% (if failed) | Partially destroyed {object} found |
| EVT_OBJECT_DESTROYED | Forensic | ForensicAnalysis | 50% (if failed) | Burn/chemical residue found |
| EVT_ALIBI_FABRICATION | Testimonial | InterviewWitness | 80% | {accomplice} claims {actor} was with them |

**Note:** These use the same event types as normal activities. During evidence generation, check `actor_states.state == STATE_FUGITIVE` to apply fugitive-specific probabilities and descriptions. The `parent_event_id` linking back to the crime provides additional context.

---

## Crime Generation

**Target: 3-5 crimes per simulation**

**Crime Probability:**
```
base (1%) × need_factor × personality_factor × archetype_bonus
```
- VOLATILE archetype: 3x bonus
- GREEDY archetype: 2x bonus
- Low needs (< 20): +0.2-0.5x factor
- Low morality (< 30): +0.5x factor

**Crime Type Selection:**
- Low money → robbery, burglary, extortion
- High aggression → assault, murder
- Has rival in memory → violence against rival
- Has debtor → extortion

**Pressure Adjustment:**
- If behind target midway through simulation → increase need decay (1.5x)
- If at target (3+) → decrease need decay (0.5x)
- If at max (5) → stop new crime generation

---

## Time Management

**Units:**
- Tick: 15 minutes (configurable)
- Default simulation: 720 hours (30 days) = 2880 ticks

**Time-Based Behavior:**
- Work hours: 9 AM - 5 PM
- Evening: 6 PM - 10 PM
- Night: 10 PM - 6 AM

**Need Decay (per tick):**
- Money: -1 (daily living costs)
- Belonging: -2 if no recent social interaction
- Security: -2 if at night or dangerous location

---

## Main Simulation Loop

```cpp
void sim_run(sim_ctx *ctx) {
    for (tick = 0; tick < total_ticks; tick++) {
        1. Decay all actor needs
        2. Decay all actor memories (salience -= 5%)
        3. For each active actor:
           a. Check state transitions
           b. Select action (weighted by needs + personality + state)
           c. Execute action → create event(s)
           d. Generate evidence for event(s)
           e. Add event to relevant actors' memories
           f. Record stat changes to database
        4. Adjust crime pressure if needed
        5. Check termination (3-5 crimes + post-crime period)
    }
}
```

---

## File Organization

### New Files

| File | Purpose |
|------|---------|
| `include/sim/event_simulation.h` | All enums, structs, public interface |
| `include/sim/actor_state_machine.h` | State transitions, behavior weights |
| `include/sim/actor_memory.h` | Memory operations |
| `include/sim/evidence_gen.h` | Evidence rules and generation |
| `include/sim/sim_db_events.h` | DB functions for new tables |
| `src/sim/event_simulation.cpp` | Main loop, sim_init/run/clear |
| `src/sim/actor_state_machine.cpp` | State transition logic |
| `src/sim/actor_memory.cpp` | Memory system |
| `src/sim/actor_decision.cpp` | Action selection algorithm |
| `src/sim/crime_generation.cpp` | Crime probability, type, target selection |
| `src/sim/evidence_gen.cpp` | Evidence rule processing |
| `src/sim/sim_db_events.cpp` | SQL table creation, inserts |

* Please make sure to add the ./include/sim folder to the included paths in the build script *
* Create a separate list of files to compile for the ./src/sim folder's code files *
I need to split this implementation from the rest of the code so it's easier for me to examine the code before accepting it fully in the project. I might want to reset and try again with a modified plan file in the future.

### Updated Files

| File | Changes |
|------|---------|
| `src/main.cpp` | Add simulation phase, new CLI flags |

---

## Implementation Phases

### 2a: Foundation
- New database tables (sim_db_events.cpp)
- All enums in event_simulation.h
- Core structs (sim_event, actor_runtime, evidence_item, sim_config)
- sim_init() to create actor_runtime from world_ctx

### 2b: Time + Movement
- Time management (tick_to_time, is_work_hours, etc.)
- Basic movement actions (go_home, go_work, go_location)
- EVT_ACTOR_ENTERS/LEAVES events

### 2c: State Machine + Memory
- State transitions with conditions
- Actor memory system (push, decay, get_relevant)
- Need decay per tick

### 2d: Social + Economic
- Social actions (meet, call, message)
- Economic actions (work, purchase, sell)
- Object + receipt creation on purchase

### 2e: Conflict + Crime
- Conflict actions (argue, threaten, fight)
- Crime planning and execution
- Crime type and target selection

### 2f: Fugitive Behavior
- FUGITIVE state implementation
- Actions: flee, hide_evidence, destroy_evidence, lay_low, establish_alibi
- Success/failure outcomes

### 2g: Evidence Generation
- Evidence rules table
- Generate evidence on event creation
- Populate evidence_reveals

### 2h: Integration + Tuning
- Tune crime probability for 3-5 target
- Tune need decay rates
- CLI options (--sim-hours, --target-crimes)
- Integration testing

---

## Key Design Decisions

1. **Stat changes as DB entries**: All changes to needs, relationship strengths recorded with timestamps for observability

2. **Objects created on purchase**: Both the item AND a receipt object for evidence trails

3. **Dynamic relationships**: New relationships can form during simulation (affairs, new rivalries, debts)

4. **Witness generation**: Other actors at same location have probability to witness events

5. **Fugitive behavior**: Perpetrators attempt to flee/conceal, creating or destroying evidence (with success/failure outcomes)

6. **ParentEventId from memory**: Causation tracked by checking actor's memory when they take actions

---

## Verification

After implementation:
1. Run `casegen.exe --size 1 --seed 12345` and verify world + simulation complete
2. Query `events` table to verify 3-5 CrimeCommitted events exist
3. Query `stat_changes` table to verify need/relationship changes are tracked
4. Query `evidence` table to verify evidence generated for crime events
5. Walk parent_event_id chain from crime back to verify causation links
6. Verify fugitive actors have post-crime events (flee, hide_evidence, etc.)
