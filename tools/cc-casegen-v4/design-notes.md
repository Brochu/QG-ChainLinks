# Case Generator — Design Notes

This is the authoritative design document for the procedural case generator. Organized by layer, from high-level concept down to concrete generation mechanics.

Appendices at the end capture decisions already argued through, conceptual anchors worth re-stating, implementation context, and working style — consolidated from prior handoff notes.

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

A richer scoring pass that rewards *reasoning process* (confidence locking, revision distance) is flagged as a future iteration. Out of scope until a playable case exists.

### Design pillars (locked)
- **Failure floor:** always scorable, rarely perfect. No unwinnable cases.
- **Evidence model:** flavor (a) only — irrelevant-but-real red-herring events. No silent deception (no lying witnesses, no planted evidence) in v1. Unreliable-but-flagged sources may be added later.
- **Deterioration:** baked into the generated case as a snapshot. No in-session decay simulation.
- **Archetype scope:** v1 targets **murder cases only.** Other archetypes (disappearance, heist, etc.) would need their own skeletons and are deferred.
- **Narrative tension is a first-class concern.** A case is not "valid" solely because it is solvable — it must also sustain a competing-theory structure through to late game. See Layer 4 (§7).

---

## 2. Generation Approach: Retrograde + Skeleton

The generator uses a **retrograde + skeleton** approach, applied to **two parallel chains** per case, with a forward-sim layer for ambient world texture.

### The approach
- Author a **skeleton** (a typed structural template with slots) once per archetype.
- At generation time, fill the skeleton by working **backwards** from the end state of the crime:
  1. Commit to the end state (victim, killer, motive, etc.)
  2. Construct the causal chain that led to it, backwards in time
  3. Build a **parallel retrograde chain** for a promoted red-herring suspect, with an engineered break point
  4. Run a light **forward simulation** for ambient noise threads around the crime
- Generate evidence from the completed event structure.

### Why retrograde for both the truth and the primary red herring
- **Solvability is free** — the generator constructs the solution, so reaching it is guaranteed achievable.
- **Causal coherence is free** — every crime-relevant event exists because a later event needed it.
- **Matches how mystery authors actually write** — killer first, clues engineered around them.
- **Structural completeness of the red herring is free** — by constructing the red herring's chain as if they had committed the crime, they automatically acquire motive, opportunity, timing, and evidence. Forward-sim cannot reliably produce this.
- **Clean separation of concerns:** the truth chain and red-herring chain use the same retrograde machinery; the forward sim produces ambient texture that never rises to suspect-grade.

### Honest tradeoffs
- Structural surprise of pure simulation is lost. A retrograde generator won't produce crimes that defy the archetype.
- The skeleton becomes visible after enough cases (~30–50 plays). Extending the horizon requires multiple archetypes.
- Variety comes from three compounding sources: slot values, retrograde causal chain shape (for both truth and red herring), and the ambient forward-sim wrapping around each case.

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
role           : enum (primary / red_herring / ambient)
theme          : enum (from ~10 motivational themes — see below)
participants   : List<Person>
active_window  : TimeWindow
intensity      : enum (low / medium / high)
```

Thread is a **convenience grouping entity**, not a fundamental one. It wraps events sharing a thematic driver.

**Three thread roles:**
- `primary` — the truth chain (exactly one per case)
- `red_herring` — a promoted suspect's retrograde chain (1 strong per case; optional 1 weak background suspect)
- `ambient` — forward-sim world texture; never suspect-grade

#### Theme palette (~10)
`affair`, `financial_dispute`, `inheritance`, `professional_rivalry`, `blackmail`, `long_running_feud`, `addiction`, `secret`, `family_conflict`, `ideological`

Constrained enum. Themes drive Knox/Van Dine-style plausibility discipline.

**Primary thread** keeps its place in the data model for **uniformity** — every event references a Thread; all thread types use the same query/simulation interface.

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

## 4. Layer 2: Skeletons

A case contains **two retrograde skeletons** (truth + primary red herring) and zero or more **ambient threads**. Both retrograde skeletons use the same structural template, with the red herring's anchor deliberately engineered to fail on one axis.

### 4.1 Truth skeleton (one per case)

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
opportunity                : (Place ref, TimeWindow)    (TimeWindow is the ToD range —
                                                         never a single timestamp)
commission                 : Event ref                  (type = commission; its
                                                         instrument participant IS
                                                         the weapon/tool)

── Crime structure (built retrograde from anchors) ────────────
killer_victim_relationship : Relationship ref           (theme-compatible with
                                                         motive_stack)
relationship_arc           : List<Event ref>            (2–4 events)
precipitating              : List<Event ref>            (3–7 events, causally chained,
                                                         MUST contain an
                                                         object_acquisition matching
                                                         commission.instrument)
aftermath                  : List<Event ref>            (1–3 events)
primary_thread             : Thread ref                 (role = primary)

── Investigator entry point ───────────────────────────────────
opening_event              : Event ref                  (typically a `discovery` event
                                                         inside aftermath)
```

**Total truth chain target:** ~10–15 events. Shorter chains lack the surface area for red-herring overlap; longer chains pad without adding tension.

**Note:** No `means` slot — the weapon/tool lives as the `commission` event's `instrument` participant. The acquisition event in `precipitating` captures how the killer got it.

### 4.2 Red-herring skeleton (one promoted suspect per case)

Structurally a mirror of the truth skeleton, with a **fake anchor** and an **engineered weak link**.

```
── Suspect anchor ─────────────────────────────────────────────
suspect                    : Person ref                 (!= killer, != victim)
fake_motive_stack          : List<theme>                (different species from truth's
                                                         motive_stack; e.g., acute vs.
                                                         structural)
fake_opportunity           : (Place ref, TimeWindow)    (overlaps truth's opportunity
                                                         ToD range)
fake_commission_shape      : Event stub                 (not a real event — the shape
                                                         the suspect's chain is built
                                                         as if leading toward)

── Red-herring structure (built retrograde) ───────────────────
suspect_victim_relationship : Relationship ref          (theme-compatible with
                                                         fake_motive_stack)
relationship_arc           : List<Event ref>            (2–4 events)
precipitating              : List<Event ref>            (3–5 events, chain coherent
                                                         up to the engineered break)
break_point                : (axis, mechanism)          (see below)
alternate_truth            : List<Event ref>            (1–3 events — what the suspect
                                                         actually did during the
                                                         commission window)

red_herring_thread         : Thread ref                 (role = red_herring)
```

**Red-herring chain target:** ~6–10 events. Shorter than the truth chain because the chain stops before committing the (fake) crime.

