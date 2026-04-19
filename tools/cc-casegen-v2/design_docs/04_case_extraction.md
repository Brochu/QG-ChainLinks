# 04 — Case Extraction

## Overview

After the 30-day simulation completes, we have an SQLite database full of ~21,000 events forming a causal DAG. Case extraction is the process of:

1. Finding all crime events (terminal nodes of interest)
2. Walking the causal chain backwards to reconstruct the full story
3. Walking forward to capture the aftermath
4. Scoring each potential case for "interestingness"
5. Selecting the best 5-6 candidates
6. Defining the case window (time bounds the player investigates)

---

## Step 1: Find Terminal Crimes

```sql
SELECT event_id, event_type, sim_time, actor_id, target_id, landmark_id
FROM sim_events
WHERE event_type IN (13, 14)  -- EVT_MURDER, EVT_THEFT
ORDER BY sim_time ASC;
```

Each result is a potential case root. A simulation with ~70 actors over 30 days should yield 5-10 crime events.

---

## Step 2: Walk Causality Backwards

For each crime event, reconstruct the full causal chain using a recursive CTE:

```sql
-- Get the complete causal ancestry of a crime
WITH RECURSIVE causal_chain AS (
    -- Start at the crime event
    SELECT event_id, tick, sim_time, event_type, actor_id, target_id,
           landmark_id, room_id, object_id, detail_int, detail_text,
           cause_event_id, 0 AS depth
    FROM sim_events
    WHERE event_id = :crime_event_id

    UNION ALL

    -- Walk backwards through cause links
    SELECT e.event_id, e.tick, e.sim_time, e.event_type, e.actor_id, e.target_id,
           e.landmark_id, e.room_id, e.object_id, e.detail_int, e.detail_text,
           e.cause_event_id, c.depth + 1
    FROM sim_events e
    JOIN causal_chain c ON e.event_id = c.cause_event_id
    WHERE c.depth < 20  -- safety limit to prevent infinite recursion
)
SELECT * FROM causal_chain ORDER BY sim_time ASC;
```

This gives us every event that contributed to the crime, ordered chronologically. The `depth` column tells us how many causal steps away from the crime each event is.

**Example result for a murder:**

| depth | event_type | sim_time | actor | target | landmark |
|-------|-----------|----------|-------|--------|----------|
| 5 | EVT_DEBT_CREATED | Day 2, 14:00 | Marco | Sergei | Bank |
| 4 | EVT_ARGUMENT | Day 5, 21:30 | Sergei | Marco | Bar |
| 3 | EVT_ARGUMENT | Day 9, 22:00 | Sergei | Marco | Bar |
| 2 | EVT_THREAT | Day 12, 20:15 | Sergei | Marco | Parking Lot |
| 1 | EVT_ACQUIRE_WEAPON | Day 13, 23:30 | Sergei | — | Sergei's Home |
| 0 | EVT_MURDER | Day 14, 01:15 | Sergei | Marco | Warehouse |

---

## Step 3: Walk Forward for Aftermath

```sql
-- Get all aftermath events caused by the crime
WITH RECURSIVE aftermath AS (
    SELECT event_id, tick, sim_time, event_type, actor_id, target_id,
           landmark_id, room_id, cause_event_id, 0 AS depth
    FROM sim_events
    WHERE cause_event_id = :crime_event_id

    UNION ALL

    SELECT e.event_id, e.tick, e.sim_time, e.event_type, e.actor_id, e.target_id,
           e.landmark_id, e.room_id, e.cause_event_id, a.depth + 1
    FROM sim_events e
    JOIN aftermath a ON e.cause_event_id = a.event_id
    WHERE a.depth < 10  -- aftermath chains are typically shorter
)
SELECT * FROM aftermath ORDER BY sim_time ASC;
```

**Example aftermath for the same murder:**

