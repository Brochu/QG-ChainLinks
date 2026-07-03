# COLD FILE — Case File Data Inventory (v1.0 — locked with design doc v1.0)
### Everything a case file must contain, before we argue about structure

> **v0.2 changes (post-review):** entities replaced by derived set + glossary (§2); links reduced to CONTRADICTS only, rest deferred (§4); warrants cut; PD favor cut pending confirmation (§1); interviews delegated to ink with a lintable manifest (§7); epilogue reduced to outcome tiers + derived review (§11); actions use flat `produces` + tri-state visibility (§6); binary format noted as compile target (§13).

Guiding principle from the design doc: **the case file is a static authored universe; the save file is a diff against it.** This inventory covers authored data only — runtime state is listed at the end for contrast, but lives in the save.

Second principle: **separate case logic from staging.** The case file must be playable by the text/debug prototype with zero art. Asset bindings (diorama meshes, hotspot 3D positions, portraits, audio) live in a parallel *staging file* referencing logic IDs.

Third principle (new): **prose lives in ink.** Briefings, interviews, document texts, and outcome memos are ink files; the case file holds IDs, rules, costs, and short labels. Ink already being external text does half the localization split for later.

---

## 1. Case metadata

| Field | Purpose |
|---|---|
| `case_id`, `title`, `setting` (date, region) | Identity & flavor |
| `deadline_days`, `blocks_per_day` | The clock (10 × 3 in the slice) |
| `briefing_ink` | Ink knot for the opening; delivers starting facts via manifest |
| `starting_facts`, `starting_locations` | Discovered/unlocked at case start |
| `starting_resources` | None — **time blocks are the only spendable resource.** Forensic access is throttled structurally instead: `lab_queue_capacity` (slice: 2) caps concurrent lab requests; extra submissions queue behind active ones, their delay clocks not starting until a slot frees. Throttles the FORENSIC tier (the only never-wrong facts) without a second currency, and turns lab use into a sequencing decision. *(PD favor and lab credits: cut, see Deferred)* |
| `tuning_targets` | Economy numbers for the validator (target ~60% action affordability), not read by the game |

## 2. Glossary (replaces the entity list)

The entity *set* is derived at runtime from the tags of discovered facts — dropdowns and board anchors only ever contain nouns the player has actually encountered. The glossary is a pure lookup table so display data isn't duplicated across facts:

| Field | Purpose |
|---|---|
| `tag_id` | The key used inside fact tags |
| `kind` | PERSON / PLACE / OBJECT / VEHICLE / ORG — used for reconstruction slot filtering |
| `display_name` | Board and dropdown text |

**Times** are flat authored `time_token`s (`~9 PM`, `after the rain`...) in the same glossary with kind TIME. The game matches tokens; it never does time math.

Validator rule: every glossary entry must appear in ≥1 fact's tags; every reconstruction answer and decoy must be introducible through some discoverable fact.

## 3. Facts

| Field | Purpose |
|---|---|
| `fact_id`, `text` | The claim, as shown on the card |
| `tags` | Glossary tag IDs (entities + time tokens) |
| `reliability` | TESTIMONY / DOCUMENT / FORENSIC |
| `conditions` | Observation metadata, testimony only (dark, distance, obstruction...) — visible on the card; powers fair invalidation |

Facts do **not** store their source action — actions declare what they produce (single direction of reference).

## 4. Contradictions (the surviving link type)

Reduced from the four-type link system: `CORROBORATES` lost both its consumers when warrants and the authored epilogue were cut; `SUPERSEDES`/`REINTERPRETS` were presentational only (the keystone recontextualization works with no data — the player holds both cards and sees it). **Deferred, not deleted** — revisit if playtesting shows the flip moment needs presentational support.

| Field | Purpose |
|---|---|
| `contradiction_id` | Identity |
| `facts` | The two fact IDs (symmetric — no direction) |
| `resolution_actions` | Action ID(s) that resolve the tension — validator enforces ≥1 |

Activates when both facts are discovered. Consumers: board red thread (no explanation given), interview leverage (confrontation topics may require an active contradiction involving that character), validator resolvability check.

## 5. Locations

| Field | Purpose |
|---|---|
| `location_id`, `name` | Identity |
| `map_zone` | Travel cost via a small zone-to-zone table |
| `unlock_rule` | Prereq expression (§9), or unlocked at start |
| `states` | Ordered named states (`crime_scene_fresh`, `bulldozed`) with activating trigger |
| `hotspots` | Logic level: each is an `inspectable` (free, yields fact IDs) or an `action_id` reference; per-state visibility; `hidden_reveal: true` flags angle-discovery hotspots so the validator enforces "never critical-path without a secondary route" (the angle itself is staging data) |

## 6. Actions

