# Game Vision & Information Matrix Design

## Game Vision Summary

**Core Inspiration Triangle:**
- **Obra Dinn**: Deduction-based gameplay, filling in blanks from gathered information
- **Clue (boardgame)**: Replayability, procedural/randomized cases
- **FBI Files**: Narrative immersion, feeling like a real investigation

**Session Design:**
- Target: 25-35 minutes per case
- Roguelike feel: failing doesn't punish heavily, next case starts immediately
- Requires large quantity of procedurally generated cases

**Core Systems:**
1. **Information Matrix** - Models investigator's current knowledge/beliefs (can be WRONG)
2. **Chain of Events** - Blanks to fill from matrix, represents the crime sequence

---

## Information Matrix: Entity Types Discussion

### The Question
What entity types should the Information Matrix track?

Initial proposal: PEOPLE, LOCATIONS, EVENTS
Additional consideration: OBJECTS

### Analysis

**PEOPLE** - Essential, no debate needed.
- Victims, suspects, witnesses, investigators, etc.
- Most fields are straightforward (name, description, relationships, alibis, motives)

**LOCATIONS** - Essential, no debate needed.
- Crime scenes, alibi locations, evidence locations, hideouts
- Fields: address, type, access, connected locations

**OBJECTS** - **Recommended: YES**

Reasoning:
- Murder weapons are central to many cases
- Evidence items connect people to locations (fingerprints on a glass, blood on a knife)
- Tools of the crime (lockpicks, rope, vehicles)
- Personal belongings place suspects at scenes
- In your chain example: "broke into...using ____" and "murdered...using ____" both expect OBJECTS

Without objects as a tracked type, the player couldn't deduce things like:
- "The crowbar found at the scene matches marks on the door"
- "The victim's necklace was found in the suspect's car"

**EVENTS** - **Recommended: YES, but with clarity on purpose**

This is the tricky one. Here's why I think events need to be first-class entities:

Events are composite - they answer "Who did What, Where, When" - but they need to be tracked because:
1. **Alibis are event claims**: "I was at the bar from 8-10pm" is an EVENT that can be verified/falsified
2. **Witness accounts are events**: "I saw a man running from the building at 9pm"
3. **The chain of events is a subset**: The crime itself is events, but there are peripheral events too
4. **Contradictions matter**: Two events that can't both be true drive deduction

The key distinction:
- **Chain of Events** (game system): The specific crime sequence with blanks
- **Events** (matrix entity): Individual occurrences the investigator learns about

### Proposed Entity Types

| Type | Purpose | Example Fields |
|------|---------|----------------|
| PEOPLE | Individuals involved | Name, role, description, occupation, relationships, alibis, motives |
| LOCATIONS | Places in the case | Name, type, access, who frequents, evidence found |
| OBJECTS | Physical items | Name, type, owner, location found, condition, significance |
| EVENTS | Things that happened | Description, participants, location, time, source, verified? |

### Open Questions
1. ~~Should RELATIONSHIPS be a separate type or fields on PEOPLE?~~ → DECIDED: Fields
2. ~~Should ORGANIZATIONS be tracked (for gang cases, corporate crimes)?~~ → DECIDED: Field on PEOPLE
3. How do we handle TIME - as event fields or standalone?
4. ~~Should EVIDENCE be a subtype of OBJECTS or its own type?~~ → DECIDED: Neither

---

## Key Design Decision: Evidence vs Matrix

**Evidence is NOT a matrix entity type.**

Evidence is a *gameplay mechanism* that unlocks information in the matrix:
- Physical evidence (knife, document, etc.) → unlocks OBJECT entries + associated field values
- Witness testimony → unlocks EVENT entries
- Forensic analysis → unlocks field values on existing entries

The matrix represents **what the investigator knows**.
Evidence represents **how they learned it**.

This separation keeps the matrix clean as pure knowledge state.

---

## Confirmed Entity Types

| Type | Purpose | Cross-references |
|------|---------|------------------|
| PEOPLE | Individuals involved | Other people (relationships), locations (frequents), orgs (affiliation) |
| LOCATIONS | Places in the case | People (who has access), objects (found here) |
| OBJECTS | Physical items | People (owner), locations (found at), events (used in) |
| EVENTS | Things that happened | People (participants), locations (where), objects (involved), time |

---

## PEOPLE Fields

Design note: First names only to avoid linking to real individuals.