| depth | event_type | sim_time | actor | target | landmark |
|-------|-----------|----------|-------|--------|----------|
| 0 | EVT_FLEE | Day 14, 01:20 | Sergei | — | Warehouse |
| 0 | EVT_HIDE_OBJECT | Day 14, 01:45 | Sergei | — | Dumpster (Alley) |
| 0 | EVT_WITNESS | Day 14, 01:15 | Linda | — | Warehouse |
| 1 | EVT_BODY_DISCOVERED | Day 14, 07:00 | Tony | Marco | Warehouse |
| 2 | EVT_CALL_POLICE | Day 14, 07:05 | Tony | — | Warehouse |

---

## Step 4: Interest Scoring

Not every crime makes a good case. We score each candidate:

```cpp
struct case_candidate {
    i64 crime_event_id;
    sim_event_type crime_type;
    i64 perpetrator_id;
    i64 victim_id;
    i32 interest_score;

    // Metrics used for scoring
    i32 chain_length;           // number of events in causal chain
    i32 unique_actors;          // distinct actor_ids across chain + aftermath
    i32 unique_landmarks;       // distinct landmark_ids
    i32 num_witnesses;          // EVT_WITNESS events in aftermath
    i32 num_hidden_evidence;    // EVT_HIDE_OBJECT + EVT_DISPOSE_EVIDENCE count
    i32 time_span_hours;        // hours between earliest chain event and crime
    i32 num_red_herrings;       // actors present near crime who aren't the perpetrator
    bool trivially_solved;      // 3+ witnesses directly saw the crime
};
```

### Scoring Formula

```cpp
i32 score_case(case_candidate *c) {
    i32 score = 0;

    // Deeper chains = more for the player to discover
    score += c->chain_length * 10;

    // More people = more interviews, more complexity
    score += c->unique_actors * 15;

    // More locations = more places to explore
    score += c->unique_landmarks * 8;

    // Witnesses provide interview content
    score += c->num_witnesses * 12;

    // Hidden evidence rewards thorough searching
    score += c->num_hidden_evidence * 20;

    // Longer time spans = harder alibi checking
    score += c->time_span_hours * 2;

    // Red herrings create confusion and false leads
    score += c->num_red_herrings * 10;

    // Penalty: if too many people directly witnessed the crime
    if (c->trivially_solved) score -= 50;

    // Bonus: murder is more dramatic than theft
    if (c->crime_type == EVT_MURDER) score += 20;

    // Bonus: if crime involved break-in (adds location exploration)
    // (check if EVT_BREAK_IN is in the chain)

    return score;
}
```

### Red Herring Detection

A **red herring actor** is someone who:
1. Was at the crime landmark within 2 hours of the crime
2. Has a negative relationship with the victim (motive)
3. Is NOT the perpetrator

```sql
-- Find red herring actors for a crime
SELECT DISTINCT e.actor_id
FROM sim_events e
JOIN relationships r ON r.from_actor = e.actor_id AND r.to_actor = :victim_id
WHERE e.landmark_id = :crime_landmark_id
  AND e.sim_time BETWEEN (:crime_time - 120) AND (:crime_time + 60)
  AND e.actor_id != :perpetrator_id
  AND r.strength < -20;
```

These actors are gold for the game: the player might suspect them, investigate them, and eventually rule them out — creating satisfying deductive gameplay.

### Trivially Solved Detection

```sql
SELECT COUNT(DISTINCT actor_id)
FROM sim_events
WHERE cause_event_id = :crime_event_id
  AND event_type = 21  -- EVT_WITNESS
  AND detail_text = 'clear';  -- high-accuracy witness
```

If 3 or more people clearly witnessed the crime, it's trivially solved and gets a score penalty.

---

## Step 5: Select Best Candidates

```cpp
// Sort all candidates by interest_score descending
// Take top 5-6
// If the best score is below MINIMUM_INTEREST_THRESHOLD, re-run simulation

#define MINIMUM_INTEREST_THRESHOLD 80
#define MAX_CASES_TO_SELECT 6
#define MIN_CASES_TO_SELECT 3
```

If the simulation produces fewer than `MIN_CASES_TO_SELECT` candidates above the threshold, we either:
1. Lower thresholds by 10% and re-run
2. Or re-seed with `seed + 1` and run a fresh simulation

