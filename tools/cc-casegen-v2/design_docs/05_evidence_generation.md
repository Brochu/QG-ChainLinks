# 05 — Evidence Generation & Solvability

## Overview

Evidence is the player's interface to the case. They never see events directly — they find evidence that points to events. Evidence is generated in two phases:

1. **Natural evidence**: Generated automatically during the simulation when events occur (CCTV captures movement, arguments create noise complaints, murders leave forensic traces)
2. **Solvability patching**: After natural generation, we verify the case is solvable and inject minimal additional evidence if gaps exist

---

## Evidence Data Model

```sql
CREATE TABLE evidence (
    evidence_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    event_id        INTEGER NOT NULL REFERENCES sim_events(event_id),
    evidence_type   INTEGER NOT NULL,   -- PHYSICAL, TESTIMONIAL, DOCUMENTARY, DIGITAL
    evidence_subtype INTEGER NOT NULL,  -- specific kind (see below)
    landmark_id     INTEGER NOT NULL,   -- where to find it
    room_id         INTEGER,            -- which room (nullable = landmark-level)
    actor_id        INTEGER,            -- who carries it (for testimonial, carried physical)
    title           TEXT NOT NULL,       -- short label: "CCTV Footage - Bar Counter"
    description     TEXT NOT NULL,       -- what the player reads when examining
    hidden_level    INTEGER NOT NULL DEFAULT 0,  -- 0=obvious, 1=search, 2=examine
    reveals_json    TEXT,               -- structured data about what this evidence proves
    is_critical     INTEGER NOT NULL DEFAULT 0   -- 1 if needed for solvability
);

CREATE INDEX idx_evi_landmark ON evidence(landmark_id);
CREATE INDEX idx_evi_room     ON evidence(room_id);
CREATE INDEX idx_evi_actor    ON evidence(actor_id);
CREATE INDEX idx_evi_event    ON evidence(event_id);
```

### Evidence Types and Subtypes

```cpp
enum evidence_type : u8 {
    EVI_PHYSICAL,       // tangible objects and traces
    EVI_TESTIMONIAL,    // witness accounts (carried by actor)
    EVI_DOCUMENTARY,    // written records, logs, receipts
    EVI_DIGITAL         // phone records, CCTV, electronic data
};

enum evidence_subtype : u8 {
    // Physical
    EVISUB_BODY,                // victim's body
    EVISUB_MURDER_WEAPON,       // the weapon used
    EVISUB_BLOOD_TRACE,         // blood at crime scene
    EVISUB_FINGERPRINTS,        // fingerprints on objects
    EVISUB_FOOTPRINTS,          // shoe prints at scene
    EVISUB_PERSONAL_ITEM,       // dropped item (wallet, phone, lighter)
    EVISUB_STOLEN_OBJECT,       // the stolen item itself
    EVISUB_FORCED_ENTRY,        // broken lock, jimmied window
    EVISUB_TOOL_MARKS,          // marks from break-in tool

    // Testimonial
    EVISUB_EYEWITNESS,          // "I saw X do Y at Z"
    EVISUB_EARWITNESS,          // "I heard shouting/a crash"
    EVISUB_CHARACTER_TESTIMONY, // "X and Y hated each other"
    EVISUB_ALIBI_TESTIMONY,     // "X was here at that time"
    EVISUB_HEARSAY,             // "I heard that X owed Y money"

    // Documentary
    EVISUB_SIGN_IN_LOG,         // hotel/office sign-in sheet
    EVISUB_RECEIPT,             // purchase receipt
    EVISUB_DEBT_RECORD,         // loan document, IOU
    EVISUB_NOISE_COMPLAINT,     // filed complaint about an argument
    EVISUB_EMPLOYMENT_RECORD,   // who works where

    // Digital
    EVISUB_CCTV_FOOTAGE,        // security camera recording
    EVISUB_PHONE_RECORD,        // call log showing who called whom
    EVISUB_TEXT_MESSAGE,         // threatening text, meeting arrangement
    EVISUB_BANK_TRANSACTION     // money transfer record
};
```

---

## Natural Evidence Generation Rules