| Field | Purpose |
|---|---|
| `action_id`, `verb` | Fixed vocabulary for the slice: SEARCH / COLLECT / INTERVIEW / CANVASS / PHONE_FAX (STAKEOUT deferred; vocabulary grows with future cases) |
| `location_id` | Where it lives (PHONE_FAX at the Field Office) |
| `label` | Text shown to the used when hovering this action |
| `cost` | Whole blocks only (1, 2...). PHONE_FAX covers up to two requests per block. COLLECT actions additionally occupy a lab queue slot for their delay duration |
| `prerequisites` | Expression (§9) |
| `hidden` | Authored data is just the `hidden` boolean; the states fall out of prereq evaluation |
| `locked_hint` | Optional short label shown on locked actions — the tease is itself a deduction motivator |
| `time_gate` | Allowed blocks-of-day and/or day range |
| `expires` | Optional day/trigger after which it's gone; validator checks expiring critical-path actions have alternates |
| `delay` | Blocks-spent before results mature (0 = immediate); maturity = pager event |
| `produces` | **Flat** fact ID list — no conditional branching. Variation is authored as multiple actions differing in prereqs/visibility |
| `pending_label` | In-fiction delay line + pager headline at maturity |
| `repeatable` | Almost always false |

## 7. Interviews (ink integration)

Interviews are actions (`verb: INTERVIEW`) whose payload is an ink reference:

| Field | Purpose |
|---|---|
| `character` | Glossary tag ID |
| `knot` | The script that owns all flow and prose |
| `fact_manifest` | **Every fact ID the ink script can produce.** The validator cross-checks this against `discoverFact(...)` calls in the ink source; reachability analysis stays intact even though flow logic lives in script |
| `re_interview` | Available again whenever the manifest contains undiscovered facts whose ink-side gates could now open (cheap approximation: re-enable when any new fact tagged with this character is discovered) |

Engine↔ink contract (external functions): `hasFact(id)`, `contradictionActive(id)`, `discoverFact(id)`, `day()`, `spendBlock()` if a script needs it. Keep the surface tiny and stable — it's effectively part of the case format.

## 8. World triggers & schedules

| Field | Purpose |
|---|---|
| `trigger_id` | Identity |
| `when` | Day/block reached, OR prereq expression satisfied, OR blocks-spent count |
| `effects` | Change location state; retarget a character's interview action; enable/expire actions; deliver an unprompted fact (SAC phone call pattern) |

Recurring schedules (waitress works evenings) are just `time_gate` on interview actions; triggers are for one-shot changes.

## 9. Prerequisite expressions (shared mini-format)

Used by unlock rules, action prereqs, and triggers: **AND/OR/NOT over fact IDs, plus `contradiction_active(id)` and `day >= n`.** No scripting — everything stays analyzable by the validator. Anything needing more expressiveness becomes a trigger or a second action.

## 10. Reconstruction

| Field | Purpose |
|---|---|
| `events[]` | `event_id`, band (BEFORE/DURING/AFTER), ordered slot templates |
| `slots` | Kind (ACTOR / VERB / OBJECT / PLACE / TIME / MOTIVE); correct glossary ID or phrase; accepted alternates with partial-credit weights; decoy pool ("all discovered PERSONs" is the default pool rule) |
| `verb_list`, `motive_list` | Authored phrase pools for non-entity slots |
| `grading` | Per-event and per-slot weights; score→outcome tier thresholds |
| `supports` | Per-slot fact IDs that justify the answer — powers the derived post-game review |

## 11. Outcomes (reduced epilogue)

| Field | Purpose |
|---|---|
| `outcome_tiers` | Threshold + one ink knot per tier (Case Closed / Plea Bargain / Mistrial / Unsolved) |
| *(derived)* | Post-game review generated from data: player timeline vs. true timeline, per-slot `supports` facts marked had / missed / ignored; undiscovered-fact teasers labeled by producing action |

## 12. What is NOT in the case file (save state)

Discovered fact set · active contradictions · player tags (CLEARED/DOUBTED/KEY), board layout, annotations · clock (day, block) · blocks-spent counter, pending-results queue, lab queue occupancy · interview/ink state (ink's own save blob) · reconstruction draft · location current-state index.

## 13. Format & pipeline decisions (settled)

1. **Validator is the second program built** (after the loader): dangling IDs, fact connection-density ≥2, contradictions resolvable, critical path reachable within block budget, expiring critical actions have alternates, manifest↔ink cross-check, glossary coverage.
2. **Text embedded / in ink for now**; string tables when localization becomes real (ink localization approach TBD — investigate).
3. **No conditional `produces`** — multiple actions + tri-state visibility instead.
4. **JSON is the authoring source of truth, permanently.** Binary is a compile target for shipping, produced by the build step, never hand-edited. The validator runs on JSON.

---

## Deferred (cut from slice, kept on record)

- Link types `CORROBORATES`, `SUPERSEDES`, `REINTERPRETS`
- Warrants & judge rules (fact-gated locked actions cover the need)
- PD favor resource (confirmed cut)
- Lab credits (replaced by lab queue capacity; re-add only if playtests show forensic brute-forcing)
- Authored per-gap/per-error epilogue lines (derived review replaces them)
- Conditional action outcomes
- STAKEOUT verb
- Fractional block costs
