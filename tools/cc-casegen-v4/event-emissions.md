# Base Emission Rules — Event Types

Base emissions for 12 of the 13 event types (all except `commission`, which is the worked example in `design-notes.md` Section 6). See Section 6 for the framework: 4 evidence types, 7 facets, the 4-pass emission process, and the rule budget (≤5 base emissions per type). This file contains **base emissions only** — place, object, and personality modifiers are separate passes. In particular, testimony `delivery` is a personality-pass concern and is never set here.

---

## 1. meeting base emissions

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

---

## 2. argument base emissions

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

---

## 3. threat base emissions

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

---

## 4. movement base emissions

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

---

## 5. object_acquisition base emissions

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

---

## 6. transaction base emissions

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

---

## 7. communication_call base emissions

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

---

## 8. communication_text base emissions

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

---

## 9. communication_written base emissions

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

---

## 10. violence base emissions

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

---

## 11. object_disposal base emissions

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
- Per design guidance, disposal surfaces as `derived_signal` (absence) first; direct traces of the vanished object itself are not emitted here — those belonged to the earlier event that touched the object.
- `pattern_deviation_signal` (e.g., "took the trash on an odd day") has been removed from the base rules. It's scenario-flavored and not a universal property of disposal. A `calculating` killer may plan disposal to minimize such deviations; a `confrontational` or `impulsive` one may produce them. That tuning belongs to the personality pass (Pass 4), not base emissions.

---

## 12. discovery base emissions

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

-------------------

# Event Modifiers — Passes 2, 3, 4

Modifiers for the 4-pass emission process defined in `design-notes.md` §6. These are **standalone cross-cutting tables** that apply to any event whose Place, participating Object, or actor matches the condition. They do not repeat per event type. Pass 1 (base emissions) lives in `event-emissions.md`; the worked `commission` example in §6 of the design notes is the calibration point for tone and granularity.

Modifiers either **add** an emission or **tune** a base emission (more/fewer/suppressed/upgraded/downgraded). They reference only the 4 evidence types (`testimony`, `physical_trace`, `record`, `derived_signal`) and the 7 facets (`who_acted`, `who_affected`, `when`, `where`, `what_type`, `what_instrument`, `topic`). They never name specific scenarios.

Budget: ≤3 modifiers per place property, ≤3 per object property, and a reasonable cap on personality modifiers. Every rule below has been checked against "does this earn its place?"

---

## Pass 2 — Place modifiers

Place properties referenced: `type`, `access_level`, `security`, `operating_hours`, `connected_to`, `capacity`.

### type

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

Note: `residence`, `commercial`, `public`, `industrial` intentionally have no type-level modifier — their evidentiary character is already captured by `access_level`, `security`, and `operating_hours`. Only `outdoor` (degrades traces) and `transport` (produces a manifest record not otherwise emitted) earn a rule.

### access_level

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
    expansion as a union, not a product; the larger pool wins and the effect
    is idempotent at the upper bound (no runaway observer stacks).
```

### security

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
    (no infrastructure to produce either; security=none is the absence case)
```

### operating_hours

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
    (no "off" window to deviate from)
```

Note: only two modifiers — the facet of operating_hours is essentially a single binary (in-window vs out-of-window). A third rule would be padding.

### connected_to

```
if place.connected_to is non-empty AND event has no direct observer:
  + adjacent_place_testimony : testimony  (CONDITIONAL — per occupant of a connected Place)
    - source = occupants of places in connected_to during event.time
    - content = overheard sounds, glimpses through shared boundary
    - facets = { when, where };
              { what_type } partial (raised voices, struggle, etc.);
              { who_acted } only if connection permits visual identification
```

Note: `connected_to` is load-bearing for trajectory evidence (per §3.2). Only one modifier earns its place here — adjacency for stationary events. The movement case (transit_observer along the connected_to path) is already covered by the base `transit_observer` rule in event-emissions.md §4, which is defined as conditional per connected Place. Re-emitting it here would double Pass 1.

### capacity

```
if place.capacity == high:
  - observer_testimony population EXPANDED
    (dense foot traffic; more conditional observer emissions fire)
    Composition note: see access_level=public above — this rule composes
    additively with that one; the larger pool wins.

if place.capacity == low:
  - observer_testimony population CONTRACTED
    (sparse occupancy; most conditional observer emissions do not fire)
