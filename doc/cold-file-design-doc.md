# COLD FILE — Game Design Document (Working Title)
### A 1990s FBI deduction game · Vertical slice design · v2.2 — **living document** (unlocked)

> **v2.2 changes:** doc **unlocked** — after working inside the format during slice implementation, the doc now evolves alongside the code, with changes tracked in this changelog. Added §13 "Known Expressiveness Limits": a monotonicity stress-test of the v2.0 format, identifying `closes_on` and internal-fact pager/board exemptions as the shaped re-adds held in reserve.
>
> **v2.1 changes:** staging simplified — a staging file is a per-location scene description: each state carries its full prop list (no diffs between states) plus the 3D position of every action point (§2, §9.1; format sketch in Data Inventory v2.1 §15). Reveal angles cut: concealed action points rely on geometric occlusion, and `hidden_reveal` becomes a validator-only flag. Location states confirmed as speced.
>
> **v2.0 changes (format-sync with Case File Data Inventory v2.0, driven by first authoring experience):** hotspots dissolved — everything interactable is an action, inspectables are cost-0 INSPECT actions (§2, §6); prereq DSL replaced by flat fact lists + three conventions (§5.3); trigger system replaced by the `schedule` list — all other effects live at their effect site (§5.5); map zones & travel costs cut for the slice (§4.1, §12); availability expressed as an absolute block range + optional `blocks_of_day` (§5.3); one ink file per case, knot-only references; tuning targets moved to validator config.
>
> **v1.0 changes:** all costs are whole blocks — PHONE_FAX costs 1 block and covers up to two requests (§4.1, §6); STAKEOUT deferred from the slice verb set (§6, §12); manual board linking cut — player expression on the board is annotations + CLEARED/DOUBTED/KEY tags (§5.4, §12). Open questions pruned to post-slice topics. Changes past this point require unlocking the doc.
>
> **v0.3 changes (simplification pass):** time is the only spendable resource; lab queue capacity replaces lab credits (§4.3); warrants & PD favor cut (§4.4); link system reduced to contradictions, rest deferred (§5.2); glossary replaces entity list (§5.1); prose delegated to ink (§9.1); tri-state action visibility (§5.3); epilogue reduced to outcome tiers + derived review (§7.3); open questions updated (§11). Deferred features tracked in §12.
>
> **v0.2:** block-based delays + pager, fact immutability, player tagging, runtime pipeline, fact economy targets.

---

## 1. Concept

The player is an FBI investigator in the mid-1990s assigned to a single case. They explore a small set of handcrafted **diorama locations**, spend **limited time** on investigative actions, and assemble an **evidence graph**. The case ends — by choice or by deadline — with a **reconstruction**: the player builds a timeline of what happened before, during, and after the crime, assembled from facts they actually discovered. The reconstruction is graded with partial credit.

**Design pillars**

1. *Deduction is the gameplay.* Information must be interpreted, cross-referenced, and doubted — not just collected.
2. *Every action is a tradeoff.* Time forces the player to choose between leads, never punishes them for engaging.
3. *The world is 1995.* No cell phones, no instant databases. Distance, delay, and paperwork are mechanics.
4. *The game never tells you if you're right* — until the reconstruction is submitted.

**Reference points:** The Case of the Golden Idol (deduction verification), Return of the Obra Dinn (delayed validation), Her Story (player-driven inference), Shadows of Doubt (evidence linking), Lumino City / Captain Toad (diorama presentation).

---

## 2. Presentation

Each location is a **self-contained 3D diorama** on a neutral backdrop:

- **Camera:** orbit around a fixed pivot, with zoom and limited vertical tilt. Optionally 2–3 preset "focus pivots" per diorama (e.g., the desk, the body, the doorway) the player can snap between.
- **Interaction:** mouse raycast onto action points. Everything clickable is an **action** in the data — free inspectables are simply cost-0 INSPECT actions, so one record type, one visibility system, and one staging binding cover the whole game. The staging file maps each `action_id` to its 3D position in the diorama.
- **Discovery through orbiting:** some action points are only visible from certain angles (a note taped under a drawer, a bullet hole visible only from the window side). The camera itself is an investigative verb. Use sparingly — 2–3 per diorama, never for critical-path evidence without a secondary route (validator-enforced via the action's `hidden_reveal` flag). Concealment is physical, not authored: the action point is staged where geometry hides it until the camera orbits past — no angle data anywhere.
- **Diorama state changes:** dioramas can change between visits (a cleaned-up crime scene, a suspect's packed suitcase) to telegraph the passage of time and the cost of delay.
- **UI layer:** the case file, evidence board, map, and reconstruction board are full-screen 2D interfaces styled as period paperwork — manila folders, typewritten reports, fax printouts, Polaroids.

Scope for the slice: **5–7 dioramas**, each roughly the size of a room or storefront.

---

## 3. Core Loop

```
Field Office (hub)
  └─ Review case file / evidence board
  └─ Choose a location on the map
        └─ Orbit diorama, inspect freely
        └─ Take costed actions ──► gain facts / evidence / new leads
  └─ New facts unlock new actions & locations
  └─ Delayed results mature as blocks are spent ──► pager buzz, mid-block
  └─ Repeat until: player attempts reconstruction OR clock expires
Reconstruction board ──► graded outcome ──► derived case review
```

The hub is the **Field Office**, itself a diorama. It's where phone/fax actions happen, where the evidence board lives, and where matured results are read in full — keeping a natural rhythm of *plan → execute → review* while the pager (§4.2) lets headlines ambush the player in the field.

---

## 4. Resource System

**Time is the only spendable resource.** Everything else that gates content is either structural (lab queue) or earned through evidence (prerequisites, leverage).

### 4.1 Time

- The case has a **deadline: 10 in-game days** (tunable).
- Each day has **3 action blocks** (morning / afternoon / evening).
- Travel is free in the slice — map zones and travel costs are cut (§12); distance is expressed narratively and through delayed remote requests instead. (Flatter re-add if ever wanted: a single `travel_cost` integer per location.)
- Some actions are **time-bounded**: an absolute availability range in blocks (`available: [from, to]`, `-1` = end of case) covers "only from day 5" and "gone after day 7" alike; an optional `blocks_of_day` filter covers recurring windows like a witness who is only at the diner in the evenings.

**Why time blocks instead of abstract points:** they create scheduling puzzles ("I can hit the motel and the pawn shop today, but the coroner closes at 5"), they justify the deadline narratively (the Bureau reassigns you), and they make delayed results meaningful.

**All costs are whole blocks.** No fractional accounting: easier to balance, instantly parsable by the player. The "cheap" feel of desk work is preserved by batching — one PHONE_FAX block covers up to two requests.

### 4.2 Delayed results — the planning layer

Some actions don't pay off immediately. **Delay is measured in action blocks the player spends**, not calendar days: a "delay: 6" request delivers after 6 more blocks of investigation. In-fiction, dialogue still describes delays in approximate time ("Quantico says a couple of days"); the engine counts spent blocks.

| Action | Delay (blocks spent) |
|---|---|
| Lab analysis (fibers, ballistics, blood type — pre-DNA-routine) | 6–8 |
| Records request by fax (DMV, bank, phone company) | 3–5 |
| VICAP / NCIC query | 2–3 |
| Background check via another field office | 5–6 |

Block-based delay removes the degenerate strategy of ending days early to fast-forward, and ties the rhythm of incoming information to player activity.

**The pager.** When a delayed result matures, the pager buzzes *wherever the player is*, mid-block. The headline arrives immediately — "QUANTICO: COD BLUNT TRAUMA, NOT FIRE" — and can redirect the afternoon on the spot; the full document is read at the Field Office (or summarized over a payphone call). Waiting becomes ambushes of information instead of a between-days tax.

### 4.3 The lab queue — throttling hard truth

Forensic facts are epistemically privileged (the only never-wrong tier), so they need a tighter throttle than generic time — otherwise optimal play converts every block into forensics and skips the testimony-doubting game that *is* the deduction.

The throttle is structural, not a currency: **the lab works at most 2 of your requests at a time** (`lab_queue_capacity`). A third submission queues behind the others, its delay clock not starting until a slot frees. The pending panel at the Field Office shows the queue and lets the player **reorder** requests that haven't started. The cost of a frivolous test is *delaying a serious one* — self-balancing, period-accurate (one backed-up state lab), and the decision it produces is sequencing, which is deduction-flavored: "the fiber test matters more than the ink analysis — it goes first."

### 4.4 Earned access — deduction as the key

Nothing is bought; access is unlocked by evidence:

- **Fact-gated actions.** Locked and secret actions (§5.3) open when their fact prerequisites are met. The locked farmhouse study isn't a warrant transaction — it's a locked SEARCH action whose prereqs are the facts that would have justified one. *(Warrants as a player-facing system: cut — police process management, not deduction. The fiction can still say "the warrant came through" in the action's result text.)*
- **Interview leverage.** Confronting a person with a specific fact — or an *active contradiction* involving them — unlocks dialogue that politeness never will. This makes the evidence graph directly playable in conversation.

### 4.5 Running out

When the clock expires, the reconstruction board opens automatically. There is **no hard fail before that point** — the failure state *is* a poor reconstruction. Design the critical path so ~60% of total facts are reachable with mediocre play; mastery is reaching the contradiction-resolving and red-herring-clearing facts.

---

## 5. Information & Evidence Model

The heart of the game and of the data architecture. (Full field-level detail lives in the companion **Case File Data Inventory** doc.)

### 5.1 Facts and the glossary

Every piece of information is a **Fact node**. Facts are **immutable and append-only**: a fact never changes after authoring. When the truth "changes," a *new* fact arrives and the old card stays on the board — the player holds both and sees what they mean together. This is what powers the recontextualization beat: the autopsy doesn't edit the witness statement, it *reframes* it, in the player's head.

```
Fact {
  id: "F-031"
  text: "Motel clerk says a green sedan was parked outside room 14 around 9 PM."
  tags: [ruth, green_sedan, motel, t_2100]     // glossary IDs
  reliability: TESTIMONY | DOCUMENT | FORENSIC
  conditions: [dark, ~40m distance, through office window]   // testimony only
}
```

- **The noun universe is derived, not authored as objects.** A flat **glossary** maps `tag_id → {kind, display_name}` (kinds: PERSON / PLACE / OBJECT / VEHICLE / ORG / TIME). The set of entities the player "knows" is exactly the tags of their discovered facts — so board anchors and reconstruction dropdowns only ever contain nouns genuinely encountered, decoys included (decoys are introduced by color facts). Times are matched tokens; the game never does time math.
- **Reliability tiers matter.** Testimony can be wrong or a lie; documents can be forged or misdated; forensics are hard facts. The case is authored so at least one early testimony fact is *honestly mistaken*, teaching doubt.
- **Observation conditions** (testimony only: lighting, distance, obstruction, witness state) are visible on the card. Their job is *fairness*: when a testimony fact is later invalidated, the seeds of doubt were inspectable from the start. They also give rational grounds for the `DOUBTED` tag (§5.4).

### 5.2 Contradictions

Reduced from a four-type link system to the one type with real mechanical consumers. **A contradiction relates claims, not truths**: the two facts cannot both be true, and the game never says which one is wrong.

```
Contradiction {
  id: "C-02"
  facts: [F-007, F-023]          // symmetric
  resolution_actions: [A-31]     // validator enforces ≥1
}
```

- Authored at build time, **dormant until both facts are discovered**; activation draws the red thread on the board — a flag, never an explanation.
- Consumers: the board thread; **interview leverage** (confrontation topics can require an active contradiction involving that character); the **validator law** that every contradiction has at least one discoverable resolving action.
- If the player had already `DOUBTED` one of the cards, activation surfaces their tag — the game acknowledging *their* prediction.

*(Deferred: CORROBORATES lost both consumers when warrants and the authored epilogue were cut; SUPERSEDES/REINTERPRETS were presentational. Revisit only if playtests show the keystone flip needs presentational support. See §12.)*

### 5.3 Unlock rules & action visibility

Every prerequisite in the format — action prereqs, location unlocks, diorama state switches — is a **flat list of fact IDs with AND semantics**. There is no expression DSL; the loader's entire conditional logic is "is this list a subset of discovered facts." Three conventions replace the lost operators:

- **OR** → the *shared knowledge-fact idiom*: every alternative route produces the **same fact**; gate on that one fact. This names the inference explicitly, which is better authoring, not just a workaround.
- **Time conditions** → the `available` block range (and optional `blocks_of_day`), never a prerequisite.
- **"Contradiction active"** → require both of the contradiction's facts (active ≡ both discovered). Interview-side leverage gating stays in ink via `contradictionActive()`.

Some facts will exist mainly as knowledge markers under this idiom; if they clutter the evidence board, an `internal: true` display flag is the sanctioned fix — one boolean, not a DSL.

Actions have a **tri-state visibility** derived from prereqs plus an authored `hidden` boolean:

- **Unlocked** — visible and selectable (prereqs met)
- **Locked** — visible, not selectable (prereqs unmet, `hidden: false`); an optional `locked_hint` label teases: *"there's something here I can't get at yet"* — itself a deduction motivator
- **Secret** — invisible (prereqs unmet, `hidden: true`); for actions whose existence is a spoiler

There is no conditional branching inside actions — an action always produces its flat fact list. Variation ("the search finds X only if you knew to look") is authored as multiple actions differing in prereqs and visibility. Prerequisites are **facts, not player-stated inferences** — inference stays free-form in the player's head and is verified only at the reconstruction.

### 5.4 The evidence board & player-initiated tagging

A corkboard UI: fact cards, entity anchors, threads drawn automatically by shared tags. The player can pin and annotate. **The board never confirms correctness** — a thinking tool, not an oracle.

Judgment tags, applicable to any fact or entity:

- **`CLEARED`** — "done with this lead." Cleared entities recede visually; actions whose only payoff touches them are dimmed (still available — the player can be wrong). The player's own tool against bleeding time on dead suspects; the game never clears anyone for them.
- **`DOUBTED`** — "I don't trust this claim." When a contradiction later activates against a doubted fact, the board surfaces that card prominently — memory-jogging that scales with engagement instead of hint-giving.
- **`KEY`** — a pin; keeps the card surfaced in the case-file sidebar.

Tags are pure player state — they never gate content or change truth. But the **derived post-game review reads them**: a correct early `CLEARED` is acknowledged; a wrong `CLEARED` on the killer is shown honestly. The player's judgment is part of the record they sign.

### 5.5 Runtime reaction pipeline

The case file is a **static authored universe**; the save is a diff: discovered fact IDs, active contradictions, player tags, board layout, clock, blocks-spent counter, pending/lab queues, ink state, reconstruction draft, location state indices. Nothing in the case file mutates at runtime.

Timed, unprompted beats (the day-3 discovery that happens regardless of player activity) live in a flat **`schedule` list** — `{at_block, delivers, pager}` — the sole survivor of the old trigger system; everything else triggers once did now lives at its effect site (unlocks in `unlock_rule`s, enables in prereqs, state changes in `states[].when`, discovery pagers in `pending_label`). Scheduled facts then drive unlocks like any other fact.

When a fact is discovered (by action, schedule, or a delayed result maturing):

```
append fact ID to discovered set
  → evaluate dormant contradictions (activate if both ends discovered)
  → evaluate unlock rules (action visibility, locations, dialogue topics)
  → evaluate diorama state switches (each state's own fact list)
  → UI reaction: card animates onto board, threads draw,
     doubted-tag payoffs surface, journal entry written,
     pager buzz if delivered mid-block
```

Everything the player perceives as "the game reacting" is this one pipeline — which is why the text/debug prototype (§9.3) is genuinely representative of the final game.

### 5.6 Fact economy targets

For a 3–5 hour case, target **35–40 facts**:

- **20–30 load-bearing:** ~10 reconstruction events × 2–3 supporting facts each
- **4–6 red-herring chain:** a suspect worth clearing, with a genuine resolution
- **5–8 color:** facts that make the world feel bigger than the solution (some double as decoy-noun introducers)

The metric that predicts engagement is **connection density**, not count: *every fact participates in at least two relationships* (shared tag, contradiction, or prerequisite). An isolated fact is trivia; deduction lives in the overlaps. Pacing check: ~4 discovered facts per day, with at least one contradiction activation or unlock per day so no day lands inert.

---

## 6. Locations & Action Vocabulary

A small, legible verb set reused across dioramas:

| Verb | Cost | Notes |
|---|---|---|
| **Inspect** | 0 blocks | A real action record with cost 0; flavor + minor facts; teaches the diorama |
| **Search** (drawer, vehicle, room) | 1 block | Often locked/secret behind fact prereqs (§5.3) |
| **Collect sample → Lab** | 1 block + lab queue slot + delay | Choose *which* test; queue capacity 2 forces sequencing (§4.3) |
| **Interview** | 1 block | Ink-driven topics (§9.1); leverage via facts or active contradictions; re-enabled when new facts tagged with that character appear |
| **Canvass** | 1 block | Produces testimony-tier facts |
| **Phone / Fax request** | 1 block, Field Office (up to two requests per block) | Cheap but slow; the 90s workhorse |

Each diorama supports 4–8 costed actions, of which the player can afford ~60% in one playthrough — guaranteeing meaningful choice and replay curiosity. *(The full block/action budget check is a validator rule fed by `tuning_targets`.)*

The verb vocabulary is intentionally minimal for the slice and is expected to grow with future cases (STAKEOUT is first in line back — see §12).

---

## 7. Reconstruction & Grading

### 7.1 The board

A horizontal **timeline in three bands: BEFORE / DURING / AFTER**. The player composes **event statements** from discovered nouns only — the derived entity set (§5.1) means dropdowns contain exactly what they've encountered, decoys included:

> `[Daniel Reyes]` `[drove]` `[the green sedan]` `to` `[the motel]` `at` `[~9 PM]` `because` `[he believed the victim kept the ledger there]`

- Authored as **slot templates**: each key event has a fixed structure (actor / verb / object / place / time / motive), motive only where it matters. Verb and motive slots draw from authored phrase pools.
- ~**8–12 key events**, decoy pools per slot (default: "all discovered nouns of the slot's kind") to defeat guessing.
- **Validation only on final submission** (fallback if playtests show all-at-once is too brutal: silent confirmation in batches of three fully-correct events, Obra-Dinn style).
- Per-slot `supports` fact IDs (authored) justify each answer — consumed by the review, not shown during play.
- The submission moment is ceremonial: a signed memo to the SAC. One click, no takebacks.

### 7.2 Grading

Weighted partial credit:

| Assertion type | Weight |
|---|---|
| Who committed the crime | 25% |
| How (method/during events) | 25% |
| Why (motive) | 20% |
| Before/after events (cover-up, lead-up) | 20% |
| Accessory/secondary characters correctly placed | 10% |

Wrong slot answers can carry authored partial weights (a near-miss motive scores half). Score maps to an outcome tier:

- **90%+ — Case Closed.** Conviction. Commendation.
- **70–89% — Plea Bargain.** Lesser charges; your gaps let the defense bargain.
- **50–69% — Mistrial.** Right person, case didn't hold.
- **<50% — Wrong Man / Unsolved.** Honest consequences.

### 7.3 Outcome & derived review

Authored content is minimal: **one short ink knot per outcome tier.** Everything else is *generated from data*:

- Player timeline vs. true timeline, side by side
- Per-slot `supports` facts marked **had / missed / ignored** — including assertions made with no supporting fact at all (the overreach lesson)
- Player-tag callouts: correct `CLEARED`s acknowledged, wrong ones shown
- Replay hooks: undiscovered facts as face-down cards labeled only by the action that would have produced them

---

## 8. The Vertical Slice Case (sketch)

**"The Ashford County Case," autumn 1995.** A motel owner is found dead in a burned-out unit in rural Virginia; local PD ruled it an accidental fire, but the insurance company's arson finding triggered an FBI fraud angle — which conceals a murder tied to an interstate stolen-goods ring (the federal hook).

**Dioramas (6):**
1. **Field Office** (hub — phone/fax, evidence board, pending panel, results)
2. **The burned motel unit** (crime scene; bulldozed by day 7 if neglected)
3. **Motel office & clerk's counter**
4. **Victim's farmhouse** (the locked study is a fact-gated SEARCH action)
5. **Roadside diner** (witnesses, evening `blocks_of_day`)
6. **Pawn shop two counties over** (unlocked by evidence; distance expressed narratively — no travel cost in the slice)

**Designed tensions:** one honest-but-wrong witness statement (with inspectable observation conditions), one forged document, one red-herring suspect with a genuine alibi resolution, one recontextualizing lab result around day 5–6 — exactly one per case, as authoring law.

The case is fully authored on paper (every fact, action, prereq, contradiction, and the solution timeline) **before any engine work** — see §10. A simulated playthrough exists as a companion doc.

---

## 9. Technical Direction

### 9.1 The key architectural decisions

**The case is data; the engine is a player for that data.** Three-file separation:

1. **Case logic file (JSON)** — meta, glossary, facts, contradictions, locations, actions, interviews, schedule, reconstruction, outcome tiers. Field-level schema lives in the **Case File Data Inventory v2.0**, which is the format authority. JSON is the authoring source of truth *permanently*; a binary compile target ships later, produced by the build step, never hand-edited.
2. **Ink files** — all long-form prose: briefing, interviews, document texts, outcome memos. Engine↔ink contract kept tiny and stable: `hasFact(id)`, `contradictionActive(id)`, `discoverFact(id)`, `day()`, `spendBlock()`. Each interview declares a **fact manifest** in the logic file; the validator cross-checks it against the ink source's `discoverFact` calls so reachability analysis survives scripting. (Inkpot for UE; ink runtimes exist for custom engines.)
3. **Staging file (per engine)** — one scene description per location: each state carries its full prop list (states fully define their scene — no diffs) and the 3D position of every action point, plus camera pivot and zoom limits. Portraits, audio, and block-of-day labels join it only when needed. Format sketch: Data Inventory §15.

The runtime is: a diorama renderer (orbit camera + raycast picking), a rules evaluator (fact-list subset checks, clock, lab/pending queues, schedule), an ink runtime, and 2D UI screens. No character controller, no physics, no AI navigation, no scripting.

**The validator is the second program built** (after the loader), with its targets in its own config — identical for every case, never stored in case files. It enforces the authoring laws: no dangling IDs; every fact producible; fact connection-density ≥2; every contradiction resolvable; critical path reachable within the block budget; bounded-availability critical actions have alternates; `hidden_reveal` never the sole route to critical facts; manifest↔ink consistency; glossary coverage of every reconstruction answer and decoy; `pending_label` wherever `delay > 0`; economy targets.

### 9.2 Engine recommendation

| Option | Fit |
|---|---|
| **Unreal** | Works; gorgeous dioramas; Inkpot covers ink. But UMG-heavy UI is UE's slow lane and this game is ~50% UI. Heavyweight for scope. |
| **Godot or Unity** | Sweet spot: fast UI iteration, easy data-driven design, trivial orbit camera; ink runtimes available. |
| **Own small engine** | Genuinely viable — the runtime list above is small and stable, and the ink C runtime integrates cleanly. Choose it only if building the engine *is part of the fun*; roughly doubles time-to-playable. |

**Recommendation:** prototype in an existing engine, keep 100% of case logic in data and a thin engine-agnostic rules layer. If the design proves out and you still want your own engine, the case file, ink files, and rules code port with you.

### 9.3 Prototype-before-dioramas rule

Build a **text/debug version first**: locations as lists, actions as buttons, facts as log lines, ink playing in a plain text panel — the full clock/queue/unlock/contradiction/reconstruction loop end-to-end. Because of §5.5, this prototype is behaviorally identical to the shipped game. If the case is fun as a spreadsheet, it will be great as dioramas. If it isn't, no camera will save it.

---

## 10. Milestones

1. **Paper case (1–2 weeks).** Author full Ashford: solution timeline, every fact, action, prereq, contradiction, grading sheet. Tabletop-playtest it with a friend as the "computer."
2. **Loader + validator (1 week).** Case file parses; all authoring laws checked. The paper case becomes the first test input.
3. **Systems prototype (2–4 weeks).** Debug-UI full loop, ink integrated in a text panel. Tune the economy against `tuning_targets`.
4. **Reconstruction & review (1–2 weeks).** The board, decoys, grading, derived review.
5. **First diorama + camera (2 weeks).** One location art-passed; staging file format proven; action-point readability rules established.
6. **Vertical slice (4–8 weeks).** All 6 dioramas, period UI skin, sound pass.

---

## 11. Open Questions (post-slice)

None of these block implementation:

- Replay shuffling (times, decoys) to keep the reconstruction honest on a second run?
- Difficulty option: show/hide the contradiction red thread?
- Tone: procedural realism (Mindhunter) vs. slight pulp (X-Files-adjacent, minus the paranormal)?

*(Resolved: dialogue is topic-based via ink; warrants/PD favor/lab credits cut; conditional produces replaced by tri-state visibility; whole-block costs; STAKEOUT and manual board linking deferred.)*

---

## 12. Deferred (cut, kept on record)

- Link types `CORROBORATES`, `SUPERSEDES`, `REINTERPRETS` (presentational/consumer-less after other cuts)
- Warrants & judge rules; wrong-but-plausible warrant dead ends
- PD favor resource; lab credits (replaced by lab queue capacity)
- Authored per-gap/per-error epilogue lines (derived review replaces them)
- Conditional action outcomes
- Mid-case inference puzzles
- STAKEOUT verb (the only verb that can *miss* — great texture, needs careful authoring; first candidate when the verb vocabulary grows with future cases)
- Manual player-drawn board threads (auto-threads + annotations + judgment tags cover the slice)
- Fractional block costs
- Fact-based expiry / `closes_on` (lost in the v2.0 flattening; see §13 — likely the first item to come back)

---

## 13. Known Expressiveness Limits (monotonicity stress-test)

An honest stress-test, thinking through cases one would actually want to author in this format: the **deduction side is fine** — layered truth, contradictions, recontextualization, red herrings, economy pressure are all fully expressible. The gaps are all on the **consequence side**, and they share one root cause: **the format is monotonic**. The discovered-fact set only grows, and every gate only opens. Concretely:

1. **Nothing can close because of what the player did.** This is the big one. Time can close things (`available` ranges), but events can't — fact-based expiry was lost in the flattening (the old format's "voluntary interview expires once F-100 makes the arrest" is now inexpressible). That means no "you spooked him and he lawyered up," no "searching before the lab results contaminated the scene," no arrest that forecloses other approaches. Compelling cases get a lot of drama from irreversibility, and right now the only irreversible thing in the game is the clock. The fix, if playtesting demands it, is surgical and DSL-free: one optional field, `closes_on: [fact list]` — same subset check as everything else, just inverted in effect. Held in reserve rather than added now, but it's the most likely deferred item to come back first.

2. **The world can only react instantly.** State changes and unlocks fire the moment their gating facts are discovered — there's no "two days after you rattled Boyd, he runs." Interestingly, the format can *almost* express delayed consequences by composition: a hidden side-effect fact on an action, delivered via `delay`, gating a state change when it matures. But that collides with two rules — delayed facts page the player, and marker facts appear on the board. If the `internal: true` flag (§5.3) is adopted, **exempt internal facts from the pager and the board**, and this pattern falls out for free. That's the actual recommendation: don't add a feature — document the pattern and add the two exemptions to the internal flag's definition.

3. **One truth, one timeline.** The reconstruction grades against a single authored solution; alternates handle near-misses, not rival theories. A case whose point is choosing between two coherent readings of the evidence can't be built. Accepted — it's the genre standard (Obra Dinn and Golden Idol are single-truth too), and multi-truth grading is a swamp — but it does rule out one specific flavor of ambiguous, literary case.

4. **No combinatorial presentation.** "Show any evidence to any witness" — the Ace Attorney verb — is inexpressible except by hand-authoring each meaningful pairing as an ink topic. This is less a format limit than an authored-content reality (the combinatorial verb is mostly noise even in games that have it), but be aware the format quietly commits to **curated confrontations**, which puts weight on authoring the right ones.

5. **Everything conversational lives or dies in ink.** The five-function contract deliberately limits ink's influence on the world to producing facts — which keeps the validator honest, but means any interview consequence beyond information (a witness who stops cooperating, a suspect who flees) must route through the fact system, and therefore inherits limits 1 and 2. Once `closes_on` and internal facts exist, this resolves itself; until then, interviews can reveal but never change anything.

The pattern across all five: the format kept **full expressiveness for the player learning about the world**, and traded away most of **the world responding to the player**. For a vertical slice whose question is "is the deduction loop fun," that's exactly the right trade — Golden Idol is essentially fully monotonic and it's brilliant. But a full game of these cases will eventually want consequence for texture, and when it does, the re-adds are small and already shaped: `closes_on`, `internal: true` with pager/board exemptions, and nothing else.
