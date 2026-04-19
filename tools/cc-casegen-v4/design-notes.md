# Case Generator — Design Notes

This document captures the locked-in design decisions for the procedural case generator. Organized by layer, from high-level concept down to concrete generation mechanics.

---

## 1. Game Concept

A single-player FBI investigator game where the player explores procedurally generated cases, examines evidence, interviews witnesses, and reconstructs the events of each case. Accuracy of reconstruction determines the score.

**North star:** sits on a spectrum between *Clue* (deductive puzzle) and *FBI Files* (aesthetic/tone). Mechanically much closer to Clue / Obra Dinn / The Case of the Golden Idol. The FBI Files comparison is aesthetic only.

**Per-case target:** 30–45 minutes of play. Cases are fully isolated; no meta-progression. Player skill itself is the progression.

### Core loop
- Player explores scenes and selects evidence to examine (limited action points — must prioritize)
- Player interviews witnesses (testimony-based evidence)
- Player assembles a reconstructed timeline of events
- Player submits final timeline → scored against ground truth

### Scoring model
Wordle-style: each event in the player's timeline is scored on whether it is (a) a real event in the case and (b) placed in the correct timeline position. Partial credit for right-event-wrong-position. Both identity and position matter.

### Design pillars (locked)
- **Failure floor:** always scorable, rarely perfect. No unwinnable cases.
- **Evidence model:** flavor (a) only — irrelevant-but-real red-herring events. No silent deception (no lying witnesses, no planted evidence) in v1. Unreliable-but-flagged sources may be added later.
- **Deterioration:** baked into the generated case as a snapshot. No in-session decay simulation.
- **Archetype scope:** v1 targets **murder cases only.** Other archetypes (disappearance, heist, etc.) would need their own skeletons and are deferred.

---

## 2. Generation Approach: Retrograde + Skeleton

The generator uses a **retrograde + skeleton** approach, not pure forward simulation.

### The approach
- Author a **skeleton** (a typed structural template with slots) once per archetype.
- At generation time, fill the skeleton by working **backwards** from the end state of the crime:
  1. Commit to the end state (victim, killer, motive, etc.)
  2. Construct the causal chain that led to it, backwards in time
  3. Run a light **forward simulation** for ambient red-herring threads around the crime
- Generate evidence from the completed event structure.

### Why this over pure simulation
- **Solvability is free** — the generator constructs the solution, so reaching it is guaranteed achievable.
- **Causal coherence is free** — every crime-relevant event exists because a later event needed it.
- **Matches how mystery authors actually write** — killer first, clues engineered around them.
- **Clean separation:** crime thread is authored retrograde; ambient red-herrings are forward-simulated. They don't interfere.

### Honest tradeoffs
- Structural surprise of pure simulation is lost. A retrograde generator won't produce crimes that defy the archetype.
- The skeleton becomes visible after enough cases (~30–50 plays). Extending the horizon requires multiple archetypes.
- Variety comes from three compounding sources: slot values, retrograde causal chain shape, and the ambient forward-sim wrapping around each case.

### Reference reading
- Ronald Knox's "Ten Commandments of Detective Fiction" (1929)
- S.S. Van Dine's "Twenty Rules" (1928)
- P.D. James — *Talking About Detective Fiction*
- Patricia Highsmith — *Plotting and Writing Suspense Fiction*
- John Dickson Carr's "Locked Room Lecture" (chapter in *The Hollow Man*)

Knox and Van Dine are essentially early attempts at constraint systems for fair-play mysteries — directly applicable.

---

## 3. Layer 1: Entity Types

Six entity types form the world model.

### ID scheme (shared across all entities)
- Typed string IDs: `person_001`, `place_042`, `event_017`, `object_008`, `thread_003`, `relationship_012`
- Type prefix prevents cross-entity confusion when debugging generator output
- Readable when inspecting generated cases
- Clean JSON serialization
- Implementation note: in C-style C++, these can be arena-interned strings with fixed-size handles at runtime.

### Attribute discipline (applies to every entity)
- Target 8–12 attributes per entity
- Every attribute must either **drive a generation decision** or be **visible to the player**; cut otherwise
- Prefer enum-typed fields over free strings when the generator needs to branch
- Maps (e.g., `physical_traits`) are acceptable when the set of relevant properties is domain-dependent