**Break point:**
- `axis` ∈ {`timing`, `opportunity`, `witness`} — the axis on which the suspect fails to be the killer. **Prefer `timing`** (objective, resolvable, produces clean "aha" moments). Avoid `motive` or `evidence` as the break axis — they feel subjective/arbitrary.
- `mechanism` — the concrete cause of the break, expressed as one or more events in `alternate_truth` (e.g., a `meeting` that serves as alibi; a `movement` placing the suspect elsewhere). The mechanism is **non-binary** — it introduces doubt, not a hard switch. See §5 constraints.

**Alternate truth events** are real things the suspect actually did during the commission window. They emit real evidence. The engineered quality is in *which* events are chosen so that the break surfaces through specific evidence the player must pursue.

### 4.3 Ambient threads (forward-sim, optional)

```
ambient_threads            : List<Thread ref>           (0–3 threads, role = ambient)
```

Ambient threads produce forward-simulated world texture. Constraints:

- Must share ≥1 Place or Person with the truth chain (so they feel connected to the world, not pasted in)
- Must NOT produce evidence of the form "this person had motive + opportunity + timing" — if a participant in an ambient thread reaches suspect-grade structural completeness, either promote the thread to `red_herring` or drop events until it no longer does.
- Ambient threads are where the **evidence-modifier noise** layer lives (see §6.9) — real unrelated events that complicate interpretation of evidence from the primary/red-herring chains.

### 4.4 Slot-filling order (retrograde)

Principle: **commit to the most constraining slot first.** Each commitment narrows the space for downstream picks.

**Phase 1 — Truth anchor commitments:**
1. `victim`
2. `motive_stack` (the thematic engine — filters the killer pool)
3. `killer`
4. `killer_victim_relationship`
5. `opportunity` (with ToD *range*, not point)
6. `commission` event (instrument picked here)

**Phase 2 — Truth chain construction:**
7. `relationship_arc`
8. `precipitating`
9. `aftermath`
10. `opening_event` flag