Each event type has rules that automatically create evidence. These fire during the simulation (or during a post-simulation pass over the event log).

### Movement Events (EVT_ARRIVE, EVT_DEPART)

| Condition | Evidence Created | Hidden Level |
|-----------|-----------------|--------------|
| Landmark has cameras | CCTV footage showing actor at timestamp | 1 (search CCTV system) |
| Landmark is hotel/office with sign-in | Sign-in log entry | 0 (obvious) |
| Landmark is bar/restaurant | Receipt if actor purchased something (50% chance) | 1 (search counter/register) |
| Night arrival at unusual location | Neighbor/guard eyewitness (if anyone present) | testimonial on witness |

**CCTV Evidence Example:**
```
title: "CCTV Recording — Bar Entrance"
description: "Grainy footage from [time]. A [build] [sex] with [hair] hair
             enters the bar. They appear [calm/agitated/intoxicated]."
reveals: { person_hint: [height, build, hair], time: [arrival_time], location: [landmark] }
hidden_level: 1
```

Note: CCTV descriptions use physical attributes, not names. The player must cross-reference descriptions with known suspects.

### Social Events (EVT_ARGUMENT, EVT_MEET, EVT_PHONE_CALL)

| Condition | Evidence Created | Hidden Level |
|-----------|-----------------|--------------|
| EVT_ARGUMENT at public location | Earwitness testimony from bystanders | testimonial on witness |
| EVT_ARGUMENT at residential, late night | Noise complaint record at police station or building manager | 1 |
| EVT_PHONE_CALL | Phone record showing caller/callee/duration | 1 (search phone records) |
| EVT_PHONE_CALL with threatening content | Text message if actor sent a follow-up threat | 2 (examine phone) |
| EVT_MEET at public location | Eyewitness: "I saw X and Y together at Z" | testimonial on witness |

