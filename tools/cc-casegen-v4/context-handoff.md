# Context Handoff

This file carries forward context that isn't in `design-notes.md` or `event-emissions.md` — the working style, the decisions already argued through, and the natural next steps. Read this after the other two files.

---

## 1. File inventory

- **`design-notes.md`** — the locked-in design across all three layers (concept, entities, skeleton, constraints, evidence framework). **Authoritative reference.**
- **`event-emissions.md`** — base emissions for 12 event types (all except `commission`, whose worked example lives in `design-notes.md` §6). Passed two writer/validator review rounds plus a polish pass.
- **`context-handoff.md`** — this file.

No code exists yet. All artifacts so far are design.

---

## 2. Working style — read before responding

The owner of this project values **honest feedback over validation**. When presented with an idea, the expected response is:

- Challenge the idea if it has real weaknesses. Don't soften criticism into "interesting point!"
- Offer a specific counter-recommendation when rejecting an approach.
- Admit when the owner's pushback is correct and your earlier suggestion was wrong. This happened multiple times during the original design discussion and was productive each time.
- Verify claims against the existing context before accepting a new direction. Phrases like "please verify with context" mean "don't take my word for it, check whether this holds."

Discipline the owner will apply back:

- **"Does this earn its place?"** — applied to every attribute, field, rule, and entity. If it doesn't drive a generation decision or isn't player-visible, cut it.
- **"Is this scenario-flavored?"** — caught when rules drift toward specific scenarios ("took the trash on an odd day") instead of general structure.
- **"Compositional, not combinatorial"** — rules should add, not multiply.

**Explicit rule: do not reference the existence or content of prior versions** (v2, v3, or earlier attempts). They failed; the owner has chosen a clean slate. Invoking them pollutes the current design frame.

---

## 3. Decisions already argued through — do NOT reopen unprompted

Each of these was debated and closed. Reopening without new information will waste the owner's time. They're listed with the reason they were rejected, so you know what argument is required to reconsider.

### Rejected approaches

| Rejected | Reason | What was chosen instead |
|---|---|---|
| Pure forward simulation | Runaway scale, shapeless output, no commitment to being a mystery | Retrograde + skeleton |
| 100% hand-authored cases | No scale, no replayability | Retrograde with slot-filling |
| Silent misleading evidence (flavor b) | Requires modeling player inference to deceive fairly; research-grade problem | Flavor (a) only: irrelevant-but-real events as red herrings |
| In-session evidence deterioration | Complexity without game value | Deterioration baked into generated case as snapshot |
| Meta-progression (unlocks, skills, levels) | Deduction-game fantasies don't match "my detective leveled up" | Player skill is the only progression |

### Rejected data-model choices

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
| N-person Relationships | Fundamentally pairwise; groups decompose to pairs + shared Place | Pairwise only; `Group` entity deferred until a case archetype forces it |
| Separate `reliability` attribute on Person | Subsumable by personality axes | Reliability emerges from (axis_1, axis_2) |
| `symmetric` flag on Relationship | Adds a redundant field | Direction semantics carried by the type enum |

### Rejected emission/rule choices

| Rejected | Reason |
|---|---|
| `pattern_deviation_signal` as base emission on `object_disposal` | Scenario-flavored; belongs in personality pass |
| `what_instrument` on `communication_text` device trace | Device isn't `role=instrument` on text events |
| Baking `delivery` (volunteered/pressured/hostile) into base emission rules | Delivery is personality-pass (Pass 4) territory |
| Folding participant testimony into observer testimony | Different evidentiary pathways; always split |
| Multiple archetypes in v1 | Each archetype needs its own skeleton; scope creep |

---

## 4. Ambiguities deliberately parked

These are unresolved and intentionally so — reopen them only when the current layer of work demands it.

- **Concrete threshold values** — "target accuracy" for solvability check, "how suspicious = red herring plausible." These are tuning knobs, not structural.
- **Difficulty levers** — deferred until a simple generator exists and we can test tweaks empirically.
- **Evidence presentation / UI** — how each evidence type surfaces in-game and what action-point cost examining it has.
- **Content text generation** — how record `content_summary` strings get filled; how testimony prose gets authored. Not a structural problem yet.
- **Save/serialization format** — implied JSON given the typed-string-ID scheme, but not committed.

---

## 5. Natural next steps — priority order

With base emissions done, the remaining evidence-pipeline work is:

### Tier 1 — complete the evidence pipeline
1. **Place modifier table (Pass 2)** — how `security`, `access_level`, `operating_hours`, place `type` add or modify emissions. Cross-cuts all 13 event types.
2. **Object modifier table (Pass 3)** — how `electronic`, `documentary`, `portable/fixed`, object `type` add or modify emissions.
3. **Personality modifier table (Pass 4)** — how axis_1 (impulsive↔calculating) and axis_2 (confrontational↔secretive) on the actor modify emissions.
4. **Downstream event adjustment rules (Pass 5)** — specifically, which events remove or reduce earlier emissions. `object_disposal → remove instrument_traces` is the canonical case; enumerate others.
5. **Facet coverage check algorithm (Pass 6)** — concrete algorithm for "does the evidence cover enough facets to make the case solvable." Will need a threshold.

Tier 1 is a natural unit of work and is what I'd tackle first after this handoff.

### Tier 2 — the generator algorithm itself
6. **Retrograde pipeline** — concrete algorithm for slot-filling in the order already defined (§4 of design-notes). Needs stop conditions, regeneration on failure, and a seed strategy.
7. **Forward sim for red herrings** — lightweight simulation that respects the already-defined `Thread` constraints.
8. **Constraint enforcement mechanics** — how compatibility constraints (§5) are actually checked. Probably a constraint-solver-lite approach given the small slot count.

### Tier 3 — validation and tuning
9. **Difficulty knobs** — only after a basic generator produces end-to-end cases.
10. **Test cases** — curated examples by hand to validate the generator's output shape.

### Tier 4 — everything else
- Content generation (names, text prose)
- UI / examination mechanics
- Scoring implementation
- Archetype extension beyond murder

---

## 6. Key conceptual anchors (things a new chat might get wrong)

Flagging these because they're subtle enough to be worth re-stating:

- **Motive is inferred, not observed.** `topic` is the observable facet; motive is what the player argues for by connecting topics across the timeline. This matches how real detective narratives work and is a core design commitment.

- **`primary_thread` is partially a wrapper.** For a simple single-killer case, its attributes duplicate case-level info. It's kept because **events reference their thread (direction (a))**, so crime events need something to point at. Uniformity > minimalism here.

- **Object ≠ Evidence.** `Object` is world state (the knife exists). `Evidence` is derived from events at generation time (fingerprints on the knife are evidence). Don't conflate.

- **Roles are relational.** A Person isn't a "witness" in their attributes — they're a witness *of* a specific event. Computed from `Event.participants` with `role=observer`.

- **Commission doesn't reveal the killer by itself.** If it did, the case would be trivial. Commission reveals the victim and that a killing happened; surrounding events reveal who.

- **Red herrings are real events, not fabrications.** They genuinely happen in the world; they just aren't the crime. The confusion comes from their shape resembling motive/opportunity for the crime. Flavor (a) only.

- **The skeleton is visible to repeat players over time.** After 30–50 cases, the owner should expect players to pattern-match the shape. Extending the horizon requires additional archetypes. This was accepted as a known limit.

- **Attribute discipline.** Entities target 8–12 attributes. Every attribute must drive generation or be player-visible. If the owner proposes a new attribute, apply this test before accepting.

---

## 7. Implementation context

- The project lives at `C:\Users\alexb\Documents\QG-ChainLinks\tools\cc-casegen-v4`
- Language: **C-style C++**. The owner has explicitly mentioned disliking heavy string handling.
- **Memory arena pattern** for IDs: the typed string IDs (`person_001`, etc.) can be interned into an arena and passed around as fixed-size handles at runtime. Confirmed as the owner's preferred approach.
- Serialization implied but not committed: typed-string IDs serialize cleanly to JSON, useful for saving/loading cases and for test cases.
- No code has been written yet. All current artifacts are Markdown design documents.

---

## 8. How work is expected to proceed

The owner runs long, careful design conversations before writing code. The pattern is:

1. Present a topic and initial direction.
2. Iterate on the design until every decision has a clear justification.
3. Lock the decisions into a reference document.
4. Move to the next topic.

Keep responses focused. Long essays aren't valued unless the topic warrants it. Lists and concrete examples beat prose. Honest pushback beats agreement. Match the existing documents' tone and density when writing anything new.

When in doubt about an approach, **describe the tradeoff explicitly** and offer a recommendation. The owner prefers "I'd recommend X because Y, though Z is viable if A" over either absolute certainty or noncommittal both-sides-ism.

---

## 9. Quick-start prompt for resuming

Paste something like this at the top of the new chat to orient a fresh context:

> I'm working on a procedural mystery case generator for a single-player FBI investigator game. The full design is documented at `C:\Users\alexb\Documents\QG-ChainLinks\tools\cc-casegen-v4\`. Please read:
>
> 1. `design-notes.md` — the locked-in design across concept, entities, skeleton, and evidence framework
> 2. `event-emissions.md` — base emission rules for 12 event types
> 3. `context-handoff.md` — working style, decisions already made, and next steps
>
> After you've read those, I'd like to tackle [X] — which is item N in the "Natural next steps" list in the handoff.