| Field | Type | Purpose |
|-------|------|---------|
| Name | string | First name only |
| Alias | string | Known alternate names/nicknames |
| Age | number | |
| Height | string | Physical descriptor for witness matching |
| Build | string | Weight/body type descriptor |
| Hair Color | string | Physical descriptor for witness matching |
| Home Location | ref→LOCATION | Where they live |
| Work Location | ref→LOCATION | Where they work |
| Occupation | string | Job title/profession |
| Phone Number | string | For phone record evidence |
| Email Address | string | For email/digital evidence |
| Role | enum | victim, suspect, witness, person_of_interest, other |
| Relationships | ref→PEOPLE[] | Connections to other people (with relationship type) |
| Org/Affiliation | string | Gang, company, group membership |
| Alibi | ref→EVENT | Claimed whereabouts during crime |
| Motive | ref→EVENT | Event that gives them reason (grounded in something that happened) |
| Opportunity | string/bool | Access to crime scene, means |

**Gameplay uses:**
- Physical descriptors enable witness account filtering ("tall man with dark hair")
- Contact info enables connection discovery via phone/email evidence
- Alibi as EVENT ref means alibis can be verified/falsified
- Motive as EVENT ref makes motives deducible, not arbitrary

---

## LOCATIONS Fields

Design note: Keep granularity at building/unit level (one apartment, not separate rooms). Add detail fields if specific areas matter.

| Field | Type | Purpose |
|-------|------|---------|
| Name | string | Descriptive name ("Mike's Apartment", "Harbor Warehouse") |
| Address | string | Street address or general area |
| Type | enum | residence, business, public, outdoor, vehicle |
| Owner | ref→PEOPLE | Who owns the property |
| Resident(s) | ref→PEOPLE[] | Who lives/works there regularly |
| AccessControl | string | Security description (locks, front desk, keypad, none) |
| OperationHours | string | When accessible (24/7, business hours, etc.) |

**Gameplay uses:**
- AccessControl feeds into "method of entry" deductions (picked lock, had key, bypassed security)
- Resident(s) establishes who would normally be at a location
- Type helps filter locations for Chain of Events blanks (fled to a ___ type location)

---

## OBJECTS Fields

| Field | Type | Purpose |
|-------|------|---------|
| Name | string | Descriptive name ("Kitchen Knife", "Gold Watch") |
| Type | enum | weapon, tool, vehicle, document, personal_item, other |
| SerialNumber | string | Unique identifier (gun registration, electronics) |
| Size | enum | small, medium, large (not precise measurements) |
| Color | string | Physical descriptor |
| Material | string | What it's made of |
| Condition | string | Forensic state (damaged, bloody, clean, etc.) |
| Owner | ref→PEOPLE | Who owns it |
| Location | ref→LOCATION | Where the item is located/found |

**Design note on vehicles:**
Vehicles can be represented as two separate matrix entries:
- OBJECT entry for the vehicle itself (getaway car, weapon in hit-and-run)
- LOCATION entry for the vehicle's interior (crime scene, evidence found inside)

Link them via fields if needed later. Keep separate for now.

**Gameplay uses:**
- SerialNumber enables ownership traces (gun registration, stolen property)
- Color/Size/Material for matching witness descriptions ("saw him carrying a large black bag")
- Condition provides forensic clues (bloody knife, damaged lock)
- Location establishes where evidence was found

---

## EVENTS Fields

| Field | Type | Purpose |
|-------|------|---------|
| Type | enum | TBD - this drives case variety, needs careful definition |
| StartTime | datetime | When the event began |
| EndTime | datetime | When the event ended (same as Start for instant events) |
| Location | ref→LOCATION | Where it happened |
| Participants | ref→PEOPLE[] | Who was involved/doing the action |
| Witnesses | ref→PEOPLE[] | Who observed (distinct from participants) |
| Status | enum | suspected, confirmed |
| Class | enum | primary_crime, related_incident, background_event |
| Description | string | What happened |
| Objects | ref→OBJECTS[] | Items involved in the event |
| Source | ref→PEOPLE or OBJECTS | How we know about this (witness, document, recording) |

**Design notes:**
- StartTime/EndTime as separate fields handles both instant events and ranges
- Participants vs Witnesses distinction is important:
  - Crime: perpetrator = participant, bystander = witness
  - Alibi: claimant = participant, verifier = witness
- Class enables filtering: Chain of Events draws from primary_crime, background provides motive/context
- Start with precise time ranges; can add fuzzy descriptors later if needed

## Event Type Taxonomy

### 1. Crime Events (Core Incidents)
| Type | Description |
|------|-------------|
| CrimeCommitted | The actual crime - type, location, time, victim(s), object used |
| CrimeAttempted | Failed crimes that still leave evidence |
| CrimeDiscovered | When/how the crime is found (distinct from when it occurred) |
| CrimeStage | For complex crimes - planning, preparation, execution, cover-up phases |
| CrimeStalking | Repeated surveillance/following of target pre-crime (establishes premeditation) |

### 2. Movement & Presence Events (Opportunity & Alibis)
| Type | Description |
|------|-------------|
| ActorEnters | Person enters a location with timestamp |
| ActorLeaves | Person leaves a location with timestamp |
| ActorSeenAt | Witness sighting (potentially unreliable) |
| GroupGathering | Multiple actors together at same location |
| Loitering | Suspicious lingering at a location |
| RoutineDeviation | Someone breaks their normal pattern |

