# COLD FILE — Runtime Statistics & Case Subsystem Events
### Design reference for what the case subsystem tracks and broadcasts (companion to the design doc & data inventory)

> Scope: gameplay-time bookkeeping (stats) and the event surface other systems / the UI react to. Not part of the case file format — this is engine/runtime state (see Data Inventory §12 "Save state"). Nothing here is authored per-case; it's uniform across all cases.

**Guiding constraint — pillar 4 ("the game never tells you if you're right").** Stats are split by consumer: the live HUD must never leak *correctness*; the post-game review is exactly where correctness is revealed. Any stat that would tell the player whether a lead is right/wrong belongs in the post-game bucket, never the HUD.

Most stats are accumulators fed by the events in §2 — wire the events first and the stats fall out.

---

## 1. Statistics

### 1.A Live / player-facing (HUD + case-file sidebar) — must NOT leak correctness

| Stat | Notes | ★ |
|---|---|---|
| Clock: day, block-of-day, blocks remaining to deadline | The core tension. `used_blocks` vs `deadline_days * blocks_per_day` | ★ |
| Pending panel: requests in flight, lab slots used vs `lab_queue_capacity`, queue depth, ETA in blocks | This *is* the planning layer (design §4.2–4.3) | ★ |
| Facts discovered (count), split by reliability tier (TESTIMONY / DOCUMENT / FORENSIC) | From `known_facts` | ★ |
| Entities known | Size of the derived noun set (what will appear on reconstruction dropdowns) | |
| Active contradictions (red threads unresolved) | Spotting tension ≠ knowing the answer, so safe to show | ★ |
| Leads available | Unlocked-but-untaken actions; plus locked teases *seen* (the `locked_hint` motivators) | |
| Coverage: locations unlocked / visited; `KEY`-pinned cards | | |
| Blocks spent total (and today) | | |

### 1.B Post-game derived review — correctness now allowed (this is the reveal, design §7.3)

| Stat | Notes | ★ |
|---|---|---|
| Final score + per-assertion breakdown (who / how / why / before_after / accessory) + outcome tier | | ★ |
| Support audit per filled slot: `supports` fact **had / missed / ignored** | Plus **overreach**: slots answered with zero supporting fact discovered | ★ |
| Judgment audit: correct `CLEARED`s, wrong `CLEARED` on the killer, `DOUBTED` facts that were later contradicted (prediction hits) | The player's judgment is part of the record they sign | ★ |
| Contradictions: surfaced / resolved / total, incl. tensions never noticed | | |
| Red-herring: did you clear the innocent suspect, and on which block | | |
| Recontextualization: did you end up holding *both* cards of the keystone flip | | |
| Missed content: undiscovered facts as face-down cards by producing action; unvisited locations; diorama states never seen | | |
| Efficiency: blocks-per-fact; % of costed content engaged (the ~60% curve) | | |

### 1.C Dev telemetry / tuning — never shown to the player

| Stat | Notes | ★ |
|---|---|---|
| Facts/day curve vs the ~4/day target; inert days (no unlock AND no contradiction activation) | Target: zero inert days | ★ |
| Affordability: fraction of costed actions affordable this run (~60% target) | | |
| Critical-path timing: block at which each keystone fact landed; path completed before deadline? | | |
| Time-to-resolve each contradiction (blocks between activation and resolving action) | | |
| Lab utilization: slot occupancy %, average queue wait, reorder frequency | | |
| Wasted blocks (actions producing no new fact); stall stretches (blocks with zero discovery) | | |

---

## 2. Case subsystem events

The spine is **`OnFactDiscovered`** — per the runtime pipeline (design §5.5), almost everything else cascades from a fact entering the set. In UE these are `DECLARE_DYNAMIC_MULTICAST_DELEGATE`s on `UCaseSubsystem` (`BlueprintAssignable`) so UMG widgets and Blueprints bind directly.

