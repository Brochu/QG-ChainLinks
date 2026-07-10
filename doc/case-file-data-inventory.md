# COLD FILE — Case File Data Inventory (v2.1)
### The authoritative field-level schema for case logic files

> **Version history:** v2.0 flattening pass — hotspots dissolved into actions, triggers replaced by the `schedule` list, prereq DSL replaced by flat fact lists, `available` block ranges, one ink file per case, tuning targets moved to validator config · v2.1 staging file format defined (§14), reveal angles removed (`hidden_reveal` is validator-only).
>
> **Deferred (kept on record):** map zones / travel costs (re-add shape: single `travel_cost` int per location) · link types `CORROBORATES` `SUPERSEDES` `REINTERPRETS` · warrants; PD favor; lab credits · STAKEOUT verb · prereq expression DSL · trigger effects DSL · conditional action outcomes · fractional block costs · authored epilogue lines · manual board threads · mid-case inference puzzles.
>
> **Where this document and the design doc disagree on data format, this document wins.**

**Principles.** The case file is a static authored universe; the save is a diff against it. Case logic is engine-agnostic and playable with zero art; the staging file (per engine, §14) is a plain scene description per location. Prose lives in one ink file per case. JSON is the authoring source of truth permanently; binary is a compile target. Keys starting with `_` are authoring notes, stripped by the compiler, ignored by the validator's reference checks.

**Field table legend:** R = required, O = optional.

---

## 1. Root object

| Field | Type | R/O | Description |
|---|---|---|---|
| `schema_version` | string | R | Format version, e.g. `"2.0"` |
| `meta` | object | R | §2 |
| `glossary` | array | R | §3 |
| `facts` | array | R | §4 |
| `contradictions` | array | R | §5 |
| `locations` | array | R | §6 |
| `actions` | array | R | §7 |
| `interviews` | array | R | §8 |
| `schedule` | array | R (may be empty) | §9 |
| `reconstruction` | object | R | §10 |
| `outcome_tiers` | array | R | §11 |

## 2. `meta`

| Field | Type | R/O | Description |
|---|---|---|---|
| `case_id` | string | R | Stable identifier, kebab-case |
| `title` | string | R | Player-facing case title |
| `setting.date` | string | R | In-fiction date (era: mid-1990s) |
| `setting.region` | string | R | In-fiction region |
| `deadline_days` | int | R | Case length in days |
| `blocks_per_day` | int | R | Action blocks per day (slice: 3). Absolute block index = `day * blocks_per_day + block`, 0-based |
| `ink_file` | string | R | The single ink file for this case; all knot references resolve inside it |
| `briefing_knot` | string | R | Ink knot played at case start |
| `starting_facts` | string[] | R | Fact IDs discovered at start |
| `starting_locations` | string[] | R | Location IDs unlocked at start |
| `lab_queue_capacity` | int | R | Max concurrent lab (COLLECT) requests; further submissions queue, delay clock paused |

## 3. `glossary[]` — the noun universe

The runtime entity set is **derived**: it is exactly the tags of discovered facts. The glossary is a lookup table only.

| Field | Type | R/O | Description |
|---|---|---|---|
| `tag_id` | string | R | Key used in fact `tags` and reconstruction slots |
| `kind` | enum | R | `PERSON` `PLACE` `OBJECT` `VEHICLE` `ORG` `TIME` |
| `display_name` | string | R | Board / dropdown text |

## 4. `facts[]`

Facts are immutable and append-only.

| Field | Type | R/O | Description |
|---|---|---|---|
| `fact_id` | string | R | `F-xxx` |
| `text` | string | R | The claim as shown on the card |
| `tags` | string[] | R | Glossary tag IDs referenced by the claim |
| `reliability` | enum | R | `TESTIMONY` `DOCUMENT` `FORENSIC` |
| `conditions` | string[] | O | Observation metadata, TESTIMONY only (e.g. "dark", "~40m", "through window"); shown on the card; powers fair invalidation and rational `DOUBTED` tagging |

**Prerequisite lists (used everywhere):** every `prerequisites` / `unlock_rule` / state-`when` field in this format is a **flat array of fact IDs, AND semantics** (`null` or `[]` = none). There is no expression DSL. Three conventions cover the rest:

1. **OR** → the *shared knowledge-fact idiom*: author every alternative route to produce the **same fact**, gate on that one fact.
2. **Time conditions** → the `available` block range on actions (§7), never a prereq.
3. **"Contradiction active"** → list both of the contradiction's facts (active ≡ both discovered). Interview-side leverage uses ink's `contradictionActive()` instead.

## 5. `contradictions[]`

| Field | Type | R/O | Description |
|---|---|---|---|
| `contradiction_id` | string | R | `C-xx` |
| `facts` | string[2] | R | The two mutually-exclusive fact IDs (symmetric) |
| `resolution_actions` | string[] | R | Action ID(s) that resolve the tension; validator enforces ≥1 |

