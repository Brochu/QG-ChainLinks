# Case Structure

## Overview

A "case" is the complete package exported from the simulator for the player to solve. This document defines what a case contains and the requirements for a valid case.

---

## Case Components

A complete case export includes:

### 1. Chain of Events
- 5-8 events from primary_crime class
- ~50% of elements blanked
- Each blank has 2+ evidence paths

### 2. Matrix Entries (Initial State)
Starting information mimics what's available at the crime scene:

- **PEOPLE**:
  - Victim: may or may not be identified (depends on context - found at home vs. no ID)
  - Reporter: whoever discovered/reported the crime
  - Others: unlocked through investigation

- **LOCATIONS**:
  - Crime scene (known, can be examined)
  - Other locations unlocked through evidence

- **OBJECTS**:
  - Physical evidence at scene (body state, weapon if visible, damage)
  - Can examine immediately to unlock early info
  - More objects revealed through investigation

- **EVENTS**:
  - CrimeDiscovered event (how/when found)
  - Other events unlocked through evidence/testimony

### 3. Evidence Pool
All discoverable evidence for the case:
- Filtered from simulation (relevant to chain)
- Injected extras (if needed for solvability)
- Mapped to discovery methods

**Evidence Reveals:**
Each evidence item has a `reveals` array describing what information it provides. Each reveal specifies:
- `entity_type`: What kind of entity (person, location, object, event)
- `entity_id`: Which specific entity
- `field`: Which field is revealed (or `null` for existence-only)
- `value`: What the player learns (or `null` for existence-only)

**Existence-only reveals:** When `field` and `value` are both `null`, the evidence reveals that an entity exists and is relevant to the case, without providing specific field data. This adds the entity to the player's "known entities" list. For example, a witness mentioning "he went to some warehouse" might reveal that `loc_003` exists without revealing its name or address.

### 4. Case Metadata
- Crime type (murder, robbery, etc.)
- Difficulty rating (based on blank count, evidence directness)
- Actor count
- Location count
- Estimated solve time (derived from complexity)

---

## Case Requirements (Validation)

A case is **valid** if:

| Requirement | Threshold |
|-------------|-----------|
| Chain length | 5-8 events |
| Blank solvability | Each blank has ≥2 evidence paths |
| Actor count | 6-8 (1 victim, 1 perp, 3-4 suspects, witnesses) |
| Location count | 3+ minimum (configurable) |
| Crime committed | At least one CrimeCommitted event exists |

If validation fails, case is rejected and simulation reruns or adjusts.

---

## Actor Requirements

**Target actors for a valid case:**

| Role | Count | Notes |
|------|-------|-------|
| Victim | 1 | Single victim for v1 |
| Perpetrator | 1 | Single perpetrator for v1 |
| Suspects | 3-4 | Including perpetrator; meaningful choice pool |
| Witnesses | Multiple | Player won't interrogate everyone; need coverage |

**Total target**: ~6-8 actors

More suspects = more meaningful deduction. More witnesses = player chooses who to talk to (resource cost).

**Matrix trimming**: Export only actors relevant to the case, not entire simulation population.

---

## Case Complexity Tiers

| Tier | Chain Length | Blank % | Actors | Evidence Directness |
|------|--------------|---------|--------|---------------------|
| Easy | 5 | 40% | 4-5 | Mostly direct |
| Medium | 6-7 | 50% | 5-7 | Mixed |
| Hard | 8 | 60% | 7+ | More circumstantial |

*Values are starting points for tuning*

---

## Export Format

What does the case file/data structure look like?

```
Case {
  metadata: {
    id, crime_type, difficulty, generated_date
  }

  chain: [
    {
      chain_index, event_id, event_type,
      blanks: [
        { blank_index, event_field, entity_type, entity_field, correct_value, evidence_ids }
      ]
    }
  ]

  matrix: {
    people: [...],
    locations: [...],
    objects: [...],
    events: [...]
  }

  evidence_pool: [
    { type, source_event, reveals, discovery_method }
  ]
}
```

**Note:** Solutions are stored inline with each blank (`correct_value`), not in a separate section. The `blank_index` field disambiguates when multiple blanks share the same `event_field` (e.g., two participants in one event).

---

## Investigation Resource System (Gameplay Note)

*This is a gameplay mechanic, not strictly case structure, but affects how evidence is accessed.*

- No real-time pressure
- Player spends a resource to examine evidence / perform investigation actions
- Resource cost gates what the player can unlock
- Creates strategic choice: what's worth investigating?
- Detailed design TBD in separate gameplay doc

---

## Open Questions

1. ~~**Actor count**~~ → DECIDED: 6-8 total (1 victim, 1 perp, 3-4 suspects, multiple witnesses)
2. ~~**Starting info**~~ → DECIDED: Crime scene state, victim ID contextual, immediate physical evidence
3. ~~**Time pressure**~~ → DECIDED: None; resource-based investigation instead
4. ~~**Export format**~~ → DECIDED: JSON for now, custom binary later