```

Note: capacity is the density dial for observer testimony — two rules tune the observer pool symmetrically in both directions. The visibility=private nuance is implicitly handled by Pass 4's secretive-actor rules (which suppress observer testimony regardless of capacity) and by `off_hours_signal` for out-of-window activity; no separate crowd-noise signal earns its place.

---

## Pass 3 — Object modifiers

Object properties referenced: `type`, `physical_traits`, `portability`, `owner`, `origin_location`. Modifiers apply when the Object participates in the event — either as `role=instrument` or as a touched object (e.g., the acquired object on `object_acquisition`, the disposed object on `object_disposal`, the document on `communication_written`).

### type (enum: weapon / document / electronic / personal / environmental / consumable / misc)

```
if object.type == electronic:
  + device_activity_log : record  (record_type = access_log)
    - storage = digital_system (on the device or its service)
    - facets = { what_instrument, when };
              { who_acted } if the device is individually-accounted;
              { who_affected } if the logged activity names a counterparty

if object.type == document AND event.type != communication_written:
  - the document Object's participation in a non-written event (e.g., a
    transaction using a contract; a meeting where a dossier is consulted;
    an object_acquisition of a sealed letter) UPGRADES the trace already
    emitted for that event (actor_possession_trace / place_presence_trace)
    to also carry the `topic` facet.
    - facets added = { topic };
                     { who_affected } if the document is signed/addressed to a
                                       party not already named in the base emission
    Condition (no double-count): this rule does NOT fire on communication_written
    events — those already emit `document_record` per event-emissions.md §9,
    and `document_artifact` already carries topic. Firing here would duplicate
    that record. This rule specifically earns its place for documents that
    show up as props in other event types.

if object.type == consumable:
  - actor_possession_trace / instrument_traces DOWNGRADED toward faint/degraded
    (consumed material destroys its own trace surface)
```

Note: `weapon`, `personal`, `environmental`, `misc` take no type-level modifier — their evidentiary behavior is fully captured by the base `physical_trace` emissions plus `physical_traits` / `portability` below. Only the three types that change the *kind* of evidence (digital log, content record, destroyed surface) earn a rule.

### physical_traits

```
if object.physical_traits includes "individually_identifying"
   (serial number, engraving, unique pattern):
  - actor_possession_trace / instrument_traces UPGRADED
    (facets now include { what_instrument } reliably and { who_acted }
     when paired with a possession record)

if object.physical_traits includes "leaves_distinctive_mark"
   (bladed, inked, pigmented, patterned surface):
  - instrument_traces UPGRADED
    (trace_type ∈ { tool_mark, damage, residue } as appropriate for the mark;
     facets add { what_instrument, what_type };
     { who_affected } added if the mark is on the subject's body rather than
     the place)
    Rationale: tunes the base `instrument_traces` emission rather than adding
    a parallel physical_trace — matches the "tune, don't add" pattern from
    the §6 commission worked example and saves a budget slot.

if object.physical_traits includes "fragile"
   (and object participates as instrument or target):
  + object_damage_trace : physical_trace
    - place = event.place
    - trace_type = damage
    - facets = { what_instrument, where, what_type }
```

Note: `physical_traits` is a free map, but these three traits are the ones the generator actually branches on. Other traits (color, size) don't drive emissions and don't earn rules.

### portability

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
    (fixed objects don't leave; disposal of a fixed object is incoherent and
     should fail skeleton validation, not emit)
```

Note: portability is a binary with two genuinely different evidentiary consequences — one additive, one suppressive. A third rule would pad.

### owner

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
    (public / unowned objects have no custody baseline to deviate from)
    Earns its place: without this rule, the conditional owner_testimony above
    would misfire on owner-less objects — producing testimony sourced from
    a null Person. The suppression is load-bearing, not bookkeeping.
```

### origin_location

```
if object.origin_location has security == high OR access_level == restricted:
  + origin_access_record : record  (record_type = access_log)
    - storage = digital_system at origin_location
    - facets = { who_acted, when, where = origin_location, what_instrument }

if event.place != object.origin_location AND object.portability == portable:
  - trajectory evidence for the object's movement from origin to event.place is
    DEFERRED to the movement / object_acquisition event that actually carried it
    (not re-emitted here — avoids double-counting; see §6 design notes on chain
     interaction vs. special-case rules)