Dormant until both facts discovered; activation draws the board's red thread. **No player-facing hint or explanation field exists** — the thread is a flag, never an answer.

## 6. `locations[]`

| Field | Type | R/O | Description |
|---|---|---|---|
| `location_id` | string | R | Stable ID (also the staging key) |
| `name` | string | R | Player-facing name |
| `unlock_rule` | string[] \| null | R | Fact list (AND); `null` = only via `starting_locations` |
| `states` | array | R | Ordered diorama states, below; first entry is the initial state |

`states[]`:

| Field | Type | R/O | Description |
|---|---|---|---|
| `state_id` | string | R | Name referenced by actions' `location_states` and by staging |
| `when` | string[] \| null | R | Fact list (AND) that switches the location to this state; `null` on the initial state. Later states win over earlier when multiple match |

## 7. `actions[]` — every interactable thing in the game

Inspectables are actions too: verb `INSPECT`, cost 0. One record type, one visibility system, one staging binding.

| Field | Type | R/O | Description |
|---|---|---|---|
| `action_id` | string | R | `A-xx`; staging binds this to a diorama position |
| `verb` | enum | R | `INSPECT` `SEARCH` `COLLECT` `INTERVIEW` `CANVASS` `PHONE_FAX` |
| `location_id` | string | R | Where it lives (`PHONE_FAX` at the field office) |
| `label` | string | R | Player-facing description of what the action does |
| `cost` | int | R | Whole blocks; `0` for INSPECT. One PHONE_FAX block covers up to two requests (engine rule) |
| `prerequisites` | string[] \| null | R | Fact list (AND) |
| `hidden` | bool | R | With prereqs, derives visibility: prereqs met → **unlocked**; unmet & `hidden:false` → **locked** (shown, unselectable); unmet & `hidden:true` → **secret** (invisible) |
| `locked_hint` | string | O | Short tease shown on locked actions |
| `available` | int[2] | R | Absolute block range `[from, to]`, inclusive; `-1` = end of case; `[0,-1]` = always. Validator checks expiring critical-path actions have alternates |
| `blocks_of_day` | int[] | O | Periodic filter (e.g. `[2]` = evenings only); absent = all blocks |
| `location_states` | string[] | O | Diorama states this action exists in; absent = all states |
| `hidden_reveal` | bool | O | Default `false`. Marks actions found only by orbiting — staging places the point where geometry conceals it (§14). Validator-only flag: never the sole route to a critical-path fact |
| `delay` | int | R | Blocks spent before results mature; `0` = immediate. Maturity = pager headline. COLLECT actions additionally occupy a lab queue slot for the delay (engine rule from verb — no per-action flag) |
| `pending_label` | string | O | Required when `delay > 0`: in-fiction delay line + pager headline text |
| `produces` | string[] | R (may be empty) | Flat fact ID list, unconditional. Variation = multiple actions. Empty for INTERVIEW (manifest owns it, §8) |
| `repeatable` | bool | R | Almost always `false` |

## 8. `interviews[]`

| Field | Type | R/O | Description |
|---|---|---|---|
| `interview_id` | string | R | `IV-xx` |
| `action_id` | string | R | The INTERVIEW-verb action this payload belongs to (single direction: interviews point at actions, never both ways) |
| `character` | string | R | Glossary tag ID (PERSON) |
| `knot` | string | R | Knot inside `meta.ink_file` |
| `fact_manifest` | string[] | R | Every fact the ink knot can `discoverFact()`; validator cross-checks against the ink source |
| `re_interview` | bool | R | If `true`, action re-enables whenever a new fact tagged with `character` is discovered (engine rule) |

Engine↔ink contract (stable, part of this format): `hasFact(id)`, `contradictionActive(id)`, `discoverFact(id)`, `day()`, `spendBlock()`.

## 9. `schedule[]` — timed unprompted delivery

Things that happen at a *time* regardless of player activity. All other reactive behavior lives at its effect site: unlocks in `unlock_rule`s, enables in prereqs, state changes in `states[].when`, discovery pagers in `pending_label`.

| Field | Type | R/O | Description |
|---|---|---|---|
| `at_block` | int | R | Absolute block index at which delivery fires; **≥ 1** — the clock starts at 0 and delivery is checked as each spent block completes, so a block-0 entry can never fire. Block-0 content belongs in `starting_facts` or the briefing knot |
| `delivers` | string[] | R | Fact IDs discovered unprompted (they then drive unlocks/states/prereqs like any fact) |
| `pager` | string | R | Pager headline shown at delivery |

## 10. `reconstruction`

| Field | Type | R/O | Description |
|---|---|---|---|
| `verb_list` | array | R | `{id, text}` phrase pool for VERB slots |
| `motive_list` | array | R | `{id, text}` phrase pool for MOTIVE slots (decoy motives are just entries only used as decoys) |
| `events` | array | R | Below |
| `grading.assertion_weights` | object | R | `who / how / why / before_after / accessory` → weight, sums to 1.0 |
| `grading.event_band_map` | object | R | Assertion type → event IDs it grades |