**Phase 3 — Red-herring anchor + chain:**
11. `suspect` (constrained: must be able to share evidence surface with truth chain; not a close accomplice of killer)
12. `fake_motive_stack` (different species from truth's)
13. `break_point` (axis + mechanism)
14. `fake_commission_shape` → drives retrograde fill for suspect's chain
15. Red-herring `relationship_arc` and `precipitating`
16. `alternate_truth` events (the mechanism of the break)

**Phase 4 — Wrapping + ambient:**
17. `primary_thread`, `red_herring_thread` (entity instantiation)
18. `ambient_threads` (forward-sim)

---

## 5. Layer 2: Constraints

### 5.1 Cardinality
```
motive_stack          : 1–3 themes
relationship_arc      : 2–4 events (each chain)
precipitating         : truth 3–7, red herring 3–5
aftermath             : 1–3 events
alternate_truth       : 1–3 events (red herring only)
ambient_threads       : 0–3 threads
promoted red herring  : exactly 1 per case
weak background susp. : 0–1 per case (optional; 2–3 of 4 axes only, never promoted)
personality extremes  : ≤ 1 per Person (killer: ≤ 2)
```

### 5.2 Referential
- `victim`, `killer`, `suspect` are Person refs; all three distinct
- `commission.participants` must include victim (role=subject), killer (role=actor), and an Object (role=instrument)
- `opening_event` typically a `discovery` inside aftermath
- All truth events reference `primary_thread`; all red-herring events reference `red_herring_thread`

### 5.3 Compatibility
1. `killer_victim_relationship.type` compatible with `motive_stack`
2. `motive_stack[0] == primary_thread.theme` (motive_stack is authoritative)
3. Killer and victim both reachable at `opportunity.(Place, TimeWindow)` via Place connectivity
4. `precipitating` must contain an `object_acquisition` whose target Object == `commission.instrument`
5. Killer has plausible access path to `opportunity.Place`
6. Red herring has plausible access path to their `fake_opportunity.Place` during the ToD range

### 5.4 Temporal
- Truth chain: `relationship_arc` → `precipitating` → `commission` → `aftermath`
- Red-herring chain: `relationship_arc` → `precipitating` → (`alternate_truth` during commission window) → (no aftermath — they didn't commit the crime)
- `opening_event` at or after commission
- Thread `active_window` encloses all its member events' times
- ToD `TimeWindow` must plausibly cover both the truth killer's commission time AND the red herring's presence in the opportunity place

### 5.5 Narrative tension constraints (cross-chain)

These enforce the dual-chain design — a case that satisfies §5.1–5.4 is *structurally valid* but may still be a dud. These constraints enforce *tension*.

1. **Constraint-axis split.** Across the 5 axes {motive, opportunity, physical_evidence, timing, witness_sighting}, neither suspect may pass all 5. At least one axis must point at the red herring over the killer. The intended default distribution:

   | Axis | Killer | Red herring |
   |---|---|---|
   | Motive | ✅ | ✅ |
   | Opportunity | ✅ | ✅ |
   | Physical evidence | ✅ | ❌ (or partial) |
   | Timing | ✅ | ❌ (the break) |
   | Witness sighting | ❌ | ✅ |

   The exact distribution can vary, but the rule holds: **both suspects must pass on ≥3 axes, and each must fail on ≥1 axis the other passes.**

2. **3.5/4 alignment for the promoted red herring.** The red herring passes {motive, opportunity, evidence-presence, timing} on 3 of 4 axes, fails exactly 1 (preferably `timing`). Weak background suspects, if present, pass 2–3 of 4 only and are never structurally complete.

3. **Motive species differentiation.** `motive_stack` (truth) and `fake_motive_stack` (red herring) must be different *species* of motive. Acute vs. structural is the canonical split (about-to-be-fired vs. long-standing grievance). Same-species motives produce fact-matching rather than qualitative judgment.

4. **Shared surface.** Truth chain and red-herring chain must share ≥3 of: Places, Persons (non-suspect), or overlapping time windows. Without shared surface, the chains read as parallel stories, not competing interpretations.

5. **Non-binary break.** The `break_point` mechanism cannot be a single piece of evidence that, once found, clears the red herring absolutely. Examples of what counts as non-binary:
   - Alibi with a ~30 min uncertainty window that partially overlaps the ToD range
   - Alibi witness is not neutral (has reason to cover for the suspect, but isn't lying)
   - Alibi reachable only via cross-referencing two independent sources
   
   Hard-binary breaks ("surveillance clearly shows suspect elsewhere at exact ToD") are forbidden — they collapse tension into a hard switch.

6. **Redundant discovery paths to the break.** The break point must be reachable via ≥2 independent evidence paths (e.g., lawyer testimony AND calendar record AND neighbor sighting). Missing one path must not make the case unfair.

7. **Symmetric contradiction.** The truth chain must have **at least one soft inconsistency** surfaced by evidence — a real, in-world fact that creates doubt about the killer without exonerating them. The canonical example is an evidence-modifier noise event (see §6.9): another person with legitimate reason to touch the weapon, producing a print that doesn't match the killer. The killer's inconsistency is *softer* than the red herring's (the killer is still the killer), but it must exist — otherwise the player has no reason to keep the red herring alive in their mind.

### 5.6 Global validation (checked on finished case)
- **Solvability** — evidence from `primary_thread` sufficient to reconstruct the timeline within accuracy threshold
- **Uniqueness** — primary chain is strictly the best fit; red-herring chain does not tie
- **Tension curve check** — apply the axis-split rule (§5.5.1); confirm the red herring passes on ≥3 axes; confirm non-binary break and redundant paths
- **Red herring plausibility** — the promoted suspect reads as a viable killer using only evidence discoverable in the first half of the investigation (before the break surfaces)

### 5.7 Soft generation guidelines
- Killer personality shapes the temporal pattern and visibility profile of the crime chain
- Red-herring personality should be *different* from killer's on at least one axis — homogeneous personalities blur the evidence profiles
- Respect Place connectivity for movement events
- Observer participants drawn from Persons plausibly present at the Place during the TimeWindow
- Thread `active_window` typically extends before `case.time_window`

---

## 6. Layer 3: Evidence Generation

### 6.1 Four evidence types

| Type | Source | Properties |
|---|---|---|
| `testimony` | Persons | Reliability-weighted; fade baked into case snapshot |
| `physical_trace` | Places, Objects | Persistent; condition baked into case snapshot |
| `record` | Intentional documents, transactions, logs | Persistent; explicit content; stored physically or digitally |
| `derived_signal` | Absence / pattern deviation | Requires investigator context to surface |

### 6.2 Seven facets (Wordle-scoring atomic units)

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

### 6.3 Evidence type attributes

**Common to all four types:**
```
id              : typed string
source_event    : Event ref
content_facets  : Set<facet>
tier            : enum (early / mid / late)      — see §6.8 staging
```

**testimony:**
```
source_person   : Person ref
reliability     : enum (low / med / high)
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

### 6.4 Compositional emission rules (not combinatorial)

Rules **add**, not **multiply**. Four independent layers compose at emission time:

1. **Per-event-type rules** — base emissions per event type (§6.7)
2. **Place modifiers** — e.g., `security=high` adds surveillance; `access_level=restricted` adds access logs (§6.8 Pass 2)
3. **Object modifiers** — e.g., `electronic` object adds digital logs (§6.8 Pass 3)
4. **Actor-personality modifiers** — e.g., `extreme_impulsive` increases scene traces; `extreme_secretive` suppresses observer testimony (§6.8 Pass 4)

### 6.5 Rule budget (hold firm)
- **≤ 5 base emissions per event type** (13 × 5 = 65 max)
- **≤ 3 modifiers per place property**
- **≤ 3 modifiers per object property**
- Role handling is parametric (loop variable), not per-rule
- Total rule ceiling: ~80

### 6.6 The emission process (per event)

**Pass 1 — base emissions** (§6.7)
**Pass 2 — place modifiers** (§6.8)
**Pass 3 — object modifiers** (§6.8)
**Pass 4 — actor-personality modifiers** (§6.8)

**Post-passes (whole-chain):**

**Pass 5 — downstream event adjustments.** Later events can reduce/remove earlier evidence (`object_disposal` → remove `instrument_traces`). This is how cover-ups work mechanically.

**Pass 6 — facet coverage check.** Union of evidence must cover enough facets to support solvability.

**Pass 7 — evidence staging** (§6.9). Tag each emitted evidence item with a tier (`early` / `mid` / `late`) so the surfacing order produces the commit→doubt→switch→resolve arc.

**Pass 8 — bounded-perspective testimony trim** (§6.9). For each testimony, apply position/attention-based facet omission to produce structural incompleteness.

**Pass 9 — `who_acted` omission quota** (§6.9). Ensure 30–50% of final emissions omit `who_acted` to force cross-referencing.

### 6.7 Base emissions — all 13 event types

#### 6.7.1 commission (worked example)

```
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
```

5 emissions. Within budget. Note #5 is conditional and usually doesn't fire — successful murders rarely have observers.

**Commission by itself doesn't need to reveal the killer.** If it did, the case would be trivial. It reveals *that the crime happened and who the victim was*. The surrounding events' evidence reveals the rest.

#### 6.7.2 meeting

```
1. participant_testimony  : testimony  (rule; one per participant, role=actor)
   - each attendee recalls the meeting from their side
   - facets: { who_acted, when, where, what_type, topic }

2. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer)
   - bystanders who saw the meeting occur
   - facets: { who_acted, when, where, what_type }

3. place_presence_trace   : physical_trace
   - incidental traces from being at the Place (fibers, prints on surfaces)
   - facets: { who_acted, where, when }

4. meeting_record         : record  (CONDITIONAL — fires if meeting is scheduled)
   - calendar entry, reservation, or appointment note
   - facets: { who_acted, when, where, what_type }
```

#### 6.7.3 argument

```
1. participant_testimony  : testimony  (rule; one per participant, role=actor)
   - each participant recalls the exchange from their side
   - facets: { who_acted, who_affected, when, where, what_type, topic }

2. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer)
   - bystanders who heard or saw the conflict
   - facets: { who_acted, who_affected, when, where, what_type, topic }

3. place_presence_trace   : physical_trace
   - both parties at the Place
   - facets: { who_acted, who_affected, where, when }
```

Note: an argument without observers and without willing participant testimony reduces to place-presence only — intentional, this is how private conflict stays murky.

#### 6.7.4 threat

```
1. target_testimony       : testimony  (from role=subject)
   - the threatened party recalls being threatened
   - facets: { who_acted, who_affected, when, where, what_type, topic }

2. actor_testimony        : testimony  (from role=actor)
   - the threatener's own recollection; load-bearing because first-person threat
     testimony is distinct from observer report
   - facets: { who_acted, who_affected, when, where, what_type, topic }

3. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer)
   - facets: { who_acted, who_affected, when, where, what_type, topic }

4. place_presence_trace   : physical_trace
   - facets: { who_acted, who_affected, where, when }
```

#### 6.7.5 movement

```
1. trajectory_trace       : physical_trace  (rule; 1–3 items along path)
   - footprints, fibers, incidental marks along the Place(s) traversed
   - facets: { who_acted, where, when }

2. origin_observer        : testimony  (CONDITIONAL — per observer at origin)
   - someone who saw them leave
   - facets: { who_acted, when, where, what_type }