```

Note: only two modifiers. The first produces a record that wouldn't otherwise exist; the second is an explicit *non-emission* rule that prevents double-counting across events — earning its place by protecting the compositional budget.

---

## Pass 4 — Actor-personality modifiers

Personality is `(axis_1, axis_2)` per §3.1: impulsive ↔ calculating and confrontational ↔ secretive, with 4 values per axis (`extreme_impulsive`, `mild_impulsive`, `mild_calculating`, `extreme_calculating`; same pattern for axis_2). Mild and extreme share direction at different amplitudes — mild rules tune, extreme rules tune harder.

Pass 4 **owns** `delivery` on testimony (`volunteered` / `requires_pressure` / `hostile`). No other pass sets it.

### axis_1 — impulsive ↔ calculating

Drives the temporal/physical shape of action: messy vs. clean, present vs. scrubbed.

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

Note: earlier drafts placed an `alibi_record` emission here. It has been removed — alibi construction is cover-up logic and belongs in Pass 5 (downstream event adjustments), OR, preferably, emerges naturally: a calculating killer's chain will include ambient events (a calendar_entry for a "dinner," a transaction elsewhere) that emit their own records via Pass 1. Every other Pass 4 rule *tunes existing emissions*; adding a new record type here would break that pattern and duplicate what Pass 5 / ambient chain events already cover.

### axis_2 — confrontational ↔ secretive

Drives the evidence profile: human-witness trail vs. documentary/physical trail.

```
if actor.axis_2 == extreme_confrontational:
  + adjacent_place_testimony : testimony  (rule — fires even if connected_to is empty,
                                           drawing from any observer in earshot)
    - content = overheard raised voices, shouting, public display
    - facets = { when, where };
              { what_type } partial;
              { who_acted } if voice-identifiable
  - observer_testimony facets EXPANDED
    (loud/public conduct gives any observer more to report — observers who
     would otherwise only cover { who_acted, when, where } now also cover
     { what_type, topic } where the content is audible)
  - any testimony sourced from this actor: delivery = volunteered

if actor.axis_2 == mild_confrontational:
  - any testimony sourced from this actor: delivery = volunteered
    (same direction as extreme, smaller amplitude — no facet expansion,
     no adjacent_place rule)

if actor.axis_2 == mild_secretive:
  - observer_testimony population CONTRACTED (actor chose quieter contexts)
  - any testimony sourced from this actor: delivery = requires_pressure

if actor.axis_2 == extreme_secretive:
  - observer_testimony SUPPRESSED where conditional
    (actor chose no-witness opportunity — even conditional observers don't fire)
  - adjacent_place_testimony SUPPRESSED
  - any testimony sourced from this actor: delivery = hostile
```

Note: delivery is set in a single place per axis_2 value. A confrontational subject on someone else's event is still confrontational about it — delivery tracks the source Person, not the event. This is consistent with delivery being a property of testimony, not of the base emission.

### cross-axis interaction (generator note — not an emission rule)

A killer configured as `extreme_calculating` on axis_1 AND `extreme_secretive` on axis_2 is the thin-evidence worst case: cleaned scene from the calculating side, suppressed observers and hostile-delivery testimony from the secretive side. No new rule fires here — this is purely the composition of the two axis rules already defined above.

Called out only so the generator is aware that when the killer is this configuration, the facet coverage check (Pass 6) will need to lean on downstream events (threats, prior arguments, trajectory records, ambient red-herring thread evidence that happens to name the killer) to hit the solvability floor. If Pass 6 fails, the fix is upstream — add an event to `precipitating` or `aftermath` — not a new modifier here.

### observer personality (separate case)

```
if observer.axis_2 == extreme_confrontational:
  - observer_testimony: delivery = volunteered

if observer.axis_2 == mild_confrontational:
  - observer_testimony: delivery = volunteered

if observer.axis_2 == mild_secretive:
  - observer_testimony: delivery = requires_pressure

if observer.axis_2 == extreme_secretive:
  - observer_testimony: delivery = hostile
  - observer_testimony content facets CONTRACTED
    (reluctant observer offers less even when pressed)
```

Note: delivery applies to whichever Person sources a testimony, not just the actor. Mirroring the axis_2 rules onto observers keeps the handling symmetric without adding new structural emissions — it only tunes `delivery` and, at the extreme, the facet subset.

---

### Pass 4 axis_1 notes

axis_1 is not mirrored onto observers. An observer's impulsiveness doesn't change the evidence their observation produces — it changes whether they act on it, which is a separate event (movement, communication, discovery) that runs through its own pass pipeline. Keeping axis_1 actor-only prevents accidental double-application.