---

### 3.1 Person

```
id                : typed string
name              : string
age               : number
sex               : enum
physical_traits   : map<string, value>        (height, build, hair, eyes,
                                               distinguishing marks — adaptable)
residence         : Place ref
occupation        : string
personality       : (axis_1, axis_2) — see below
```

**Plus** edges to other Persons via `Relationship` entities (not a field on Person).

**Not an attribute:** `type` / `role` (victim/perpetrator/witness). Roles are **relational** — derived from participation in Events, never stored directly on a Person.

#### Personality: 2 axes, 4 values per axis

**Axis 1 — Impulsive ←→ Calculating:** drives the temporal shape of action (how motive crystallizes, commission style, aftermath behavior).

**Axis 2 — Confrontational ←→ Secretive:** drives the evidence profile (whether conflicts play out publicly or privately; human-witness trail vs. documentary/physical trail).

**Values per axis (4):**
- `extreme_calculating` / `mild_calculating` / `mild_impulsive` / `extreme_impulsive`
- `extreme_confrontational` / `mild_confrontational` / `mild_secretive` / `extreme_secretive`

**Rules:**
- Every Person commits to one value on each axis (no neutral middle — direction is always signaled)
- **At most one extreme trait per Person.** Exception: the killer may have up to two extremes for dramatic cases.
- Mild and extreme share the same branch direction, at different amplitudes
- Reliability (as a witness) is subsumed by personality, not a separate attribute

---

### 3.2 Place

```
type              : enum (residence / commercial / public / outdoor / transport / industrial)
name_or_address   : string
owner             : Person ref | null          (null for public places)
regular_occupants : List<(Person, role)>       (resident, employee, member, regular)
access_level      : enum (public / semi-public / private / restricted)
security          : enum (none / low / high)
operating_hours   : TimeWindow | always
connected_to      : List<Place ref>            (spatial adjacency graph)
capacity          : enum (low / medium / high) — optional, drives witness density
```

**Connectivity is load-bearing.** The `connected_to` graph enables **trajectory evidence** — traces of someone's path to/from a scene (lobby camera, elevator record, hallway witness). Without it, an entire evidence class is unavailable.

---

### 3.3 Object

```
id                : typed string
name              : string                    ("kitchen knife", "burner phone", "diary")
type              : enum (weapon / document / electronic / personal /
                          environmental / consumable / misc)
physical_traits   : map<string, value>
owner             : Person ref | null
portability       : enum (fixed / portable)
origin_location   : Place ref
```

Objects are **passive** — acted upon in events, don't act themselves. State changes (bloody, broken, missing) derive from events that touched the object; not stored on the object.

---

### 3.4 Event

```
id                : typed string
type              : enum — see 13 event types below
time              : timestamp (+ optional duration)
place             : Place ref
participants      : List<(Person | Object, role)>
                    — role ∈ {actor, subject, observer, instrument, target}
causal_parents    : List<Event ref>          (events that enabled this one)
visibility        : enum (public / semi-private / private / secret)
thread            : Thread ref
```

**Participant modeling:** uniform list of (entity, role) tuples. Both Persons and Objects participate; role tags give semantics. This handles heterogeneous event shapes cleanly (a meeting has no subject; a transaction has buyer/seller as dual actors; a stabbing has actor+subject+instrument).

**Causality on the event** forms an implicit DAG. Events can have causal parents across threads (important for cross-thread links that thread-level grouping alone would lose).

**Thread direction:** events store their thread reference; Thread membership is derived. Single source of truth.

#### The 13 event types

Grouped by generative role:

**Social / relational (motive-driving):**
1. `meeting` — neutral social contact
2. `argument` — verbal conflict
3. `threat` — explicit threat against a person

**Capability / opportunity:**
4. `movement` — travel between Places (trajectory evidence)
5. `object_acquisition` — gaining control of an object
6. `transaction` — money/goods/favors exchange

**Communication:**
7. `communication_call` — voice
8. `communication_text` — electronic messaging
9. `communication_written` — physical letters/documents

**Physical:**
10. `violence` — physical harm short of death
11. `commission` — the crime itself (anchor event for retrograde)

**Cover-up / state change:**
12. `object_disposal` — hiding, destroying, discarding