3. destination_observer   : testimony  (CONDITIONAL — per observer at destination)
   - someone who saw them arrive
   - facets: { who_acted, when, where, what_type }

4. transit_observer       : testimony  (CONDITIONAL — per observer on connecting Places)
   - witnesses along the `connected_to` path
   - facets: { who_acted, when, where, what_type }
```

Note: `movement` carries no `topic` or `what_instrument`. It is the backbone of trajectory evidence — surveillance/log records come in via place modifiers (Pass 2).

#### 6.7.6 object_acquisition

```
1. actor_possession_trace : physical_trace  (on the acquired Object)
   - fingerprints, handling wear, transfer residue
   - facets: { what_instrument };
             { who_acted } if individually-identifying (prints, DNA);
             { what_type } if the handling pattern is diagnostic of acquisition

2. acquirer_testimony     : testimony  (from role=actor)
   - facets: { who_acted, when, where, what_type, what_instrument }

3. source_testimony       : testimony  (CONDITIONAL — from prior owner/giver as role=subject)
   - the person the object came from, acted upon by the acquirer
   - facets: { who_acted, when, where, what_type, what_instrument }

4. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer)
   - facets: { who_acted, when, where, what_type };
             { what_instrument } if visible to observer
```

Note: `what_type` on the physical trace is conditionalized because most handling traces don't by themselves tell you the event was specifically an acquisition — that reading usually requires pairing with the testimony or a transaction record.

#### 6.7.7 transaction

```
1. transaction_record     : record
   - receipt, ledger line, transfer record, digital payment log
   - facets: { who_acted, who_affected, when, where, what_type, topic }

2. participant_testimony  : testimony  (rule; one per participant, role=actor)
   - each side of the exchange
   - facets: { who_acted, who_affected, when, where, what_type, topic }

3. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer)
   - facets: { who_acted, who_affected, when, where, what_type }

4. place_presence_trace   : physical_trace
   - facets: { who_acted, who_affected, where, when }
```

#### 6.7.8 communication_call

```
1. call_log_record        : record  (record_type = call_log)
   - telco/device log showing endpoints and duration
   - facets: { who_acted, who_affected, when, what_type }

2. participant_testimony  : testimony  (rule; one per participant — sender role=actor, recipient role=subject)
   - each party recalls the call
   - facets: { who_acted, who_affected, when, what_type, topic }

3. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer on either end)
   - someone in the room during the call
   - facets: { who_acted, who_affected, when, where, what_type, topic }
     (`where` here is the observer's own endpoint location — they can't see the other end)
```

Note: `where` is generally absent from the record itself — location evidence for calls arrives via place modifiers (e.g., cell tower logs) in Pass 2.

#### 6.7.9 communication_text

```
1. message_record         : record  (record_type = text_message / email)
   - persisted content on device or server
   - facets: { who_acted, who_affected, when, what_type, topic }

2. participant_testimony  : testimony  (rule; one per participant — sender role=actor, recipient role=subject)
   - facets: { who_acted, who_affected, when, what_type, topic }

3. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer who saw the exchange)
   - someone who read over a shoulder or was shown the message
   - facets: { who_acted, who_affected, when, what_type, topic }
