# 07 — JSON Output Format

## Overview

The final output of the case generator is a self-contained JSON file that the game loads to present a complete playable case. It contains everything needed: locations, rooms, actors, evidence, interviews, and the solution for validation.

This is an interim format. A custom binary format may replace it later, but the structure and fields will remain the same.

---

## Top-Level Structure

```json
{
    "version": 1,
    "generator": "cc-casegen-v2",
    "seed": 1234567890,
    "case_id": "a7b3c9d1",

    "meta": { ... },
    "time_window": { ... },
    "city": { ... },
    "persons": [ ... ],
    "objects": [ ... ],
    "evidence": [ ... ],
    "interviews": [ ... ],
    "solution": { ... }
}
```

---

## `meta` — Case Metadata

```json
{
    "meta": {
        "crime_type": "murder",
        "difficulty": 3,
        "interest_score": 142,
        "num_locations": 6,
        "num_rooms": 18,
        "num_persons": 8,
        "num_evidence": 17,
        "num_interviews": 6,
        "estimated_minutes": 40
    }
}
```

| Field | Type | Description |
|-------|------|-------------|
| crime_type | string | "murder" or "theft" |
| difficulty | int | 1-4 (easy/medium/hard/expert) |
| interest_score | int | Raw score from case extraction |
| num_locations | int | Explorable landmarks |
| num_rooms | int | Total rooms across all locations |
| num_persons | int | Persons of interest |
| num_evidence | int | Total evidence pieces |
| num_interviews | int | Interviewable actors |
| estimated_minutes | int | Target play session length |

---

## `time_window` — Investigation Period

```json
{
    "time_window": {
        "start": 19080,
        "end": 20235,
        "crime_time": 20175,
        "display": {
            "start": "Day 13, 6:00 PM",
            "end": "Day 14, 1:15 PM",
            "crime": "Day 14, 1:15 AM"
        }
    }
}
```

All times are in `sim_time` format (minutes since Day 0, 00:00). The `display` object provides human-readable versions for the game UI.

---

## `city` — Locations and Rooms

### Districts

```json
{
    "city": {
        "districts": [
            {
                "id": 1,
                "name": "Dockside Quarter",
                "type": "docks",
                "description": "A rough waterfront area with warehouses and dive bars."
            }
        ],
        "locations": [ ... ],
        "transit": [ ... ]
    }
}
```

### Locations

```json
{
    "id": 7,
    "district_id": 1,
    "name": "The Rusty Anchor",
    "type": "bar",
    "address": "42 Harbor Street",
    "description": "A dimly lit bar popular with dockworkers. Smells of salt and stale beer.",
    "hours": "evening",
    "has_cameras": true,
    "rooms": [
        {
            "id": 14,
            "name": "Bar Counter",
            "description": "A long oak counter with worn leather stools. Bottles line the back wall.",
            "is_exterior": false,
            "is_private": false,
            "evidence_ids": [3, 8, 15],
            "interactable_objects": [
                {
                    "object_id": 22,
                    "name": "Cash register",
                    "description": "An old mechanical register, still in use.",
                    "examine_evidence_id": null
                },
                {
                    "object_id": 23,
                    "name": "CCTV monitor",
                    "description": "A small screen showing grainy feeds from two cameras.",
                    "examine_evidence_id": 8
                }
            ]
        },
        {
            "id": 15,
            "name": "Back Room",
            "description": "A small room with a card table and a few chairs. Door is usually locked.",
            "is_exterior": false,
            "is_private": true,
            "evidence_ids": [],
            "interactable_objects": []
        },
        {
            "id": 16,
            "name": "Alley",
            "description": "A narrow alley behind the bar. Dumpsters, crates, a single flickering light.",
            "is_exterior": true,
            "is_private": false,
            "evidence_ids": [19],
            "interactable_objects": [
                {
                    "object_id": 30,
                    "name": "Dumpster",
                    "description": "A rusted metal dumpster. Something might be hidden inside.",
                    "examine_evidence_id": 19
                }
            ]
        }
    ]
}
```

### Transit

Travel times between locations for the player (used if the game has a time-based investigation system):