**Discovery:**
13. `discovery` — finding something significant

#### Deliberately excluded event types
- `observation` — subsumed by the `observer` participant role on existing events
- `reconciliation` — folds into `argument` with an outcome tag or `meeting` in the relationship arc
- `learning` / `information_acquisition` — derivable from communication/observation
- `object_interaction` (generic use of an object) — too vague; folds into movement/communication

**Rule for future additions:** new event types must produce evidence that isn't already covered by the existing 13. Otherwise it's a *variation*, not a type.

---

### 3.5 Thread

```
id             : typed string
role           : enum (primary / red_herring)
theme          : enum (from ~10 motivational themes — see below)
participants   : List<Person>
active_window  : TimeWindow
intensity      : enum (low / medium / high)
```

Thread is a **convenience grouping entity**, not a fundamental one. It wraps events sharing a thematic driver.

#### Theme palette (~10)
`affair`, `financial_dispute`, `inheritance`, `professional_rivalry`, `blackmail`, `long_running_feud`, `addiction`, `secret`, `family_conflict`, `ideological`

Constrained enum. Themes drive Knox/Van Dine-style plausibility discipline.

**Primary thread** keeps its place in the data model for **uniformity** — every event references a Thread; red-herring pipelines and crime pipelines use the same query/simulation interface.

---

### 3.6 Relationship

```
id                : typed string
person_a          : Person ref
person_b          : Person ref                 (directional: a→b for directional types)
type              : enum (~15 types)
valence           : enum (positive / neutral / negative / complex)
intensity         : enum (low / medium / high)
status            : enum (current / past / dormant)
public_knowledge  : enum (public / private / secret)
started_at        : timestamp | window
```

**Relationships are strictly pairwise** in v1. Group identity (families, companies as entities) is deferred — handled implicitly via pairwise relationships + shared Places.

**Direction semantics are carried by the type enum.** Some types are symmetric (spouse, sibling); some are directional (employer → employee, creditor → debtor). No separate `symmetric` flag needed.

#### Relationship type palette (~15)
`spouse`, `ex_partner`, `romantic_partner`, `sibling`, `parent`, `child`, `friend`, `acquaintance`, `neighbor`, `coworker`, `employer`, `creditor`, `rival`, `blackmailer`, `therapist_patient`

**Valence is separate from intensity.** Intensity says "how much," valence says "which direction." Siblings can have an intense loving bond or an intense hateful one — both facets are needed.

---

## 4. Layer 2: Case Skeleton

### The skeleton

```
── Case metadata ──────────────────────────────────────────────
id                         : typed string
archetype                  : enum                       (v1: always `murder`)
time_window                : TimeWindow                 (may extend backward for
                                                         relationship history)

── Anchor slots (retrograde starting point) ───────────────────
victim                     : Person ref
killer                     : Person ref
motive_stack               : List<theme>                (1–3 themes, ordered by dominance)
opportunity                : (Place ref, TimeWindow)
commission                 : Event ref                  (type = commission; its
                                                         instrument participant IS
                                                         the weapon/tool)

── Crime structure (built retrograde from anchors) ────────────
killer_victim_relationship : Relationship ref           (theme-compatible with
                                                         motive_stack)
relationship_arc           : List<Event ref>            (2–4 events)
precipitating              : List<Event ref>            (3–5 events, causally chained,
                                                         MUST contain an
                                                         object_acquisition matching
                                                         commission.instrument)
aftermath                  : List<Event ref>            (1–3 events)
primary_thread             : Thread ref                 (role = primary)

── Red herrings (forward-sim) ─────────────────────────────────
red_herring_threads        : List<Thread ref>           (2–4 threads, role = red_herring)

── Investigator entry point ───────────────────────────────────
opening_event              : Event ref                  (typically a `discovery` event
                                                         inside aftermath)
```

**Note:** No `means` slot — the weapon/tool lives as the `commission` event's `instrument` participant. The acquisition event in `precipitating` captures how the killer got it.

### Slot-filling order (retrograde)

Principle: **commit to the most constraining slot first.** Each commitment narrows the space for downstream picks.

**Anchor commitments:**
1. `victim`
2. `motive_stack` (the thematic engine — filters the killer pool)
3. `killer`
4. `killer_victim_relationship`
5. `opportunity`
6. `commission` event (instrument picked here)

