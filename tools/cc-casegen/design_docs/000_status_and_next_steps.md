# Design Status & Next Steps

**Last Updated:** 2026-01-23

---

## Current Progress

### Completed Design Docs

| Doc | Status | Summary |
|-----|--------|---------|
| 001_game_vision_and_info_matrix.md | Complete | 4 entity types, all fields defined, 34 event types, ParentEventId for causation |
| 002_chain_of_events_system.md | Complete | Causation-based extraction, 5-8 events, 50% blank ratio, 2+ evidence minimum |
| 003_evidence_system.md | Complete | Two-phase (natural gen + extraction), no gating, witnesses truthful / suspects lie |
| 004_case_structure.md | Complete | Trimming rules, crime categories, metadata, solution format all defined |

### Schema Files Created

| File | Status | Notes |
|------|--------|-------|
| schemas/case_export_v0.schema.json | **v0.1.0** | Refinements applied (inline solutions, blank_index) |
| schemas/sample_case.json | **Updated** | Example kidnapping case, matches v0.1.0 schema |

### Key Decisions Made
- Matrix entity types: PEOPLE, LOCATIONS, OBJECTS, EVENTS
- Evidence is gameplay mechanism, not matrix type
- Chain extracted via ParentEventId causal links (preferred) or primary_crime class (fallback)
- Causation derived from actor memory during simulation
- Solvability: each blank needs 2+ evidence paths
- No time pressure; resource-based investigation instead
- Red herrings emerge naturally from simulation (distance 1-2 from crime)
- Witnesses truthful, suspects can lie
- Export format: JSON (v1), custom binary later
- Matrix trimming: include entities at distance ≤2 from crime, computed at export time
- Location minimum: 3+ (configurable)
- Crime categories: kidnapping, attempted murder, extortion, drug trafficking, money laundering
- Solution format: index-based (chain_index + field + correct_value)
- Difficulty field reserved but empty until we have data
- Many fields converted to enums for consistency (height, build, hair_color, access_control, etc.)

---

## Session Notes: 2026-01-23 - World Setup Phase

**Completed:**
- Implemented World Setup phase with working code
- Files created: `name_cycle.h/cpp`, `name_gen.h/cpp`, `world_setup.h/cpp`, `sim_db.h/cpp`
- Generates: districts, locations, people (with archetypes), relationships (with strength), objects
- Uses Markov chain (trained on city_names.csv) for district and street names
- Street pool is generated and reused across locations for realistic city feel
- Person aliases (nicknames) - ~30% of people get one
- Relationship strength tracking (0-100, varies by type)
- CLI: `casegen.exe --size 0-3 --seed N --db file.db`

**TODO for future sessions:**
- Consider adding 1-2 more personality archetypes if good ideas emerge
  - Current: VOLATILE, GREEDY, LOYAL, AVERAGE
  - Ideas to explore: PARANOID? MANIPULATIVE? IMPULSIVE?

---

## Next Session: Simulator Implementation

**Goal:** Begin simulator implementation in small incremental steps.

### Schema Changes (COMPLETED 2026-01-22)

1. ~~**Simplify chain structure**~~ ✓ Removed `template` field
2. ~~**Move solutions inline with chain**~~ ✓ `correct_value` now in each blank
3. ~~**Blank indexing**~~ ✓ Added `blank_index` to disambiguate multiple blanks per event
4. ~~**Evidence structure**~~ ✓ Kept as-is

### Simulator Implementation Plan
- Work in small incremental steps
- Allow for review and feedback at each step
- Watch for potential perf issues early

### Deferred topics:
- Gameplay mechanics (resource system, investigation actions)
- Difficulty calculation (needs real data)

---

## Document Index

```
design_docs/
├── 000_status_and_next_steps.md  (this file)
├── 001_game_vision_and_info_matrix.md
├── 002_chain_of_events_system.md
├── 003_evidence_system.md
├── 004_case_structure.md
└── 005_implementation_phases.md   ← START HERE for new sessions

schemas/
├── case_export_v0.schema.json
└── sample_case.json
```

---

## Quick Reference: Core Parameters

| Parameter | Value |
|-----------|-------|
| Session target | 25-35 minutes per case |
| Chain length | 5-8 events |
| Blank ratio | 50% |
| Evidence minimum per blank | 2 |
| Actor count | 6-8 (1 victim, 1 perp, 3-4 suspects, witnesses) |
| Export format | JSON (v1) |