```json
{
    "transit": [
        { "from": 7, "to": 15, "minutes": 10 },
        { "from": 7, "to": 2, "minutes": 25 },
        { "from": 15, "to": 2, "minutes": 30 }
    ]
}
```

---

## `persons` — People in the Case

```json
{
    "id": 12,
    "name": "Marco Valenti",
    "alias": null,
    "sex": "M",
    "age": 34,
    "height": "tall",
    "build": "heavy",
    "hair_color": "dark",
    "occupation": "Dockworker",
    "home_location_id": 2,
    "work_location_id": 15,
    "phone_number": "+1-555-0147",
    "is_victim": true,
    "is_alive": false,
    "can_interview": false,
    "interview_location_id": null,
    "known_relationships": [
        {
            "target_person_id": 5,
            "type": "coworker",
            "strength": 40,
            "is_public": true,
            "description": "They work the same shift at the shipyard."
        },
        {
            "target_person_id": 8,
            "type": "debtor",
            "strength": -35,
            "is_public": false,
            "description": "Marco owes a significant debt."
        }
    ],
    "schedule_hints": [
        "Works at the shipyard weekdays, 8 AM to 5 PM.",
        "Usually at The Rusty Anchor on weekday evenings."
    ]
}
```

| Field | Description |
|-------|-------------|
| is_victim | Was this person the victim of the crime? |
| is_alive | Dead victims can't be interviewed |
| can_interview | Is this person available for interview? |
| interview_location_id | Where to find them for interviews |
| known_relationships | What the player can learn about — `is_public` means it's common knowledge, `!is_public` requires evidence to discover |
| schedule_hints | Natural language routine descriptions (help player identify alibi inconsistencies) |

---

## `objects` — Trackable Items

```json
{
    "id": 45,
    "name": "Kitchen knife",
    "type": "weapon_knife",
    "description": "A 20cm chef's knife with a black handle. The blade shows signs of recent use.",
    "owner_name": "Marco Valenti",
    "found_at": {
        "location_id": 16,
        "room_id": 16,
        "room_name": "Alley"
    },
    "is_evidence": true,
    "flags": ["bloody", "fingerprinted", "hidden"]
}
```

Objects that are evidence link back to evidence entries. Objects that aren't evidence are scenery/red herrings.

---

## `evidence` — Everything the Player Can Find

```json
{
    "id": 8,
    "type": "digital",
    "subtype": "cctv_footage",
    "title": "CCTV Recording — Bar Entrance",
    "found_at": {
        "location_id": 7,
        "location_name": "The Rusty Anchor",
        "room_id": 14,
        "room_name": "Bar Counter"
    },
    "hidden_level": 1,
    "discovery": {
        "action": "examine",
        "target_object_id": 23,
        "target_name": "CCTV monitor"
    },
    "content": {
        "description": "Grainy footage from 9:47 PM. A tall, heavy-set man with dark hair enters the bar. He sits at the counter. At 10:12 PM, a shorter man approaches and a heated argument begins. The tall man slams his fist on the counter and storms out at 10:23 PM.",
        "reveals": [
            {
                "type": "person_at_location",
                "person_id": 5,
                "person_hint": { "sex": "M", "height": "tall", "build": "heavy", "hair": "dark" },
                "location_id": 7,
                "time": 19787,
                "time_display": "Day 13, 9:47 PM"
            },
            {
                "type": "event_hint",
                "event_type": "argument",
                "participants": [5, 12],
                "time": 19812,
                "time_display": "Day 13, 10:12 PM"
            }
        ]
    },
    "is_critical": true,
    "chain_step": 2
}
```

### Evidence Discovery

| hidden_level | Player Action Required |
|-------------|----------------------|
| 0 | Visible immediately upon entering the room |
| 1 | Must "search" the room (interact with objects, look around carefully) |
| 2 | Must "examine" a specific object in the room (zoom in, inspect closely) |

The `discovery` field tells the game exactly how to present the evidence:
- `action: "visible"` → shown when player enters room
- `action: "search"` → found when player searches the room
- `action: "examine"` → found when player examines a specific object

### Evidence Reveals

The `reveals` array tells the game what information this evidence provides. Types:

| reveal type | Fields | Meaning |
|-------------|--------|---------|
| person_at_location | person_id/hint, location_id, time | Someone was somewhere at some time |
| event_hint | event_type, participants, time | Something happened between people |
| relationship | from_person, to_person, rel_type, detail | Two people have a connection |
| object_link | object_id, person_id, link_type | An object is connected to a person (fingerprints, ownership) |
| motive | person_id, motive_type, detail | Why someone might commit a crime |
| timeline | entries[] | Sequence of times and locations for a person |
| contradiction | claim_evidence_id, contradicts | This evidence contradicts another piece |

---

## `interviews` — Dialogue Trees

```json
{
    "person_id": 19,
    "person_name": "Linda Chen",
    "role": "bartender",
    "location_id": 7,
    "cooperation": "high",
    "topics": [
        {
            "key": "general",
            "prompt": "Tell me about yourself.",
            "response": "I'm Linda. I've been bartending at the Anchor for about three years now. I know most of the regulars.",
            "requires_evidence": null,
            "unlocks": ["evening", "regulars"],
            "reveals_evidence_id": null,
            "reliability": 100
        },
        {
            "key": "evening",
            "prompt": "What happened that evening?",
            "response": "It was a normal night until those two started going at it. One of my regulars and some guy. It got loud around ten. I thought they were going to throw punches.",
            "requires_evidence": null,
            "unlocks": ["argument_detail", "regular_identity"],
            "reveals_evidence_id": 24,
            "reliability": 100
        },
        {
            "key": "argument_detail",
            "prompt": "What were they arguing about?",
            "response": "Money, I think. The regular — Sergei — kept saying something about 'what you owe me.' The other guy was trying to calm him down but Sergei wasn't having it.",
            "requires_evidence": [8],
            "unlocks": ["sergei_temper"],
            "reveals_evidence_id": 25,
            "reliability": 90
        },
        {
            "key": "sergei_temper",
            "prompt": "Is Sergei normally aggressive like that?",
            "response": "He's got a temper, yeah. Especially when he's been drinking. But I've never seen him that angry before. Whatever that guy owed him, it was serious.",
            "requires_evidence": [24, 25],
            "unlocks": [],
            "reveals_evidence_id": 26,
            "reliability": 100
        },
        {
            "key": "regular_identity",
            "prompt": "Who is this regular?",
            "response": "Sergei Kozlov. He comes in most evenings. Lives somewhere in the Docks area, I think. Works at the shipyard.",
            "requires_evidence": null,
            "unlocks": ["sergei_temper"],
            "reveals_evidence_id": 27,
            "reliability": 100
        }
    ]
}
```

### Interview Fields

| Field | Description |
|-------|-------------|
| cooperation | "high", "medium", "low", "hostile" — affects tone and detail level |
| topics[].key | Unique identifier for this topic |
| topics[].prompt | What the player sees as a conversation option |
| topics[].response | What the actor says (may contain inaccuracies at low reliability) |
| topics[].requires_evidence | Array of evidence_ids needed to unlock, null = always available |
| topics[].unlocks | Topic keys that become available after this topic |
| topics[].reveals_evidence_id | New evidence piece created by this conversation (null if none) |
| topics[].reliability | 0-100, how accurate this response is (100 = fully truthful) |

---

## `solution` — Answer Key

Not shown to the player. Used by the game to validate deductions and score the player.