**Chain construction (independent subproblems after anchors):**
7. `relationship_arc`
8. `precipitating`
9. `aftermath`
10. `opening_event` flag

**Wrapping + parallel:**
11. `primary_thread` (entity instantiation, bookkeeping)
12. `red_herring_threads` (forward-sim)

---

## 5. Layer 2: Constraints

### Cardinality
```
motive_stack          : 1–3 themes
relationship_arc      : 2–4 events
precipitating         : 3–5 events
aftermath             : 1–3 events
red_herring_threads   : 2–4 threads
personality extremes  : ≤ 1 per Person (killer: ≤ 2)
```

### Referential
- `victim`, `killer` are Person refs, `killer ≠ victim`
- `commission.participants` must include victim (role=subject), killer (role=actor), and an Object (role=instrument)
- `opening_event` typically a `discovery` inside aftermath
- All crime events reference `primary_thread`

### Compatibility
1. `killer_victim_relationship.type` compatible with `motive_stack`
2. `motive_stack[0] == primary_thread.theme` (motive_stack is authoritative)
3. Killer and victim both reachable at `opportunity.(Place, TimeWindow)` via Place connectivity
4. `precipitating` must contain an `object_acquisition` whose target Object == `commission.instrument`
5. Killer has plausible access path to `opportunity.Place`

### Temporal
- `relationship_arc` → `precipitating` → `commission` → `aftermath`
- `opening_event` at or after commission
- Thread `active_window` encloses all its member events' times

### Global validation (checked on finished case)
- **Solvability** — evidence from primary_thread sufficient to reconstruct the timeline within accuracy threshold
- **Uniqueness** — primary_thread timeline is the strictly best fit; no red_herring_thread tied
- **Red herring plausibility** — each red_herring_thread produces at least one participant as suspicious as killer on at least one of (motive, opportunity, means)

### Soft generation guidelines
- Killer personality shapes the temporal pattern and visibility profile of the crime chain
- Respect Place connectivity for movement events
- Observer participants drawn from Persons plausibly present at the Place during the TimeWindow
- Red herring threads should share some participants or places with the crime thread
- Thread `active_window` typically extends before `case.time_window`

---

## 6. Layer 3: Evidence Generation

### 4 evidence types

| Type | Source | Properties |
|---|---|---|
| `testimony` | Persons | Reliability-weighted; fade baked into case snapshot |
| `physical_trace` | Places, Objects | Persistent; condition baked into case snapshot |
| `record` | Intentional documents, transactions, logs | Persistent; explicit content; stored physically or digitally |
| `derived_signal` | Absence / pattern deviation | Requires investigator context to surface |

### 7 facets (Wordle-scoring atomic units)

```
who_acted         : primary actor(s), role=actor
who_affected      : subject(s), role=subject
when              : time position
where             : Place
what_type         : which of the 13 event types
what_instrument   : Object with role=instrument, if applicable
topic             : subject matter of interaction (null for non-interactive events)
```

**Motive is NOT a facet.** It's an **emergent inference** the player draws from `topic` facets across the timeline. This matches how real detective narratives work: motive is argued for, not directly revealed.

Every event instance carries a **subset** of applicable facets. Not every event uses every facet.

### Evidence type attributes

**Common to all four types:**
```
id              : typed string
source_event    : Event ref
content_facets  : Set<facet>
```

**testimony:**
```
source_person   : Person ref
reliability     : enum (low/med/high)
delivery        : enum (volunteered / requires_pressure / hostile)
```

**physical_trace:**
```
place           : Place ref
object          : Object ref | null
trace_type      : enum (blood / fingerprint / fiber / tool_mark / damage / residue / DNA / footprint / …)
condition       : enum (fresh / degraded / faint)
```

**record:**
```
record_type     : enum (text_message / call_log / calendar_entry / transaction /
                        letter / email / access_log / surveillance_footage / receipt / …)
storage         : Place ref | digital_system
involved_persons: List<Person ref>
content_summary : string
```

**derived_signal:**
```
signal_type     : enum (missing_item / pattern_deviation / unusual_presence / unusual_absence)
expected        : string
observed        : string
surfacing_place : Place ref
```

