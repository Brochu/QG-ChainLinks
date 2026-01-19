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
| Actor count | TBD minimum (enough for suspects + witnesses) |
| Location count | TBD minimum |
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
    { event_template, blanks: [{field, answer, evidence_paths}] }
  ]

  matrix_initial: {
    people: [...],
    locations: [...],
    objects: [...],
    events: [...]
  }

  evidence_pool: [
    { type, source_event, reveals, discovery_method }
  ]

  solution: {
    // Complete answers for all blanks (for validation)
  }
}
```

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
- Matrix trimming rules: How to select which actors/locations/objects make it into case export?
- Location count: Minimum locations for a valid case?

---

## Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-01-18 | Actor target: 6-8 total | 1 victim, 1 perp, 3-4 suspects, multiple witnesses |
| 2026-01-18 | Starting info = crime scene state | Victim ID contextual; immediate evidence examinable |
| 2026-01-18 | No time pressure; resource-based instead | Strategic depth without real-time stress |
| 2026-01-18 | JSON export (v1), custom binary later | Readability now, optimization later |
| 2026-01-18 | Matrix trimming required | Export only case-relevant entries, not full simulation |
