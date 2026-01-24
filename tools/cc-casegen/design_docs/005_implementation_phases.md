# Implementation Phases Overview

This document provides a quick reference for each implementation phase of the case generator. Each section is designed to give enough context to start a fresh Claude Code session.

---

## Pipeline Overview

```
┌─────────────────┐
│  World Setup    │ ← Generate city, people, relationships, objects
└────────┬────────┘
         ▼
┌─────────────────┐
│ Event Simulation│ ← Run until crimes occur
└────────┬────────┘
         ▼
┌─────────────────┐
│ Case Selection  │ ← Pick a crime to turn into a case
└────────┬────────┘
         ▼
┌─────────────────┐
│Chain Extraction │ ← Build 5-8 event chain via ParentEventId
└────────┬────────┘
         ▼
┌─────────────────┐
│Evidence Mapping │ ← Map evidence to chain blanks
└────────┬────────┘
         ▼
┌─────────────────┐     ┌──────────────────┐
│Solvability Check│────►│ Reject / Retry?  │──► Back to Case Selection
└────────┬────────┘ NO  └──────────────────┘    or inject evidence
         │ YES
         ▼
┌─────────────────┐
│ Red Herrings &  │ ← Distance 1-2 actors/evidence that mislead
│ Case Enrichment │
└────────┬────────┘
         ▼
┌─────────────────┐
│ Matrix Trimming │ ← Include only distance ≤2 from crime
└────────┬────────┘
         ▼
┌─────────────────┐
│  Case Export    │ ← Output JSON for game consumption
└─────────────────┘
```

---

## Phase 1: World Setup ✓ COMPLETED

**Goal:** Generate a believable city subset with people, locations, objects, and relationships.

**What it does:**
- Creates districts (residential, commercial, industrial, nightlife, docks, financial)
- Generates locations within districts (residences, businesses, public spaces)
- Creates people with demographics, personality traits (0-100), needs (0-100), and archetypes
- Builds relationship graph (family, work, social, economic ties)
- Assigns initial objects (phones, vehicles)

**Key files:**
- `world_setup.h` - Data structures and enums
- `world_setup.cpp` - All generation logic
- `name_cycle.h/cpp` - Prime-step cycling through name CSVs (first names only)
- `name_gen.h/cpp` - Markov chain for district/street names
- `sim_db.h/cpp` - SQLite table creation and insert helpers

**Configuration:**
- City sizes: SMALL (50), MEDIUM (80), LARGE (120), METRO (150 actors)
- Personality archetypes: VOLATILE, GREEDY, LOYAL, AVERAGE
- Relationships include strength (0-100)
- ~30% of people get aliases (nicknames)

**CLI:** `casegen.exe --size 0-3 --seed N --db file.db`

---

## Phase 2: Event Simulation

**Goal:** Run a time-stepped simulation where actors make decisions and events occur, eventually leading to crimes.

**Key concepts:**
- Actors have needs (money, belonging, status, security) that decay over time
- Low needs + personality traits = motivation for action
- Events have `ParentEventId` tracking causation (critical for chain extraction)
- Simulation runs until a crime occurs + enough post-crime time for investigation opportunities

**Event types to implement (from 001_game_vision_and_info_matrix.md):**
- Movement: ActorEnters, ActorLeaves, ActorSeenAt
- Communication: PhoneCall, MessageSent, Meeting
- Social: Argument, Threat, RelationshipStatusChange
- Financial: Transaction, FinancialChange, Employment
- Crime prep: CrimeStalking, CrimeStage (planning, preparation, execution)
- Crime: CrimeCommitted
- Objects: ObjectAcquired, ObjectMoved, ObjectDestroyed, ObjectModified

**Actor decision loop:**
1. Evaluate current needs
2. Consider available actions based on personality
3. Select action (weighted by urgency and traits)
4. Execute action → generate event(s)
5. Update world state

**Evidence generation (natural):**
Events automatically generate evidence as byproduct:
- ActorSeenAt → witness sightings
- PhoneCall → phone records, cell tower pings
- Transaction → receipts, bank records
- CrimeCommitted → physical evidence at scene

**Output:** Events stored in SQLite `events` table with full causation tracking.

---

## Phase 3: Case Selection

**Goal:** Choose which crime from the simulation becomes the case for export.

**Selection criteria:**
- Must have a CrimeCommitted event
- Prefer crimes with richer causal chains (more ParentEventId links)
- Prefer crimes involving actors with interesting relationships (rivals, ex-partners, debtors)
- Avoid crimes that are too simple (impulse crimes with no buildup)

**Multiple crimes possible:**
- Simulation may produce multiple crimes
- Select the "best" one based on criteria above
- Or allow configuration to target specific crime types

**Output:** Selected crime event ID to feed into chain extraction.

---

## Phase 4: Chain Extraction

**Goal:** Extract a coherent 5-8 event chain from the selected crime.

**Method (causation-based):**
1. Start from the CrimeCommitted event
2. Walk backward through `ParentEventId` links
3. Build causal chain of events leading to crime
4. Include key post-crime events (fleeing, evidence disposal)
5. Trim to 5-8 events if longer

**Fallback method (if causation incomplete):**
1. Select all events with `class: primary_crime`
2. Order chronologically
3. Apply trimming rules (remove redundant events, prioritize key moments)

**Trimming priorities:**
- Keep: CrimeCommitted, CrimeStage events, key entry/exit
- Remove: Redundant ActorSeenAt at same location, minor movements

**Output:** Ordered list of 5-8 chain events.

---

## Phase 5: Blank Creation & Evidence Mapping