### Compositional emission rules (not combinatorial)

Rules **add**, not **multiply**. Four independent layers compose at emission time:

1. **Per-event-type rules** — base emissions per event type
2. **Place modifiers** — e.g., `security=high` adds surveillance; `access_level=restricted` adds access logs
3. **Object modifiers** — e.g., `electronic` object adds digital logs
4. **Actor-personality modifiers** — e.g., `extreme_impulsive` increases scene traces; `extreme_secretive` suppresses observer testimony

### Rule budget (hold firm)
- **≤ 5 emissions per event type** (13 × 5 = 65 max)
- **≤ 3 modifiers per place property**
- **≤ 3 modifiers per object property**
- Role handling is parametric (loop variable), not per-rule
- Total rule ceiling: ~80

### The 4-pass emission process (per event)

1. Apply base emissions (≤5)
2. Apply place modifiers
3. Apply object modifiers
4. Apply actor-personality modifiers

### Post-passes

5. **Downstream event adjustments** — later events can reduce/remove earlier evidence (`object_disposal` → remove `instrument_traces`). This is how cover-ups work mechanically.
6. **Facet coverage check** — union of evidence must cover enough facets to support solvability.

### Worked example: `commission` base emissions

```
The 4-pass emission process
Given an event instance, evidence is generated by:

Base emissions — apply the event-type's core emission rules (≤5). These are the "no matter what" evidence a commission produces.
Place modifiers — layer on what the place adds (security → surveillance; restricted access → badge logs; operating hours → unusual-presence signals).
Object modifiers — layer on what involved Objects add (electronic → digital logs; documentary → content records).
Actor-personality modifiers — adjust what the base emissions produce based on the killer's personality (impulsive → messier scene; calculating → cleaner; secretive → fewer witnesses; confrontational → adjacent-place testimony).
Then two post-passes:

Downstream event adjustments — later events in the chain can reduce/remove earlier evidence (e.g., an object_disposal of the instrument removes instrument_traces).
Facet coverage check — verify the union of evidence covers enough facets to support solvability.
Pass 1 — commission's 5 base emissions
commission emits:
  1. body_at_scene        : physical_trace
     - place   = commission.place
     - object  = null  (the body itself is the trace; victim ref carried on evidence)
     - trace_type = body
     - facets  = { who_affected, where, what_type }
  2. scene_traces         : physical_trace (rule; emits 1–4 trace items)
     - place   = commission.place
     - trace_types can include: blood, footprint, fiber, tool_mark, struggle_sign
     - which subset fires depends on place properties + killer personality
     - facets  = { where, what_type } always;
                 { who_acted } if any trace is individually-identifying
                 { when } if forensically datable
  3. instrument_traces    : physical_trace
     - place   = current location of commission.instrument
     - object  = commission.instrument
     - trace_types: blood, fingerprint, damage
     - facets  = { what_instrument, what_type };
                 { who_acted } if fingerprinted
  4. victim_absence       : derived_signal
     - signal_type = unusual_absence
     - surfacing_place = victim's residence or workplace
     - expected = "victim present in routine patterns"
     - observed = "victim absent from all expected patterns"
     - facets  = { who_affected, when }  (when = approximate time of last activity)
  5. observer_testimony   : testimony  (conditional)
     - fires ONLY if commission has a participant with role=observer
     - source = the observer Person
     - reliability based on observer's personality + relationship to participants
     - facets  = any subset depending on observer reliability;
                 high-reliability observer reveals {who_acted, who_affected,
                 what_type, when, where, what_instrument}
5 emissions. Within budget. Note #5 is conditional and usually doesn't fire — successful murders rarely have observers.

Pass 2 — place modifiers
if commission.place.security == high:
  + surveillance_footage : record
    - storage = digital_system
    - involved_persons = whoever appeared on camera around commission time
    - facets = { who_acted, when, where }
if commission.place.access_level == restricted:
  + access_log_entry : record
    - storage = digital_system
    - facets = { who_acted, when, where }
if commission.place.type == outdoor:
  - scene_traces emission is downgraded (weather degrades traces faster)
Pass 3 — object modifiers (on the instrument)
if instrument.type == electronic:
  + device_activity_log : record
    - facets = { what_instrument, when }
if instrument.physical_traits includes "purchased_recently":
  + purchase_record : record (but this is really from the object_acquisition event,
                              not commission — skip here to avoid double-counting)
Pass 4 — killer personality modifiers
if killer.axis_1 == extreme_impulsive:
  - scene_traces produces MORE items (struggle, disturbance, blood spatter)
  + emotional_residue : derived_signal on killer's subsequent behavior events
if killer.axis_1 == extreme_calculating:
  - scene_traces produces FEWER items (cleaned up)
  - instrument_traces are reduced if calculating killer wiped the weapon
if killer.axis_2 == confrontational:
  + adjacent_place_testimony : testimony (rule)
    - source = occupants of places connected_to commission.place
    - content = overheard sounds
    - facets = { when, where, what_type (partial) }
if killer.axis_2 == secretive:
  - observer_testimony is suppressed (killer chose no-witness opportunity)
Pass 5 — downstream event adjustments
This is where the event chain interacts with emissions. Examples:

If aftermath contains an object_disposal of the instrument → remove instrument_traces, possibly add a derived_signal for missing_item
If aftermath contains a cleanup event → reduce scene_traces
If precipitating includes a notable threat from killer to victim → the threat event's emissions contribute topic evidence that feeds motive inference
The key insight: downstream events can remove evidence from earlier events, which is how cover-ups work mechanically. The generator tracks this as a post-pass over the full event chain.

Pass 6 — facet coverage check
Sum up the facets covered by all evidence from commission (plus its adjusted modifiers). For solvability, commission's evidence should cover at least {who_affected, where, what_type} directly (the "a murder happened to person X at place Y" baseline).

Other facets (who_acted, when, what_instrument, topic) typically come from the surrounding events' evidence — precipitating arguments reveal topic, threats reveal who_acted as a candidate, relationship_arc reveals connection.

Commission by itself doesn't need to reveal the killer. If it did, the case would be trivial. It reveals that the crime happened and who the victim was. The rest of the case's job is revealing the surrounding facets.

What this illustrates for the framework
A few things I want to call out from this walkthrough:

The 5-emission budget holds comfortably. Even for the highest-stakes event, we fit.
Modifiers do a lot of work without adding special cases. extreme_calculating doesn't create new rules — it tunes existing ones.
Events interact through the chain, not through special-case emission rules. Object disposal doesn't need a rule that says "also reduce commission traces" — commission's rules just query the chain.
Facet coverage is the validation interface. The generator checks "does this event reveal enough facets to support solvability" without having to reason about evidence types.
Does this process look right? If so, the remaining 12 event types fill in using the same 4-pass structure — each with its own base emissions — and we can move to place modifiers and object modifiers as standalone passes that apply across all events.
```