**Remaining:**
- None - all open questions resolved (see sections below)

---

## Matrix Trimming Rules

Matrix trimming ensures only case-relevant entries are exported, avoiding noise from the full simulation.

**Distance-Based Inclusion**

Distance from crime = number of relational hops through the causal/relationship graph.

| Distance | Description | Examples |
|----------|-------------|----------|
| 0 | Direct crime participants | Perpetrator, victim, crime location, murder weapon |
| 1 | One hop away | Witness at scene, perp's associate, victim's family, day-of locations |
| 2 | Two hops | Friend of a witness, perp's workplace, object owned by associate |
| 3+ | Distant | Unrelated NPCs, locations never visited by relevant actors |

**Trimming Rule:** Include distance 0-2, exclude distance 3+.

**Red herrings** naturally fall at distance 1-2: someone with motive but no opportunity, suspicious behavior unrelated to the actual crime.

**Computation Timing:** Distance is calculated at export time, not during simulation. When a crime is selected for export:
1. BFS/flood-fill outward from the crime event
2. Tag each entity with its distance
3. Apply trimming threshold
4. Export included entities

The threshold (default: 2) should be configurable.

---

## Supported Crime Categories (v1)

Limited set for initial implementation:

**Violent Crimes**
- Kidnapping
- Attempted murder / assassination

**Organized Crime / Racketeering**
- Extortion rings
- Drug trafficking network
- Money laundering operations

Categories are kept flat (no sub-fields like `ransom_demanded` or `substance_type`) - that information exists in the simulation but isn't extracted as category-specific metadata.

---

## Case Metadata Fields

| Field | Type | Notes |
|-------|------|-------|
| case_id | string | Unique identifier |
| crime_type | enum | From supported crime categories |
| difficulty | string/null | **Reserved for future** - leave empty for now |
| actor_count | number | Quick reference |
| location_count | number | Quick reference |
| generation_seed | number | For reproducibility |
| version | string | Schema version for compatibility |

Difficulty calculation deferred until we have real data to derive a formula.

---

## Solution Format

Solutions are stored inline with each blank in the chain (not in a separate section).

```json
"chain": [
  {
    "chain_index": 2,
    "event_id": "evt_003",
    "event_type": "ActorEnters",
    "blanks": [
      {
        "blank_index": 0,
        "event_field": "participants",
        "entity_type": "person",
        "entity_field": "name",
        "correct_value": "person_003",
        "evidence_ids": ["evi_006", "evi_007"]
      },
      {
        "blank_index": 1,
        "event_field": "location",
        "entity_type": "location",
        "entity_field": "name",
        "correct_value": "loc_002",
        "evidence_ids": ["evi_003", "evi_009"]
      }
    ]
  }
]
```

**Key fields:**
- `blank_index`: Position within the chain event (0-indexed) - disambiguates multiple blanks with the same event_field
- `event_field`: Which field of the event is blanked (e.g., `participants`, `location`, `objects`)
- `entity_field`: Which field of the entity to display/match (e.g., `name`, `alias`, `address`)
- `correct_value`: The entity ID that is the correct answer
- `evidence_ids`: References to evidence that can reveal this blank (min 2 for solvability)

**Scoring approach:**
- Correct element + correct field = full points
- Wrong or inaccurate value = point deduction (player didn't find best evidence)
- Exact scoring formula TBD with playtesting

---

## Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-01-22 | Added entity_field to blanks, renamed field→event_field | Maps which entity property to display (e.g., person's name) |
| 2026-01-22 | Solutions inline with blanks, added blank_index | Simpler structure, unambiguous blank identification |
| 2026-01-22 | Removed template field from chain | Game handles display text, not generator |
| 2026-01-19 | Matrix trimming: distance ≤2 included | Hop-based distance from crime; computed at export time |
| 2026-01-19 | Location minimum: 3+ | Starting point, made configurable for tuning |
| 2026-01-19 | Crime categories: violent + organized (5 types) | Limited scope for v1, flat structure |
| 2026-01-19 | Difficulty field reserved (empty for now) | Need real data before deriving formula |
| 2026-01-19 | Solution format: index-based blanks | chain_index + field + correct_value per blank |
| 2026-01-18 | Actor target: 6-8 total | 1 victim, 1 perp, 3-4 suspects, multiple witnesses |
| 2026-01-18 | Starting info = crime scene state | Victim ID contextual; immediate evidence examinable |
| 2026-01-18 | No time pressure; resource-based instead | Strategic depth without real-time stress |
| 2026-01-18 | JSON export (v1), custom binary later | Readability now, optimization later |
| 2026-01-18 | Matrix trimming required | Export only case-relevant entries, not full simulation |