```

Note: the device used is not modeled as `role=instrument` on a text event (texts aren't "stabbed with" a phone), so no `what_instrument` facet is emitted here. Device-handling traces, when they matter, come in via object modifiers (Pass 3) on the phone as an electronic object.

#### 6.7.10 communication_written

```
1. document_artifact      : physical_trace  (the letter/note as an object)
   - handwriting, paper, ink, prints on the physical document
   - facets: { who_acted, who_affected, what_type, topic }
     (no `when` — paper doesn't timestamp itself; the `document_record` below carries the timing)

2. document_record        : record  (record_type = letter)
   - content of the written communication
   - facets: { who_acted, who_affected, when, what_type, topic }

3. participant_testimony  : testimony  (rule; one per participant — sender role=actor, recipient role=subject)
   - facets: { who_acted, who_affected, when, what_type, topic }
```

#### 6.7.11 violence

```
1. victim_testimony       : testimony  (role=subject; survives by definition)
   - facets: { who_acted, who_affected, when, where, what_type, what_instrument }

2. injury_trace           : physical_trace  (on the victim)
   - bruises, cuts, medical-exam findings
   - facets: { who_affected, what_type, when };
             { what_instrument } if wound-patterning is diagnostic

3. scene_traces           : physical_trace  (rule; 1–3 items at the Place)
   - facets: { where, what_type };
             { who_acted } if individually-identifying;
             { when } if forensically datable

4. instrument_traces      : physical_trace  (CONDITIONAL — if an instrument is used)
   - facets: { what_instrument, what_type };
             { who_acted } if fingerprinted

5. observer_testimony     : testimony  (CONDITIONAL — per non-participant observer)
   - facets: { who_acted, who_affected, when, where, what_type };
             { what_instrument } if visible to observer
```

Note: toned-down `commission` — same shape minus `body_at_scene` and `victim_absence`, plus the living `victim_testimony`.

#### 6.7.12 object_disposal

```
1. missing_item_signal    : derived_signal  (signal_type = missing_item)
   - the disposed object's expected location is empty
   - facets: { what_instrument, where }

2. disposal_trajectory_trace : physical_trace  (rule; 1–2 items along disposal path)
   - incidental traces of the disposer en route
   - facets: { who_acted, where, when }

3. disposal_site_trace    : physical_trace  (CONDITIONAL — at disposal endpoint)
   - disturbed ground, ash, water-logged fragment
   - facets: { where, what_type };
             { what_instrument } if identifiable remains of the object survive

4. observer_testimony     : testimony  (CONDITIONAL — per observer along path or at endpoint)
   - facets: { who_acted, when, where, what_type };
             { what_instrument } if visible to observer
```

Notes:
- Disposal surfaces as `derived_signal` (absence) first; direct traces of the vanished object itself are not emitted here — those belonged to the earlier event that touched the object.
- `pattern_deviation_signal` (e.g., "took the trash on an odd day") is not a base emission. It's scenario-flavored. A `calculating` killer may plan disposal to minimize such deviations; a `confrontational` or `impulsive` one may produce them. That tuning belongs to the personality pass (Pass 4), not base emissions.

#### 6.7.13 discovery

```
1. discoverer_testimony   : testimony  (role=actor on the discovery event)
   - how, when, where the discoverer came upon the thing
   - facets: { who_acted, when, where, what_type }

2. discovery_report_record : record  (CONDITIONAL — if reported to authority/institution)
   - 911 call log, incident report, intake entry
   - facets: { who_acted, when, where, what_type }

3. co_discoverer_testimony : testimony  (CONDITIONAL — per additional observer present)
   - facets: { who_acted, when, where, what_type }
```

Note: `discovery` is the mechanic that **surfaces** other events' evidence — it does not re-emit the thing found. The body, the missing object, the letter, etc., are emissions of their own source events; discovery adds only the find itself (who, when, where).

### 6.8 Modifier passes (2, 3, 4)

Modifiers are **standalone cross-cutting tables** that apply to any event whose Place, participating Object, or actor matches the condition. They do not repeat per event type.

Modifiers either **add** an emission or **tune** a base emission (more/fewer/suppressed/upgraded/downgraded). They reference only the 4 evidence types and the 7 facets. They never name specific scenarios.

Budget: ≤3 modifiers per place property, ≤3 per object property, and a reasonable cap on personality modifiers.

#### 6.8.1 Pass 2 — Place modifiers

Place properties referenced: `type`, `access_level`, `security`, `operating_hours`, `connected_to`, `capacity`.

**type:**
```
if place.type == outdoor:
  - scene_traces / place_presence_trace / trajectory_trace DOWNGRADED
    (weather degrades; condition trends toward faint/degraded)
  - disposal_site_trace UPGRADED
    (open terrain: more spatial disturbance to find)

if place.type == transport:
  + transit_manifest_record : record  (record_type = access_log)
    - storage = digital_system
    - involved_persons = people logged on that vehicle/route at event time
    - facets = { who_acted, when, where }
```

Note: `residence`, `commercial`, `public`, `industrial` intentionally have no type-level modifier — their evidentiary character is already captured by `access_level`, `security`, and `operating_hours`.

**access_level:**
```
if place.access_level == restricted:
  + access_log_entry : record  (record_type = access_log)
    - storage = digital_system
    - involved_persons = whoever badged / keyed / logged in around event time
    - facets = { who_acted, when, where }

if place.access_level in { restricted, private } AND actor is not a regular_occupant:
  + unusual_presence_signal : derived_signal
    - signal_type = unusual_presence
    - expected = "only regular_occupants present"
    - observed = "non-occupant at place during event"
    - facets = { who_acted, when, where }

if place.access_level == public:
  - observer_testimony population EXPANDED
    (wider pool of plausible bystanders; more conditional observer emissions fire)
    Composition note: this rule and the capacity=high EXPANDED rule below are
    additive and independent — both can fire on the same event. Treat the pool
    expansion as a union, not a product.
```

**security:**
```
if place.security == high:
  + surveillance_footage : record  (record_type = surveillance_footage)
    - storage = digital_system
    - involved_persons = whoever appeared on camera around event time
    - facets = { who_acted, when, where };
              { who_affected } if subject also on camera;
              { what_instrument } if instrument visible on camera

if place.security == none:
  - surveillance_footage / access_log_entry SUPPRESSED
```

**operating_hours:**
```
if event.time falls OUTSIDE place.operating_hours:
  + off_hours_signal : derived_signal
    - signal_type = unusual_presence
    - surfacing_place = place
    - expected = "place unoccupied outside operating_hours"
    - observed = "activity / access during closed window"
    - facets = { when, where };
              { who_acted } if paired with any access_log_entry / surveillance_footage

if place.operating_hours == always:
  - off_hours_signal SUPPRESSED
```

**connected_to:**
```
if place.connected_to is non-empty AND event has no direct observer:
  + adjacent_place_testimony : testimony  (CONDITIONAL — per occupant of a connected Place)
    - source = occupants of places in connected_to during event.time
    - content = overheard sounds, glimpses through shared boundary
    - facets = { when, where };
              { what_type } partial (raised voices, struggle, etc.);
              { who_acted } only if connection permits visual identification
```

**capacity:**
```
if place.capacity == high:
  - observer_testimony population EXPANDED

if place.capacity == low:
  - observer_testimony population CONTRACTED
```

#### 6.8.2 Pass 3 — Object modifiers

Object properties referenced: `type`, `physical_traits`, `portability`, `owner`, `origin_location`. Modifiers apply when the Object participates in the event.

**type:**
```
if object.type == electronic:
  + device_activity_log : record  (record_type = access_log)
    - storage = digital_system (on the device or its service)
    - facets = { what_instrument, when };
              { who_acted } if the device is individually-accounted;
              { who_affected } if the logged activity names a counterparty

if object.type == document AND event.type != communication_written:
  - UPGRADE the trace already emitted for that event (actor_possession_trace /
    place_presence_trace) to also carry the `topic` facet.
    - facets added = { topic };
                     { who_affected } if signed/addressed to a party not named in base

if object.type == consumable:
  - actor_possession_trace / instrument_traces DOWNGRADED toward faint/degraded
```

**physical_traits:**
```
if object.physical_traits includes "individually_identifying"
   (serial number, engraving, unique pattern):
  - actor_possession_trace / instrument_traces UPGRADED
    (facets now include { what_instrument } reliably and { who_acted }
     when paired with a possession record)

if object.physical_traits includes "leaves_distinctive_mark"
   (bladed, inked, pigmented, patterned surface):
  - instrument_traces UPGRADED
    (trace_type ∈ { tool_mark, damage, residue };
     facets add { what_instrument, what_type };
     { who_affected } added if the mark is on the subject's body)

if object.physical_traits includes "fragile"
   (and object participates as instrument or target):
  + object_damage_trace : physical_trace
    - place = event.place
    - trace_type = damage
    - facets = { what_instrument, where, what_type }
```

**portability:**
```
if object.portability == portable AND event.place != object.origin_location:
  + displacement_signal : derived_signal
    - signal_type = unusual_presence
    - surfacing_place = event.place
    - expected = "object at origin_location"
    - observed = "object at event.place"
    - facets = { what_instrument, where };
              { when } if the move is forensically datable

if object.portability == fixed:
  - any emission that depends on the object being moved (displacement_signal,
    missing_item_signal on object_disposal) SUPPRESSED
```

**owner:**
```
if object.owner is not null AND actor != object.owner:
  + ownership_mismatch_signal : derived_signal
    - signal_type = unusual_presence
    - surfacing_place = object.origin_location (or current custodial place)
    - expected = "owner has custody of object"
    - observed = "non-owner in possession / using object"
    - facets = { who_acted, what_instrument };
              { when } if paired with a datable trace or record

if object.owner is not null:
  + owner_testimony : testimony  (CONDITIONAL — from the owner, not as event participant)
    - content = recognition of the object, last-seen context, custody pattern
    - facets = { what_instrument };
              { who_acted } if owner can name the last person with access;
              { when } if owner can bracket when it went missing/changed

if object.owner == null:
  - owner_testimony / ownership_mismatch_signal SUPPRESSED
```

**origin_location:**
```
if object.origin_location has security == high OR access_level == restricted:
  + origin_access_record : record  (record_type = access_log)
    - storage = digital_system at origin_location
    - facets = { who_acted, when, where = origin_location, what_instrument }

if event.place != object.origin_location AND object.portability == portable:
  - trajectory evidence for the object's movement from origin to event.place is
    DEFERRED to the movement / object_acquisition event that actually carried it
    (not re-emitted here — avoids double-counting)
```

#### 6.8.3 Pass 4 — Actor-personality modifiers

Personality is `(axis_1, axis_2)` per §3.1. Mild and extreme share direction at different amplitudes — mild rules tune, extreme rules tune harder.

Pass 4 **owns** `delivery` on testimony (`volunteered` / `requires_pressure` / `hostile`). No other pass sets it.

**axis_1 — impulsive ↔ calculating:** drives the temporal/physical shape of action.

```
if actor.axis_1 == extreme_impulsive:
  - scene_traces / place_presence_trace / instrument_traces produce MORE items
    (struggle signs, disturbance, spatter, additional trace_types fire)
  + emotional_residue_signal : derived_signal  (on actor's subsequent events)
    - signal_type = pattern_deviation
    - expected = "actor continues routine"
    - observed = "routine disrupted — missed shifts, erratic movement, outbursts"
    - facets = { who_acted, when }

if actor.axis_1 == mild_impulsive:
  - scene_traces produce slightly more items (same direction, smaller amplitude)

if actor.axis_1 == mild_calculating:
  - scene_traces / instrument_traces produce FEWER items (partial cleanup)

if actor.axis_1 == extreme_calculating:
  - scene_traces / instrument_traces produce MUCH FEWER items
    (thorough cleanup; condition trends toward faint)
  - off_hours_signal, unusual_presence_signal, displacement_signal all
    DOWNGRADED when produced by this actor (deliberate avoidance)
```

Note: alibi construction is cover-up logic and emerges from chain events (a calendar_entry, a transaction elsewhere emit their own records via Pass 1), not a new Pass 4 rule.

**axis_2 — confrontational ↔ secretive:** drives the evidence profile.

```
if actor.axis_2 == extreme_confrontational:
  + adjacent_place_testimony : testimony  (rule — fires even if connected_to is empty,
                                           drawing from any observer in earshot)
    - content = overheard raised voices, shouting, public display
    - facets = { when, where };
              { what_type } partial;
              { who_acted } if voice-identifiable
  - observer_testimony facets EXPANDED
    (loud/public conduct gives any observer more to report)
  - any testimony sourced from this actor: delivery = volunteered

if actor.axis_2 == mild_confrontational:
  - any testimony sourced from this actor: delivery = volunteered

if actor.axis_2 == mild_secretive:
  - observer_testimony population CONTRACTED (actor chose quieter contexts)
  - any testimony sourced from this actor: delivery = requires_pressure

if actor.axis_2 == extreme_secretive:
  - observer_testimony SUPPRESSED where conditional
  - adjacent_place_testimony SUPPRESSED
  - any testimony sourced from this actor: delivery = hostile
```

**Cross-axis worst case (generator note, not an emission rule):** a killer configured as `extreme_calculating` AND `extreme_secretive` is the thin-evidence case. No new rule fires here — this is purely composition. When this profile occurs, Pass 6 (facet coverage) will need to lean on downstream events to hit the solvability floor. If Pass 6 fails, add an event to `precipitating` or `aftermath` — don't add a modifier.

**Observer personality (symmetric rule):**
```
if observer.axis_2 == extreme_confrontational:  delivery = volunteered
if observer.axis_2 == mild_confrontational:     delivery = volunteered
if observer.axis_2 == mild_secretive:           delivery = requires_pressure
if observer.axis_2 == extreme_secretive:
  - delivery = hostile
  - observer_testimony content facets CONTRACTED
```

Note: axis_1 is not mirrored onto observers. An observer's impulsiveness doesn't change the evidence their observation produces — it changes whether they act on it, which is a separate event running through its own pipeline.

### 6.9 Passes 7, 8, 9 — Narrative tension passes

These are new passes applied after Passes 1–6. They do not add evidence; they **tag, trim, and audit** existing emissions so the discovery experience produces genuine theory tension. See Layer 4 (§7) for the principles that motivate them.

#### Pass 7 — Evidence staging

Every emitted evidence item is assigned a `tier` ∈ {`early`, `mid`, `late`}.

Staging rules:
- **Early tier** — evidence from the `opening_event` and its immediate surrounding scene; any evidence tied to Places the player reaches in the first exploration round. Budget: ~40% of total evidence.
- **Mid tier** — evidence requiring an initial interview or a non-opening Place visit. Budget: ~40%.
- **Late tier** — evidence gated behind specific pursued leads or cross-references. Includes: the red herring's **break point** mechanism, at least one independent corroboration of the killer's weapon chain, and any evidence that recontextualizes earlier findings. Budget: ~20%.

**Key constraint:** the red herring's break (§4.2) must be `late` tier; the truth chain's uniqueness-confirming evidence must be `late` or `mid` tier. This enforces the commit→doubt→switch→resolve arc structurally.

#### Pass 8 — Bounded-perspective testimony trim

Testimony emissions are intentionally *incomplete*, not merely unreliable. For each testimony emission:

- Identify the **position/attention constraints** on the source Person at the event (line-of-sight, distance, duration of presence, attention elsewhere)
- Drop a subset of the base emission's facets consistent with what the source could not plausibly have perceived

Examples:
- A witness who saw only the entry to an alley loses `what_type`, `what_instrument`, `who_affected`
- A witness who heard voices but couldn't see loses `who_acted` (unless voice-identifiable)
- A witness whose attention was divided loses `when` precision (gains uncertainty range)

Rule: **every testimony emission drops ≥1 facet** from the base-emission set unless the source is a direct first-person participant with full attention. The exception is participant_testimony from a willing, confrontational, first-person participant — those stay maximally complete.

This is what produces Her Story's "locally true, globally incomplete" effect without lying witnesses.

#### Pass 9 — `who_acted` omission quota

Across all finalized evidence for a case, 30–50% of items must have `who_acted` absent from their facets. If fewer, selectively drop `who_acted` from:

- Physical traces that are not individually-identifying (downgrade fingerprint to smudge, DNA to partial)
- Testimony where the source's position/attention made identity inference marginal
- Records that log activity without naming a person (access log entries without badged ID)

If more than 50%, the case is over-obscured — remove some lower-priority evidence items or upgrade a trace to identifying.

This forces cross-referencing: the player must combine two pieces of evidence ("someone was at the back door at 22:15" + "access log shows Dana's card used at 22:15") to pin identity.

### 6.10 Anti-patterns to resist
1. **Special-casing** — if tempted, decompose into base rule + modifiers
2. **Overly specific emission types** — prefer abstract types that presentation specializes
3. **Scenario-authored rules** — no separate rules for "murder of spouse" vs "murder of coworker"; the participants differ, the rules don't
4. **Fixing tension via new event types or facets** — tension failures are constraint and staging failures (§5.5, §6.9), not emission failures

---

## 7. Layer 4: Narrative Tension

Layers 1–3 produce a *valid* case: solvable, causally coherent, within budget. Layer 4 is what makes it *compelling*. A case that satisfies everything above but fails Layer 4 is technically solvable but flat — one suspect dominates, the player collects evidence linearly, the experience is "wait until the weapon chain completes."

The goal of Layer 4 is the **commit → doubt → switch → resolve** arc: the player initially commits to a theory, encounters evidence that creates doubt, switches to an alternate theory, then resolves on the true killer as one of the two chains proves more complete under scrutiny.

### 7.1 Principles

1. **Dual retrograde with partial fracture.** Truth = full retrograde chain. Primary red herring = partial retrograde, coherent until one engineered break. Not symmetrical — partial. Forward-sim demotes to ambient world texture only.

2. **Constraint overlap, not constraint dominance.** Neither suspect should pass all axes. Distribute (motive / opportunity / physical_evidence / timing / witness_sighting) so each suspect fails on a different axis. *"Make both suspects feel slightly wrong — let the player decide which wrongness matters."*

3. **3.5/4 alignment for the promoted red herring.** The red herring passes all 4 axes (motive, opportunity, evidence, timing) but fails exactly 1 — preferably `timing`, because it's objective, resolvable, and produces clean "aha" moments. Avoid `motive` or `evidence` as the break axis — they feel subjective or arbitrary.

4. **1 strong + optional 1 weak noise suspect.** Not 2+ strong promoted suspects. Combinatorial fairness explodes otherwise. Weak suspects (2–3 of 4 axes) are acceptable noise; they are never promoted and never structurally complete.

5. **Weak links are non-binary.** The contradiction that disproves the red herring must be probabilistic, not absolute. "Lawyer says ~22:30, maybe a bit after" beats "confirmed at 22:30." Preserves tension into late game instead of collapsing to a hard switch.

6. **Redundant discovery paths.** At least 2 independent ways to reach any critical evidence, including the red herring's break. Correctness should depend on reasoning, not on covering one specific lead.

7. **Motive species differentiation.** When two suspects exist, their motives must be different *species* — acute consequence (about-to-be-fired, about-to-be-exposed) vs. structural grievance (ownership, inheritance, standing). Same-species motives collapse the comparison into fact-matching; different species force qualitative judgment.

### 7.2 Evidence source hierarchy (three tiers)

Every piece of evidence in the case originates from one of three sources:

**Tier 1 — Suspect-grade chains (retrograde).**
- Truth chain (§4.1) — the killer's actual path
- Primary red-herring chain (§4.2) — the promoted suspect's retrograde chain with engineered break

**Tier 2 — Evidence-modifier noise.**
- Real, unrelated events that complicate interpretation of Tier 1 evidence
- Canonical example: a coworker legitimately borrowed the murder weapon earlier that day, producing a second set of prints on the handle
- Not suspect-grade — these events never make their actor look like the killer
- Provides the **symmetric contradiction** required by §5.5.7 (the soft inconsistency against the killer's chain)
- Formally lives in `ambient_threads` but is **audited** by the generator to ensure it touches crime-relevant evidence

**Tier 3 — Ambient world noise.**
- Forward-simulated world texture: supplier deliveries, drunk regulars, neighborhood movement
- Never suspect-grade, never interacts with Tier 1 evidence interpretation
- Makes the world feel lived-in; provides flavor (a) red herrings per design pillar

### 7.3 Relationship to the earlier layers

- **§4 (Skeleton) provides the structure.** Two retrograde chains + ambient.
- **§5.5 (Narrative tension constraints) enforces the axis split, non-binary break, and symmetric contradiction.**
- **§6.9 (Passes 7/8/9) produces the moment-to-moment experience** — staging, bounded perspective, identity-omission cross-referencing.
- **§7 (this section) is the principle layer** — the "why" these constraints and passes exist.

A case that passes §5 validation but feels flat in playtest indicates a tuning problem in §5.5 or §6.9, not a problem with §7. §7 itself has no knobs; it is the design rationale.

---

## 8. Open / Deferred

- **Concrete threshold values** — "target accuracy" for solvability, red-herring suspicion thresholds, exact `who_acted` omission percentage (30–50% range). Tuning parameters, not structural. Calibrate empirically once a playable case exists.
- **Evidence staging heuristic details** — the early/mid/late tier rules in §6.9 are directional; the exact placement algorithm will need tuning against a reference case.
- **Bounded-testimony facet-drop heuristic** — §6.9 Pass 8 defines the rule in principle; the concrete position/attention model needs specification per testimony type.
- **Scoring that rewards reasoning process** — partial submission / confidence locking / revision distance. Parked until a playable case exists; v1 ships Wordle-style timeline scoring.
- **Difficulty levers** — deferred until a simple generator is running and can be tested empirically.
- **Archetype variation** — only murder in v1; other archetypes get their own skeletons later.
- **Group entity** — deferred; promote if/when a case archetype genuinely needs collective actors.
- **Evidence presentation / UI** — how each evidence type surfaces in-game and what action-point cost examining it has.
- **Content text generation** — how record `content_summary` strings get filled; how testimony prose gets authored. Not a structural problem yet.
- **Save/serialization format** — implied JSON given the typed-string-ID scheme, but not committed.

---

## 9. Guiding Principles (summary)

1. **Every attribute earns its place or is cut** — drives generation or is player-visible
2. **Constrained enums over free text** — anywhere the generator branches
3. **Compositional over combinatorial** — add rules, don't multiply
4. **Commit to constraints coarse-to-fine** — retrograde goes from open to fully specified
5. **Derive what you can, store only what you must** — state emerges from events wherever possible
6. **Every rule must answer "what does this do that existing rules don't?"** — or it's a variation, not a new rule
7. **Validity ≠ compellingness** — a case that passes §5.1–5.4 but not §5.5 is solvable but flat. Enforce narrative tension constraints as hard constraints, not soft goals.
8. **Two theories, each with a contradiction** — if the player can't hold two competing suspects in mind at the midgame, the case is underbuilt.

---

# Appendix A — Decisions already argued through

These were debated and closed. Reopening without new information wastes time. Each is listed with the reason it was rejected, so the argument required to reconsider is clear.

## Rejected approaches

| Rejected | Reason | What was chosen instead |
|---|---|---|
| Pure forward simulation | Runaway scale, shapeless output, no commitment to being a mystery | Retrograde + skeleton for truth AND red herring; forward-sim for ambient only |
| 100% hand-authored cases | No scale, no replayability | Retrograde with slot-filling |
| Silent misleading evidence (flavor b) | Requires modeling player inference to deceive fairly; research-grade problem | Flavor (a) only: irrelevant-but-real events as red herrings |
| In-session evidence deterioration | Complexity without game value | Deterioration baked into generated case as snapshot |
| Meta-progression (unlocks, skills, levels) | Deduction-game fantasies don't match "my detective leveled up" | Player skill is the only progression |
| Red herrings as forward-sim only | Can't produce structurally complete suspects — the review's "suspect promotion" problem | Primary red herring is retrograde with engineered break; forward-sim produces ambient only |

## Rejected data-model choices

| Rejected | Reason | What was chosen instead |
|---|---|---|
| `type` / `role` field on Person | Roles are relational — a Person is a witness *of* an event | Roles derived from Event participation |
| `means` as a top-level skeleton slot | Redundant — it's the `commission` event's `instrument` participant | No `means` slot; derived from commission |
| `why` / motive as a facet | Motive isn't directly observable through any single piece of evidence | `topic` facet; motive is emergent inference across topics |
| `observation` as an event type | Observation is a participant role, not a distinct event | `role=observer` on existing events |
| `reconciliation`, `learning`, `object_interaction` as event types | Derivable from other event types | Folded into existing 13 |
| `Evidence` as a Layer 1 entity | Conflates world state with game state | `Evidence` is derived from events in Layer 3 |
| Free-text `description` field on events | Derivable from participants + type; creates sync risk | Derive at presentation time |
| 3-value personality axes (extreme/moderate/extreme) | Middle value gives no generation signal | 4-value scheme (extreme_A / mild_A / mild_B / extreme_B) |
| Single scale combining intensity + valence | Loses information (intense-loving vs. intense-hating) | Separate `intensity` and `valence` on Relationship |
| N-person Relationships | Fundamentally pairwise; groups decompose to pairs + shared Place | Pairwise only; `Group` entity deferred |
| Separate `reliability` attribute on Person | Subsumable by personality axes | Reliability emerges from (axis_1, axis_2) |
| `symmetric` flag on Relationship | Adds a redundant field | Direction semantics carried by the type enum |
| Point-in-time `opportunity.TimeWindow` | Collapses timing tension | ToD is always a range, never a point |

## Rejected emission/rule choices

| Rejected | Reason |
|---|---|
| `pattern_deviation_signal` as base emission on `object_disposal` | Scenario-flavored; belongs in personality pass |
| `what_instrument` on `communication_text` device trace | Device isn't `role=instrument` on text events |
| Baking `delivery` into base emission rules | Delivery is personality-pass (Pass 4) territory |
| `alibi_record` as a Pass 4 emission | Alibi emerges from chain events' own emissions, not a new rule |
| Folding participant testimony into observer testimony | Different evidentiary pathways; always split |
| Multiple archetypes in v1 | Each archetype needs its own skeleton; scope creep |

---

# Appendix B — Conceptual anchors

Subtle enough to be worth re-stating:

- **Motive is inferred, not observed.** `topic` is the observable facet; motive is what the player argues for by connecting topics across the timeline.

- **`primary_thread` is partially a wrapper.** For a simple single-killer case, its attributes duplicate case-level info. It's kept because events reference their thread, so crime events need something to point at. Uniformity > minimalism here.

- **Object ≠ Evidence.** `Object` is world state (the knife exists). `Evidence` is derived from events at generation time (fingerprints on the knife are evidence).

- **Roles are relational.** A Person isn't a "witness" in their attributes — they're a witness *of* a specific event. Computed from `Event.participants` with `role=observer`.

- **Commission doesn't reveal the killer by itself.** If it did, the case would be trivial. Commission reveals the victim and that a killing happened; surrounding events reveal who.

- **Red herrings are real events, not fabrications.** They genuinely happen in the world; they just aren't the crime. Flavor (a) only. The red-herring chain's "fake anchor" (§4.2) is a generation-time scaffold, not a fabricated in-world event — the suspect's chain events all really happened, the scaffold just shaped retrograde fill.

- **The skeleton is visible to repeat players over time.** After 30–50 cases, pattern-matching is expected. Extending the horizon requires additional archetypes. Accepted as a known limit.

- **Attribute discipline.** Entities target 8–12 attributes. Every attribute must drive generation or be player-visible.

- **Validity is necessary but not sufficient.** A case passing §5.1–5.4 validation can still be a dud. Always re-check §5.5 narrative-tension constraints before accepting a generated case.

---

# Appendix C — Implementation context

- Project root: `C:\Users\alexb\Documents\QG-ChainLinks\tools\cc-casegen-v4`
- Language: **C-style C++**. Owner has explicitly mentioned disliking heavy string handling.
- **Memory arena pattern** for IDs: typed string IDs (`person_001`, etc.) interned into an arena, passed around as fixed-size handles at runtime.
- Serialization implied but not committed: typed-string IDs serialize cleanly to JSON, useful for saving/loading cases and test fixtures.
- No code yet — all current artifacts are this Markdown design.

---

# Appendix D — Working style

The owner values **honest feedback over validation**. Expected collaboration pattern:

- Challenge ideas with real weaknesses. Don't soften criticism into "interesting point!"
- Offer a specific counter-recommendation when rejecting an approach.
- Admit when the owner's pushback is correct and earlier suggestions were wrong.
- Verify claims against existing context before accepting a new direction.

Discipline applied to any new proposal:
- **"Does this earn its place?"** — every attribute, field, rule, entity. If it doesn't drive a generation decision or isn't player-visible, cut it.
- **"Is this scenario-flavored?"** — caught when rules drift toward specific scenarios instead of general structure.
- **"Compositional, not combinatorial"** — rules should add, not multiply.

Work proceeds through long, careful design conversations before writing code:
1. Present a topic and initial direction
2. Iterate on the design until every decision has a clear justification
3. Lock decisions into this reference document
4. Move to the next topic

Lists and concrete examples beat prose. When uncertain about an approach, describe the tradeoff explicitly and offer a recommendation. "I'd recommend X because Y, though Z is viable if A" beats both absolute certainty and noncommittal both-sides-ism.
