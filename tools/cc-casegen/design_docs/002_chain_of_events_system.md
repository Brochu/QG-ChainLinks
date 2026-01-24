# Chain of Events System

## Overview

The Chain of Events is the core gameplay mechanic where the player fills in blanks to reconstruct what happened during a crime. Like Obra Dinn's death identification, but for police procedural cases.

**Reference example from earlier discussion:**
```
1. ________ broke into ________'s apartment on (date)_________ using ____________
2. The perpetrator murdered the victim using ____________
3. Then he/she covered their tracks by ______________
4. Finally he/she fled to (location) ______________
```

Each blank is filled by selecting entries from the Information Matrix (PEOPLE, LOCATIONS, OBJECTS, EVENTS).

---

## Key Questions to Answer

1. **Selection**: How do we select which events from the simulation become part of the chain?
2. **Blank Creation**: Which fields become blanks vs. given information?
3. **Difficulty**: How do we control difficulty (more blanks = harder)?
4. **Validation**: How does the player submit answers? Partial credit?
5. **Variety**: How do we ensure chains feel different across cases?

---

## Chain Extraction Process

**Causation-Based Extraction (Preferred)**

With `ParentEventId` tracking in the simulation (see doc 001), chain extraction becomes straightforward:
1. Start from the crime event (CrimeCommitted)
2. Walk backward through parent links to build causal chain
3. Result is naturally coherent narrative

This approach replaces heuristic-based extraction for events that have causal links populated.

**Fallback: Class-Based Extraction**

For events without causal links, use the original approach:

**Step 1: Gather Chain Events**
- Select all events with `Class: primary_crime`
- Order chronologically
- Apply trimming rules to keep chain manageable:
  - Maximum chain length (TBD - maybe 5-8 events?)
  - Remove redundant events (multiple ActorSeenAt at same location)
  - Prioritize: CrimeCommitted > CrimeStage > entry/exit events

**Step 2: Identify Blankable Elements**
For each chain event, list all matrix references:
- Participants (WHO)
- Location (WHERE)
- Time (WHEN)
- Objects (WHAT/HOW)

**Step 3: Validate Evidence Paths**
For each potential blank, count discoverable evidence sources:
- Witness accounts (ActorSeenAt, event Witnesses field)
- Physical evidence (ObjectDiscovered, forensic Condition)
- Documentation (PhoneCall, MessageSent, Transaction records)
- Cross-references (relationships, known locations, alibis)

**Solvability Constraint:** Only allow blanking if `evidence_count >= 2`
- Ensures player has multiple paths to discover the answer
- Prevents unsolvable cases

**Step 4: Apply Blanking**
- For eligible elements, apply % chance to blank (difficulty scaling)
- Higher difficulty = more blanks, harder evidence paths

---

## Example Chain Construction

**Simulated Events (primary_crime class):**
1. CrimeStalking: John watches Sarah's apartment (3 occasions)
2. CrimeStage(preparation): John purchases crowbar
3. ActorEnters: John enters Sarah's building
4. CrimeCommitted: John murders Sarah with kitchen knife
5. ObjectMoved: John takes Sarah's laptop
6. ActorLeaves: John flees to motel

**Resulting Chain (with blanks):**
1. ________ stalked the victim at ________ over the previous week
2. The perpetrator broke into the building on (date)________
3. ________ murdered ________ using ________
4. The perpetrator stole ________ from the scene
5. The perpetrator fled to ________

**Evidence paths for "WHO" (John):**
- Witness saw tall man loitering near building (physical description match)
- Fingerprints on crowbar (ObjectDiscovered)
- Cell tower records place his phone near scene (PhoneCall metadata)
- Relationship: ex-boyfriend (motive via RelationshipStatusChange event)

---

## Configuration Parameters

| Parameter | Value | Notes |
|-----------|-------|-------|
| Chain Length | 5-8 events | Trim if over, reject case if under |
| Blank Ratio | 50% | Starting point, tune with playtesting |
| Evidence Minimum | 2 per blank | Solvability floor |
| Blank Difficulty Weighting | None (v1) | Keep simple, all blanks treated equally |

These are tuning levers for later difficulty adjustments.

---

## Validation & Scoring

**Submission Model:** Single submission at end of case
- Player can change any blank until they choose to submit
- No incremental locking (unlike Obra Dinn's "3 correct = locked")
- Feedback delivered all at once after submission

**Scoring:** Letter grade based on correct blanks
| Grade | Threshold (approximate) |
|-------|------------------------|
| S | 100% correct |
| A | ~90%+ |
| B | ~80%+ |
| C | ~70%+ |
| D | ~60%+ |
| E | ~50%+ |
| F | Below 50% |

*Exact thresholds TBD based on playtesting*

---

## Open Questions (Remaining)

1. **Trimming Rules**: Specific criteria for removing events when chain > 8?

---

## Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-01-19 | Causation-based chain extraction preferred | Walk ParentEventId links from crime backward for coherent narrative |
| 2026-01-18 | Extract chain from primary_crime events | Post-hoc extraction simpler than tagging during simulation |
| 2026-01-18 | Solvability constraint: evidence_count >= 2 per blank | Ensures every blank has discoverable answer |
| 2026-01-18 | Chain length: 5-8 events | Balance between trivial and overwhelming |
| 2026-01-18 | Blank ratio: 50% | Starting point, will tune with playtesting |
| 2026-01-18 | No blank difficulty weighting (v1) | Simplify to get project off ground |
| 2026-01-18 | Single submission at end | Player can revise until ready, feedback all at once |
| 2026-01-18 | Letter grade scoring (F-S) | Based on % of correct blanks |
