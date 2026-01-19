# Design Status & Next Steps

**Last Updated:** 2026-01-18

---

## Current Progress

### Completed Design Docs

| Doc | Status | Summary |
|-----|--------|---------|
| 001_game_vision_and_info_matrix.md | Complete | 4 entity types (PEOPLE, LOCATIONS, OBJECTS, EVENTS), all fields defined, 34 event types |
| 002_chain_of_events_system.md | Complete | Extraction process, 5-8 events, 50% blank ratio, 2+ evidence minimum, letter grade scoring |
| 003_evidence_system.md | Complete | Two-phase (natural gen + extraction), no gating, witnesses truthful / suspects lie |
| 004_case_structure.md | Mostly complete | 6-8 actors, crime scene starting state, resource-based investigation, JSON export |

### Key Decisions Made
- Matrix entity types: PEOPLE, LOCATIONS, OBJECTS, EVENTS
- Evidence is gameplay mechanism, not matrix type
- Chain extracted post-hoc from primary_crime events
- Solvability: each blank needs 2+ evidence paths
- No time pressure; resource-based investigation instead
- Red herrings emerge naturally from simulation
- Witnesses truthful, suspects can lie
- Export format: JSON (v1), custom binary later

---

## Next Session: Crime Output File Format (v0)

**Goal:** Define the complete, concrete file format for exported cases before restarting simulation work.

### What needs to be specified:
1. **Full JSON schema** for case export
2. **Matrix entry formats** for each type (PEOPLE, LOCATIONS, OBJECTS, EVENTS)
3. **Chain format** with blank definitions
4. **Evidence pool format** with discovery methods
5. **Metadata fields** (crime type, difficulty, etc.)
6. **Solution format** (answers for validation)

### Open questions to resolve:
- Matrix trimming rules: How to select which entries make it into export?
- Location count: Minimum for valid case?
- Crime types: What crime categories to support? (will emerge from file structure work)

### Deferred topics:
- Simulation design (waiting on file format)
- Gameplay mechanics (resource system, investigation actions)

---

## Document Index

```
design_docs/
├── 000_status_and_next_steps.md  (this file)
├── 001_game_vision_and_info_matrix.md
├── 002_chain_of_events_system.md
├── 003_evidence_system.md
└── 004_case_structure.md
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