**Commission by itself doesn't need to reveal the killer.** If it did, the case would be trivial. It reveals *that the crime happened and who the victim was*. The surrounding events' evidence reveals the rest.

### Anti-patterns to resist
1. **Special-casing** — if tempted, decompose into base rule + modifiers
2. **Overly specific emission types** — prefer abstract types that presentation specializes
3. **Scenario-authored rules** — no separate rules for "murder of spouse" vs "murder of coworker"; the participants differ, the rules don't

---

## 7. Open / Deferred

- **Concrete threshold values** — "target accuracy" for solvability, red herring suspicion thresholds. Tuning parameters, not structural.
- **Remaining 12 event types' emission rules** — fill in using the same 4-pass template once `commission` is fully validated
- **Place and object modifier tables** — standalone passes across all events
- **Difficulty levers** — deferred until a simple generator is running and can be tested empirically
- **Archetype variation** — only murder in v1; other archetypes get their own skeletons later
- **Group entity** — deferred; promote if/when a case archetype genuinely needs collective actors

---

## 8. Guiding Principles (summary)

1. **Every attribute earns its place or is cut** — drives generation or is player-visible
2. **Constrained enums over free text** — anywhere the generator branches
3. **Compositional over combinatorial** — add rules, don't multiply
4. **Commit to constraints coarse-to-fine** — retrograde goes from open to fully specified
5. **Derive what you can, store only what you must** — state emerges from events wherever possible
6. **Every rule must answer "what does this do that existing rules don't?"** — or it's a variation, not a new rule
