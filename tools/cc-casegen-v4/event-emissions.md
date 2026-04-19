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