### 3. Interaction Events (Relationships & Motives)
| Type | Description |
|------|-------------|
| DirectConversation | Between actors, location, duration, witnesses |
| PhoneCall | Caller, receiver, duration, cell tower location |
| MessageSent | Digital/SMS content (may be partially recoverable) |
| Argument | Public or private conflict |
| Transaction | Money/items exchanged (legal or illegal) |
| Threat | Explicit or implied menace |

### 4. Object Lifecycle Events (Evidence Chain)
| Type | Description |
|------|-------------|
| ObjectCreated | Birth of evidence (purchase, crafting, finding) |
| ObjectMoved | Transferred between actors/locations |
| ObjectUsed | During a crime or legitimate activity |
| ObjectDestroyed | Attempted elimination of evidence |
| ObjectDiscovered | Found by police or civilians |
| ObjectModified | Altered state (serial numbers filed, cleaned, etc.) |

### 5. Discovery & Reporting Events (Case Triggers)
| Type | Description |
|------|-------------|
| BodyFound | Who found it, condition, location |
| BreakInReported | By victim or security system |
| MissingPersonReported | Time lag creates complexity |
| AnonymousTip | Potentially biased or red herring |
| SuspiciousActivityReported | Bystander observations |

### 6. Actor State Change Events (Motive & Capacity)
| Type | Description |
|------|-------------|
| FinancialChange | Debt, windfall, bankruptcy |
| RelationshipStatusChange | Breakup, divorce, affair start/end |
| EmploymentChange | Fired, hired, promotion/demotion |
| EmotionalState | Stress, rage, euphoria (observed or inferred) |
| Injury | New wounds that may match crime scene |
| ScheduleChange | Shift work, travel plans |

### 7. Temporal & Environmental Events (Context)
| Type | Description |
|------|-------------|
| SecuritySystemOffline | Camera outage, disabled alarm |
| WeatherEvent | Affects evidence preservation, witness visibility |
| PowerOutage | Creates blind spots |
| PublicEvent | Alibi opportunities or crowd cover |

**Design note:** Investigation events are NOT tracked - the investigation is what the player does, not something recorded in the matrix. Events represent what happened before/during the crime and its discovery.

**Gameplay uses:**
- Status tracks investigation progress (suspected → confirmed/contradicted)
- Class helps organize which events are core to solving vs. supporting context
- Witnesses vs Participants enables verification mechanics

---

## Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-01-18 | Session target: 25-35 min | Roguelike feel, low commitment per case |
| 2026-01-18 | 4 entity types: PEOPLE, LOCATIONS, OBJECTS, EVENTS | Core elements needed for deduction |
| 2026-01-18 | RELATIONSHIPS = field on PEOPLE | Cross-refs already needed, no separate type required |
| 2026-01-18 | ORGANIZATIONS = field on PEOPLE | Sufficient for 25-35 min case scope |
| 2026-01-18 | EVIDENCE = gameplay mechanism, not matrix type | Separates "what you know" from "how you learned it" |
| 2026-01-18 | PEOPLE fields defined | 17 fields including physical descriptors, contact info, relationships |
| 2026-01-18 | LOCATIONS fields defined | 7 fields, granularity at building/unit level |
| 2026-01-18 | OBJECTS fields defined | 9 fields, vehicles can be dual-entry (object + location) |
| 2026-01-18 | EVENTS fields defined | 11 fields, Participants vs Witnesses distinction |
| 2026-01-18 | Event Type taxonomy: 7 categories, 34 types | Crime (incl. CrimeStalking), Movement, Interaction, Object Lifecycle, Discovery, State Change, Environmental |
| 2026-01-18 | Investigation events excluded from matrix | Investigation is player action, not tracked data |
| 2026-01-18 | Database tables mapped | actors→PEOPLE, locations→LOCATIONS, events→EVENTS, objects→NEW |

---

## Implementation Notes: Database Tables

**Existing tables (from archive/v1) - need field updates:**
| Table | Maps To | Status |
|-------|---------|--------|
| actors | PEOPLE | Update fields to match new spec |
| locations | LOCATIONS | Update fields to match new spec |
| events | EVENTS | Update for new taxonomy (33 types) |
| relationships | PEOPLE.Relationships field | May keep as junction table |
| districts | Parent of LOCATIONS | Keep as-is |

**New tables needed:**
| Table | Maps To | Notes |
|-------|---------|-------|
| objects | OBJECTS | New table - baseline objects created during Population pass |

**Simulator pass updates:**
1. Foundation → LOCATIONS (exists)
2. Population → PEOPLE + baseline OBJECTS (objects creation is new)
3. Simulation → EVENTS (which may create/move/modify OBJECTS)