**Witness Accuracy:**
Testimony quality depends on conditions:
- Same room + daytime + sober witness → "clear" (names, descriptions, actions)
- Same room + nighttime OR intoxicated → "partial" (physical description only, fuzzy on time)
- Same landmark, different room → "distant" (heard something, can't describe who)
- Exterior + night → "vague" (saw a figure, approximate time)

### Economic Events (EVT_TRANSACTION, EVT_DEBT_CREATED)

| Condition | Evidence Created | Hidden Level |
|-----------|-----------------|--------------|
| EVT_TRANSACTION at a shop/bank | Receipt or bank transaction record | 1 |
| EVT_DEBT_CREATED | Debt record (IOU, text message, witness to the loan) | 1-2 |
| EVT_DEBT_CREATED with argument | Character testimony from bystander about the financial dispute | testimonial |

### Crime Events (EVT_MURDER, EVT_THEFT)

**EVT_MURDER always generates:**

| Evidence | Description | Hidden Level | Location |
|----------|-------------|--------------|----------|
| Body | Victim's body with visible injuries | 0 | Crime scene room |
| Blood traces | Blood spatter on floor/walls | 1 | Crime scene room |
| Murder weapon | If not hidden: at scene. If hidden: at hiding location | 0 or 2 | Scene or hiding spot |
| Fingerprints on weapon | Forensic evidence linking killer to weapon | 2 (examine weapon) | On the weapon |
| Footprints | If exterior/wet floor: shoe prints leading away | 1 | Crime scene room |
| Victim's phone | Last calls/texts may show threats or meeting arrangements | 1 | On victim or in room |

**EVT_THEFT always generates:**

| Evidence | Description | Hidden Level | Location |
|----------|-------------|--------------|----------|
| Missing object report | Owner or associate reports the item missing | 0 (interview-based) | Owner's location |
| Forced entry marks | If EVT_BREAK_IN preceded: visible damage | 0-1 | Entry point |
| Stolen object | On the thief or at their home or at pawn shop | 1-2 | Variable |
| Tool marks | If tools used for break-in | 1 | Entry point |

### Aftermath Events

| Event | Evidence Created | Hidden Level |
|-------|-----------------|--------------|
| EVT_HIDE_OBJECT | Object exists at hiding location with OBJ_FLAG_HIDDEN | 2 |
| EVT_DISPOSE_EVIDENCE | Object destroyed — creates absence (weapon missing from kitchen, empty holster) | 1 |
| EVT_WITNESS | Testimonial evidence on the witness actor (see accuracy rules above) | — |
| EVT_BODY_DISCOVERED | Discoverer testimony about finding the body and their reaction | testimonial |
| EVT_FLEE | CCTV (if cameras) showing person leaving rapidly. Possibly eyewitness at exit | 1 |

---

## The `reveals_json` Field

Each evidence piece has structured data about what it proves. This is used by the solvability checker and by the game to track what the player knows:

```json
{
    "type": "person_at_location",
    "person_hint": {
        "sex": "M",
        "height": "tall",
        "build": "heavy",
        "hair": "dark"
    },
    "person_id": 12,
    "landmark_id": 7,
    "time": 2715,
    "confidence": "partial"
}
```

```json
{
    "type": "relationship",
    "from_person": 12,
    "to_person": 5,
    "relationship": "debtor",
    "detail": "Marco owed Sergei a significant amount of money"
}
```

```json
{
    "type": "object_link",
    "object_id": 45,
    "person_id": 12,
    "link_type": "fingerprints",
    "detail": "Fingerprints on the knife handle match Marco Valenti"
}
```

---

## Solvability Guarantee

After evidence generation, we verify the case is solvable. A case is solvable if a player can:

1. **Identify the perpetrator** — At least one evidence chain leads to the killer/thief
2. **Establish motive** — At least one evidence piece explains WHY
3. **Reconstruct the chain** — Each step in the causal chain has at least one piece of evidence

### Critical Path Verification Algorithm

```
function verify_solvability(case):
    // Step 1: Crime scene must have obvious evidence
    scene_evidence = query evidence WHERE landmark_id = crime_landmark AND hidden_level = 0
    if scene_evidence is empty → FAIL: "No obvious evidence at crime scene"

    // Step 2: Perpetrator must be identifiable
    perp_evidence = query evidence WHERE reveals_json links to perpetrator_id
    if perp_evidence is empty → FAIL: "Perpetrator not identifiable"

    // Step 3: Each chain step must have evidence
    for each event in causal_chain:
        event_evidence = query evidence WHERE event_id = event.event_id
        if event_evidence is empty:
            // Check if this event is "bridgeable" — can the player infer it
            // from adjacent events' evidence?
            if not bridgeable → FAIL: "Gap at chain step {event}"

    // Step 4: Motive must be provable
    motive_evidence = query evidence WHERE reveals_json.type = 'relationship'
        AND involves perpetrator AND victim
    if motive_evidence is empty → FAIL: "No motive evidence"

    return PASS
```

### Bridgeable Events

Not every chain step needs its own evidence. Some events are inferrable:

- EVT_DEPART → If the player knows actor was at Location A at 9 PM and at Location B at 10 PM, departure is implied
- EVT_ACQUIRE_WEAPON → If the weapon is found with the perpetrator's fingerprints and it's from their home, acquisition is implied
- EVT_MEET → If two actors were at the same location, meeting is implied

Only non-inferrable events need direct evidence: arguments, threats, the crime itself, and hiding evidence.

---

## Evidence Patching

When a solvability gap is detected, we inject evidence that is consistent with the simulation state. We never invent events — we only create evidence for events that actually happened.

### Patching Strategies

**Gap: No one saw the perpetrator near the crime scene**
- Check: Was any actor at the same landmark within 2 hours?
- If yes: Give that actor a testimonial evidence piece (they noticed someone matching the perpetrator's description)
- If no: Check if landmark has cameras → add CCTV footage
- Last resort: Add a neighbor who "saw someone leaving in a hurry" (vague eyewitness)

**Gap: Motive not provable**
- Check: Do any actors know about the perpetrator-victim relationship?
- If yes: Give one a character testimony ("Everyone knew they hated each other")
- If no: Add a documentary trace — debt record at a bank, text message on victim's phone, filed complaint

**Gap: Missing chain step (e.g., weapon acquisition has no evidence)**
- Check: Was anyone at the weapon's original location when it was taken?
- If yes: Add testimonial ("I noticed the kitchen knife was missing")
- If no: Add physical evidence — empty knife block, disturbed drawer, CCTV at home

**Gap: Crime scene has no obvious evidence (hidden_level 0)**
- This shouldn't happen for murder (body is always obvious)
- For theft: Ensure the missing object generates a "something is gone" notice at hidden_level 0

### Patching Rules

1. **Minimum intervention**: Add the fewest evidence pieces possible to close the gap
2. **Consistency**: Only reference events that actually happened in the simulation
3. **Realistic**: Evidence types must make sense for the context (no CCTV in a park, no sign-in log at a bar)
4. **Mark as critical**: Patched evidence gets `is_critical = 1` so we know it was injected

---

## Difficulty Scaling

Difficulty is controlled by adjusting evidence density and hidden levels AFTER the solvability check passes. The solvability check ensures the case is always solvable regardless of difficulty — we only change how hard it is to find the necessary evidence.

### Difficulty Levels

```cpp
enum difficulty_level : u8 {
    DIFFICULTY_EASY   = 1,
    DIFFICULTY_MEDIUM = 2,
    DIFFICULTY_HARD   = 3,
    DIFFICULTY_EXPERT = 4
};
```

### Evidence Adjustment by Difficulty

| Adjustment | Easy | Medium | Hard | Expert |
|------------|------|--------|------|--------|
| Redundant evidence per chain step | 3+ | 2 | 1 | 1 |
| Non-critical evidence hidden_level | 0-1 | 0-1 | 1-2 | 2 |
| Critical evidence hidden_level | 0 | 0-1 | 1 | 1-2 |
| Witness testimony accuracy | clear | clear/partial | partial | partial/vague |
| Extra red herring actors | 0 | 1 | 2 | 3+ |
| CCTV clarity | name-level | description-level | silhouette | degraded |
| Interview topic gating | none | light | moderate | heavy |
| Misleading witnesses | 0 | 0 | 0-1 | 1-2 |

### Applying Difficulty

```
function apply_difficulty(case, level):
    // Remove redundant evidence for higher difficulty
    for each chain_step in case.chain:
        evidence_for_step = query evidence WHERE event_id = chain_step.event_id
        if level >= HARD and count(evidence_for_step) > 1:
            mark least-obvious piece as non-essential
            if level == EXPERT: set hidden_level = 2

    // Increase hidden levels
    for each evidence in case.evidence:
        if not is_critical:
            evidence.hidden_level = min(2, evidence.hidden_level + (level - 1))

    // Degrade witness accuracy
    for each testimonial in case.evidence WHERE type = TESTIMONIAL:
        if level >= HARD:
            degrade accuracy one level (clear → partial, partial → vague)

    // Add misleading witnesses at EXPERT
    if level == EXPERT:
        inject 1-2 false testimonials that point away from the real perpetrator
        (actor claims to have seen someone else, conflicting timeline)
```

### Misleading Witnesses (Expert only)

At expert difficulty, some witnesses provide partially false information:
- An intoxicated witness misidentifies the perpetrator's hair color
- A nervous witness gets the time wrong by an hour
- A rival of the perpetrator claims they were elsewhere (false alibi)

These are marked internally (the game knows they're unreliable) but the player must figure out which testimonies to trust by cross-referencing with physical evidence.

---

## Evidence Count Targets

| Difficulty | Physical | Testimonial | Documentary | Digital | Total |
|------------|----------|-------------|-------------|---------|-------|
| Easy | 4-6 | 6-8 | 3-4 | 3-4 | 16-22 |
| Medium | 3-5 | 4-6 | 2-3 | 2-3 | 11-17 |
| Hard | 2-4 | 3-5 | 1-2 | 1-2 | 7-13 |
| Expert | 2-3 | 2-4 | 1 | 1 | 6-9 |

These are targets, not hard limits. The natural evidence generation may produce more or less depending on what happened in the simulation. The difficulty adjuster trims or supplements to hit the range.