```json
{
    "solution": {
        "perpetrator_id": 5,
        "perpetrator_name": "Sergei Kozlov",
        "victim_id": 12,
        "victim_name": "Marco Valenti",
        "crime_type": "murder",
        "crime_time": 20175,
        "crime_time_display": "Day 14, 1:15 AM",
        "crime_location_id": 15,
        "crime_room_id": 31,
        "motive": {
            "type": "debt_dispute",
            "description": "Marco owed Sergei a large sum and refused to pay. Escalating confrontations over several days led to murder."
        },

        "chain": [
            {
                "step": 1,
                "event_type": "debt_created",
                "time": 4320,
                "time_display": "Day 3, 12:00 PM",
                "description": "Marco borrows money from Sergei at the docks.",
                "actors": [12, 5],
                "location_id": 18,
                "critical_evidence_ids": [25, 40]
            },
            {
                "step": 2,
                "event_type": "argument",
                "time": 19812,
                "time_display": "Day 13, 10:12 PM",
                "description": "Sergei confronts Marco about the debt at The Rusty Anchor.",
                "actors": [5, 12],
                "location_id": 7,
                "critical_evidence_ids": [8, 24]
            },
            {
                "step": 3,
                "event_type": "threat",
                "time": 19823,
                "time_display": "Day 13, 10:23 PM",
                "description": "Sergei threatens Marco outside the bar.",
                "actors": [5, 12],
                "location_id": 7,
                "critical_evidence_ids": [27, 28]
            },
            {
                "step": 4,
                "event_type": "acquire_weapon",
                "time": 19890,
                "time_display": "Day 13, 11:30 PM",
                "description": "Sergei takes a knife from his apartment.",
                "actors": [5],
                "location_id": 2,
                "critical_evidence_ids": [29, 30]
            },
            {
                "step": 5,
                "event_type": "murder",
                "time": 20175,
                "time_display": "Day 14, 1:15 AM",
                "description": "Sergei kills Marco at the warehouse.",
                "actors": [5, 12],
                "location_id": 15,
                "critical_evidence_ids": [31, 32, 33]
            }
        ],

        "scoring": {
            "identify_perpetrator": {
                "points": 30,
                "requires": "Player selects the correct perpetrator"
            },
            "identify_motive": {
                "points": 20,
                "requires": "Player identifies debt/financial dispute as the motive"
            },
            "reconstruct_chain": {
                "points": 50,
                "per_step": 10,
                "requires": "Player correctly orders and links the chain events"
            },
            "max_score": 100
        }
    }
}
```

### Scoring System

The player's final score is based on three components:

| Component | Points | How it works |
|-----------|--------|-------------|
| Identify perpetrator | 30 | Binary: correct or not |
| Identify motive | 20 | Match motive type (debt, revenge, opportunity, etc.) |
| Reconstruct chain | 50 | 10 points per correctly placed chain step (order + participants) |
| **Max** | **100** | |

Bonus modifiers (optional, game can decide):
- Found all evidence: +10 bonus
- No incorrect accusations: +5 bonus
- Solved under time target: +5 bonus

---

## Complete Example File

A minimal but complete case file:

```json
{
    "version": 1,
    "generator": "cc-casegen-v2",
    "seed": 42,
    "case_id": "demo_001",
    "meta": {
        "crime_type": "murder",
        "difficulty": 2,
        "interest_score": 125,
        "num_locations": 4,
        "num_rooms": 12,
        "num_persons": 6,
        "num_evidence": 14,
        "num_interviews": 4,
        "estimated_minutes": 40
    },
    "time_window": {
        "start": 19080,
        "end": 20400,
        "crime_time": 20175,
        "display": {
            "start": "Day 13, 6:00 PM",
            "end": "Day 14, 4:00 PM",
            "crime": "Day 14, 1:15 AM"
        }
    },
    "city": {
        "districts": [],
        "locations": [],
        "transit": []
    },
    "persons": [],
    "objects": [],
    "evidence": [],
    "interviews": [],
    "solution": {}
}
```

*(Full content for each array follows the schemas defined above)*

---

## File Size Estimates

| Component | Typical Size |
|-----------|-------------|
| meta + time_window | ~200 bytes |
| city (6 locations, 18 rooms) | ~8 KB |
| persons (8 actors) | ~4 KB |
| objects (10-15 items) | ~2 KB |
| evidence (15-20 pieces) | ~6 KB |
| interviews (6 actors, ~5 topics each) | ~8 KB |
| solution | ~3 KB |
| **Total** | **~30-35 KB** |

Compact enough to load instantly. No streaming or pagination needed.

---

## Versioning

The `version` field allows forward compatibility. If the format changes:
- `version: 1` — current format as described
- Future versions increment this number
- The game checks version and uses the appropriate parser
- Breaking changes require a new major version; additive changes (new optional fields) can be added without version bump