`events[]`:

| Field | Type | R/O | Description |
|---|---|---|---|
| `event_id` | string | R | `E-xx` |
| `band` | enum | R | `BEFORE` `DURING` `AFTER` |
| `template` | string | R | Sentence with slot placeholders, e.g. `"[ACTOR] [VERB] at [PLACE], [MOTIVE]."` |
| `slots` | array | R | Below, in template order |

`slots[]`:

| Field | Type | R/O | Description |
|---|---|---|---|
| `kind` | enum | R | `ACTOR` `VERB` `OBJECT` `PLACE` `TIME` `MOTIVE` |
| `answer` | string | R | Correct glossary ID (entity kinds) or phrase ID (VERB/MOTIVE) |
| `alternates` | array | O | `{id, weight}` partial-credit answers, weight in (0,1) |
| `decoy_pool` | string \| string[] | R | Explicit ID list, or a pool token: `discovered_persons` `discovered_places` `discovered_vehicles` `discovered_objects` `discovered_times` |
| `supports` | string[] | R | Fact IDs justifying the answer; powers the derived post-game review (had / missed / ignored) |

Validation on final submission only. Submission is ceremonial (signed memo), irreversible.

## 11. `outcome_tiers[]` — single source for thresholds

| Field | Type | R/O | Description |
|---|---|---|---|
| `tier` | string | R | `case_closed` `plea_bargain` `mistrial` `unsolved` |
| `min` | float | R | Minimum score for this tier (list ordered descending; no duplicate threshold table anywhere else) |
| `knot` | string | R | Outcome memo knot inside `meta.ink_file` |

## 12. Save state (NOT in the case file)

Discovered fact set · known entity tags (cache of discovered facts' tags — the derived noun universe) · active contradictions · player tags (`CLEARED` `DOUBTED` `KEY`; no entry = untagged), board layout, annotations · clock (absolute block index) · completed non-repeatable actions · pending-results list & lab-queue occupancy/order · ink state blob · reconstruction draft.

**Not stored, derived on demand:** per-location current state (a pure function of the discovered-fact set) and fired schedule entries (the per-block tick passes each block index exactly once, so `at_block == clock` fires exactly once by construction).

## 13. Validator laws (run on JSON; config in the validator, not the case)

No dangling IDs anywhere · every fact producible (action, schedule, or starting) · `schedule.at_block` ≥ 1 (§9) · every glossary entry appears in ≥1 fact; every reconstruction answer/decoy introducible via some fact · fact connection-density ≥ 2 (shared tag / contradiction / prereq / supports) · every contradiction has ≥1 reachable resolution action · critical path reachable within the block budget · actions with bounded `available` on the critical path have alternates · `hidden_reveal` never the sole route to a critical fact · interview manifests ⊇ ink `discoverFact` calls · `pending_label` present wherever `delay > 0` · exactly one recontextualizing beat per case (checked by a human, flagged by convention) · economy targets (~60% affordability, ~4 facts/day) from validator config.

## 14. Staging file (per engine; NOT part of the case file)

The case file contains zero visual data. The staging file fills that gap: **one scene-description file per location**, sharing only the ID vocabulary (`location_id`, `state_id`, `action_id`) with the case file. Whether staging is hand-authored or emitted by a generator tool, the runtime cannot tell the difference.

```json
{
  "location_id": "motel-unit",
  "camera": { "pivot": [0, 1, 0], "zoom": [2.5, 6.0] },
  "states": {
    "burned": {
      "props": [
        { "asset": "bed_double_burned", "pos": [1.4, 0, 0.8], "rot": 90 },
        { "asset": "nightstand",        "pos": [2.2, 0, 0.8] }
      ],
      "action_points": [
        { "action_id": "A-07", "pos": [2.2, 0.5, 0.9] },
        { "action_id": "A-12", "pos": [0.3, 0.1, 1.6] }
      ]
    },
    "bulldozed": {
      "props": [ { "asset": "rubble_pile", "pos": [1.0, 0, 1.0] } ],
      "action_points": [ { "action_id": "A-19", "pos": [1.0, 0.4, 1.0] } ]
    }
  }
}
```

- **`props` — full list per state.** Every state fully defines its scene; no diffing or inheritance between states. Redundant, trivially debuggable, and what a generator emits anyway.
- **`action_points` — a position per action playable in that state.** The runtime spawns a clickable marker for each point whose action is currently visible per the logic rules; clicking hands the `action_id` to the rules engine — the same call the text prototype makes.
- **Concealment is geometric.** A `hidden_reveal` action point is simply placed where geometry occludes it (under the drawer, behind the headboard) until the camera orbits past. No authored angles anywhere.
- **Deferred bindings**, added only when they hurt: portraits, audio, block-of-day labels.