**Goal:** Decide which fields become blanks and map evidence to each blank.

**Blank creation:**
- Target ~50% of fields blanked
- Eligible fields: participants (WHO), location (WHERE), time (WHEN), objects (WHAT/HOW)
- Mark each potential blank with available evidence sources

**Evidence mapping:**
- For each blank, list all evidence that can reveal it
- Evidence types: physical, testimonial, documentary, digital, forensic
- Track discovery method for each evidence piece

**Output:** Chain events with blank specifications and evidence mappings.

---

## Phase 6: Solvability Validation

**Goal:** Ensure every blank has at least 2 evidence paths.

**Validation rules:**
- Each blank must have ≥2 independent evidence sources
- Evidence sources must be discoverable through different methods
- No blank should require "lucky guessing"

**If validation fails:**
1. **Option A:** Reject case, return to Phase 3 (Case Selection) with different crime
2. **Option B:** Inject additional evidence (add witness, document, physical evidence)
3. **Option C:** Reduce blank count (give more information for free)

**Injection rules:**
- Injected evidence must be plausible within case context
- Prefer witnesses over documents (more natural)
- Track injected vs. natural evidence for debugging

**Output:** Validated chain with confirmed solvability, or rejection signal.

---

## Phase 7: Red Herrings & Case Enrichment

**Goal:** Add misleading but fair elements that make deduction interesting.

**Red herring sources (natural from simulation):**
- Distance 1-2 actors with motive but no opportunity
- Suspicious behavior unrelated to the actual crime
- Circumstantial evidence pointing to wrong suspect
- Partial observations that could implicate innocents

**What makes a good red herring:**
- Someone with clear motive (ex-partner, rival, debtor)
- Evidence that initially points to them (seen near scene, owns similar weapon)
- But alibis or other evidence ultimately exonerates them

**Enrichment opportunities:**
- Ensure 3-4 plausible suspects beyond the actual perpetrator
- Include at least one strong red herring with motive + partial evidence
- Balance: not so many that it's overwhelming

**Output:** Enriched case with suspect pool and misleading elements identified.

---

## Phase 8: Matrix Trimming

**Goal:** Include only case-relevant entities in the export.

**Distance-based inclusion:**
| Distance | Description | Examples |
|----------|-------------|----------|
| 0 | Direct crime participants | Perpetrator, victim, crime location, weapon |
| 1 | One hop away | Witness at scene, perp's associate, victim's family |
| 2 | Two hops | Friend of witness, perp's workplace, object owned by associate |
| 3+ | Excluded | Unrelated NPCs, never-visited locations |

**Trimming rule:** Include distance 0-2, exclude 3+.

**Computation:**
1. BFS/flood-fill outward from crime event
2. Tag each entity with distance
3. Apply threshold (default: 2)
4. Mark included entities for export

**Output:** Set of entity IDs to include in final export.

---

## Phase 9: Case Export

**Goal:** Output a complete, valid case to JSON.

**Case validation (final check):**
| Requirement | Threshold |
|-------------|-----------|
| Chain length | 5-8 events |
| Blank solvability | Each blank has ≥2 evidence paths |
| Actor count | 6-8 (1 victim, 1 perp, 3-4 suspects, witnesses) |
| Location count | 3+ minimum |
| Crime committed | At least one CrimeCommitted event exists |

**Export structure:**
```json
{
  "metadata": {
    "case_id": "...",
    "crime_type": "kidnapping",
    "difficulty": null,
    "version": "0.1.0",
    "generation_seed": 12345
  },
  "chain": [
    {
      "chain_index": 0,
      "event_id": "evt_001",
      "event_type": "CrimeStage",
      "blanks": [
        {
          "blank_index": 0,
          "event_field": "participants",
          "entity_type": "person",
          "entity_field": "name",
          "correct_value": "person_003",
          "evidence_ids": ["evi_006", "evi_007"]
        }
      ]
    }
  ],
  "matrix": {
    "people": [...],
    "locations": [...],
    "objects": [...],
    "events": [...]
  },
  "evidence_pool": [
    {
      "id": "evi_001",
      "type": "testimonial",
      "source_event": "evt_005",
      "reveals": [...],
      "discovery_method": "interview_witness"
    }
  ]
}
```

**Supported crime categories (v1):**
- Violent: Kidnapping, Attempted murder
- Organized: Extortion, Drug trafficking, Money laundering

**Output:** JSON file ready for game consumption.

---

## Quick Reference

| Phase | Input | Output | Loop Back? |
|-------|-------|--------|------------|
| 1. World Setup | Config (size, seed) | SQLite DB with world | No |
| 2. Event Simulation | World state | Events with causation | No |
| 3. Case Selection | Events table | Selected crime ID | Yes (from Phase 6) |
| 4. Chain Extraction | Crime ID | 5-8 event chain | No |
| 5. Blank & Evidence | Chain | Blanks + evidence map | No |
| 6. Solvability | Blanks + evidence | Validated or reject | Yes → Phase 3 |
| 7. Red Herrings | Validated case | Enriched case | No |
| 8. Matrix Trimming | Full world + case | Trimmed entity set | No |
| 9. Case Export | All above | JSON file | No |

---

## Core Parameters

| Parameter | Value |
|-----------|-------|
| Session target | 25-35 minutes per case |
| Chain length | 5-8 events |
| Blank ratio | 50% |
| Evidence minimum per blank | 2 |
| Actor count in export | 6-8 |
| Suspect pool | 3-4 (including perpetrator) |
| Red herrings | 1+ strong, natural from distance 1-2 |
| Export format | JSON (v1) |