| Event | Fires when | Reactors |
|---|---|---|
| ★ `OnFactDiscovered(fact_id, source, mid_block)` | any fact enters the set (action / schedule / matured result) | board card animation, journal entry, glossary/dropdown refresh, pager if `mid_block` |
| `OnEntityKnown(tag_id)` | a fact introduces a *new* glossary noun | board anchor appears; reconstruction dropdown grows |
| ★ `OnContradictionActivated(c_id)` | both of a contradiction's facts are known | draw red thread; surface any `DOUBTED` tag on those cards |
| `OnContradictionResolved(c_id)` | a `resolution_action` fires | thread state updates; interview leverage may open |
| ★ `OnDoubtedFactContradicted(fact_id, c_id)` | activation hits a card the player pre-`DOUBTED` | the "game acknowledges your prediction" beat — surface prominently |
| `OnLocationUnlocked(loc_id)` | `unlock_rule` becomes satisfied | map pin lights up |
| `OnActionVisibilityChanged(action_id, state)` | secret → locked → unlocked transitions | diorama hotspot appears/enables; "new lead" ping |
| `OnLocationStateChanged(loc_id, state_id)` | a `states[].when` becomes satisfied | diorama swaps assets — telegraphs time passing |
| ★ `OnResultsMatured(action_id, fact_ids)` | a delayed request's clock hits 0 | **pager buzz** with `pending_label`; pending panel update |
| `OnLabQueueChanged` | submit / slot frees / reorder | pending panel refresh |
| `OnTimeAdvance(new_block)` | each spent block, after that block's maturations & schedule deliveries have fired (the clock only ever advances +1, so no from/to pair) | HUD clock; day-boundary detection (`new_block % blocks_per_day == 0`) |
| `OnDayChanged(day)` | crossing a day boundary | day banner / flavor |
| `OnDeadlineImminent` / `OnDeadlineReached` | last block / clock expires | warning; auto-open reconstruction |
| `OnActionCommitted(action_id)` / `OnActionRejected(action_id, reason)` | player takes / is denied an action | animations; feedback (see reason enum below) |
| `OnInterviewLeverageAvailable(character, c_id)` / `OnReInterviewEnabled(character)` | active contradiction involving the character, or a new character-tagged fact | highlight a confrontation topic; "worth talking to X again" |
| `OnFactTagged(fact_id, tag)` | player sets `CLEARED` / `DOUBTED` / `KEY` | recede/surface card; feed the review audit |
| `OnReconstructionOpened` / `OnSlotChanged` / `OnOutcomeComputed(tier, score)` | board opens / draft edits / submission | board UI; draft autosave; outcome memo (ink knot) + derived review |

### 2.1 Rejection reasons (enum)

`ECommitActionResult`, returned by `commit_action` / `can_commit` so the debug console and the real UI report failures consistently (an `OnActionRejected` event can wrap it later):

`UnknownAction` (bad id) · `NotAtLocation` · `PrerequisitesNotMet` (locked / secret) · `OutsideAvailabilityWindow` (`available` range) · `WrongBlockOfDay` (`blocks_of_day`) · `WrongLocationState` (`location_states`) · `NotEnoughBlocks` (would exceed the deadline budget) · `LabQueueFull` (queue at capacity — refuse-at-capacity, pending a queue-behind decision) · `AlreadyCompleted` (non-repeatable action already taken).

### 2.2 Correctness-blind rule

Keep the **live** events correctness-blind. `OnContradictionActivated` says "these two claims clash," never which is false. `OnOutcomeComputed` is the **only** event that carries a verdict, and it fires only after the ceremonial submit.

### 2.3 Implemented so far

- `FOnFactDiscovered(FName fact_id, int32 when_block)` → `on_fact_discovered` — first discovery only (re-discovery deduped); fires for starting facts, action `produces`, matured lab requests, and schedule deliveries, stamped with the exact block it landed on.
- `FOnNewLabRequest(FLabRequest)` → `on_new_lab_request` — a COLLECT action was queued.
- `FOnLabRequestComplete(FLabRequest)` → `on_lab_request_complete` — a request's delay elapsed; its `produces` discover in the same tick. (Covers the table's `OnResultsMatured`.)
- `FOnScheduleComplete(FText pager_text)` → `on_schedule_complete` — a `schedule` entry fired at its `at_block`.
- `FOnTimeAdvance(int32 new_time)` → `on_time_advance` — once per spent block, after that block's maturations and deliveries. (Covers the table's block/clock event.)
- `commit_action` returns `ECommitActionResult` (§2.1) rather than firing an event.

Next spine pieces: contradiction activation (`OnContradictionActivated` + the `DOUBTED` payoff) and the unlock reactions (`OnLocationUnlocked`, `OnActionVisibilityChanged`, `OnLocationStateChanged`).