In practice, with 70 actors over 30 days and default tuning, we almost always get enough candidates.

---

## Step 6: Define Case Window

The **case window** is the time period the player investigates. It must contain all events the player needs to solve the case, but not so much time that it's overwhelming.

```cpp
struct case_window {
    sim_time start;     // earliest point of investigation
    sim_time end;       // latest point (usually when police are called)
    sim_time crime_time; // when the crime occurred
};
```

**Calculation:**

```
earliest_chain_event = MIN(sim_time) from causal chain (excluding very old root causes)
crime_time = sim_time of the crime event
police_time = sim_time of EVT_CALL_POLICE (or crime_time + 24 hours if no one called)

// Start 6 hours before the immediate pre-crime events (not the deep root cause)
// Use depth <= 3 events as "immediate" chain
immediate_start = MIN(sim_time) WHERE depth <= 3

case_window.start = immediate_start - 360  // 6 hours before
case_window.end = police_time
case_window.crime_time = crime_time
```

**Why cut off deep root causes?** A debt from Day 2 that leads to a murder on Day 14 is part of the motive but not part of the case window. The player learns about the debt through interviews and documents, not by investigating Day 2 directly. The case window focuses on the "action" portion — the last few days where the confrontations, weapon acquisition, and crime happen.

**Relevant events within the window:**

All events at landmarks visited by any actor involved in the case (perpetrator, victim, witnesses, red herrings) during the case window. This includes:
- The crime chain events
- Routine events of involved actors (provides alibi data)
- Events by uninvolved actors at the same landmarks (noise/red herrings)
- All aftermath events

---

## Step 7: Collect Case Actors

Not all 70 actors appear in the case. We collect only relevant ones:

```sql
-- All actors who appear in case-relevant events
SELECT DISTINCT actor_id FROM (
    -- Actors in the causal chain
    SELECT actor_id FROM causal_chain
    UNION
    SELECT target_id AS actor_id FROM causal_chain WHERE target_id IS NOT NULL
    UNION
    -- Actors in the aftermath
    SELECT actor_id FROM aftermath
    UNION
    -- Actors at case landmarks during case window (alibi witnesses, red herrings)
    SELECT DISTINCT e.actor_id
    FROM sim_events e
    WHERE e.landmark_id IN (SELECT DISTINCT landmark_id FROM causal_chain)
      AND e.sim_time BETWEEN :window_start AND :window_end
      AND e.event_type IN (0, 1)  -- EVT_DEPART, EVT_ARRIVE
);
```

Typically this yields 6-12 actors per case — the perpetrator, victim, witnesses, red herrings, and a few people who were nearby for unrelated reasons.

---

## Step 8: Collect Case Landmarks

Similarly, only landmarks relevant to the case:

```sql
SELECT DISTINCT landmark_id FROM (
    -- Landmarks in the causal chain
    SELECT landmark_id FROM causal_chain WHERE landmark_id IS NOT NULL
    UNION
    -- Landmarks in the aftermath
    SELECT landmark_id FROM aftermath WHERE landmark_id IS NOT NULL
    UNION
    -- Home/work of case actors (can be investigated)
    SELECT home_landmark_id AS landmark_id FROM actors
    WHERE id IN (:case_actor_ids)
    UNION
    SELECT work_landmark_id AS landmark_id FROM actors
    WHERE id IN (:case_actor_ids)
);
```

Typically 4-8 landmarks per case.

---

## Data Flow Summary

```
sim_events table (21,000 rows)
    │
    ├─ Find crime events (5-10 rows)
    │
    ├─ For each crime:
    │   ├─ Walk backward → causal chain (3-8 events)
    │   ├─ Walk forward → aftermath (3-10 events)
    │   ├─ Find red herrings (0-3 actors)
    │   └─ Compute interest score
    │
    ├─ Sort by score, take top 5-6
    │
    └─ For each selected case:
        ├─ Define case window (8-48 hours)
        ├─ Collect case actors (6-12)
        ├─ Collect case landmarks (4-8)
        └─ Pass to Evidence Generation (doc 05)
```
