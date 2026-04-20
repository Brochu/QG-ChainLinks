# Playable Case 01 — Holloway & Reed

**Case oracle file.** This contains the full ground truth, all evidence, all surface-paths, and the constraint analysis. The player is investigating; I (Claude) reveal information from this file as the player asks for it.

---

## 1. Setting

**Holloway & Reed Architects** — a 12-person boutique architecture firm. 4th-floor downtown loft office in a 5-story converted warehouse building. Three weeks from a major proposal deadline on a $3M city library renovation contract.

**Building layout (4th floor):**
- Main entrance (badge-access from elevator lobby)
- Open-plan workspace (desks for associates and juniors)
- Two partners' offices along the north wall (Ines's and Theo's; interior keycode required)
- Design studio (south side; supply cabinets with X-Acto knives, foam cutters, model materials; large sink; sharps disposal bin)
- Conference room
- Kitchen
- Bathrooms

**Adjacent:**
- 4th floor across the hall: small advertising agency (one occupant working late that night — Maya Chen)
- 3rd floor: empty (vacant office space)
- Lobby: security camera covers main entrance + elevator bank, NOT the stairwell to the architects' floor
- Ground floor: coffee shop (closes 21:00)

---

## 2. Cast

### Victim
- **Ines Holloway**, 58, founding partner. Demanding, perfectionist. The firm's design soul. Killed Tuesday night.

### Killer (truth)
- **Sloane Mercier**, 42, senior associate. 12 years at the firm. Promised partnership "soon" for the last four years. Personality: `mild_calculating`, `mild_secretive`. Recently learned Ines was negotiating to merge the firm with another practice — which would dissolve her partnership track entirely.

### Promoted red herring
- **Theo Reed**, 60, co-founding partner. Wants to retire and sell his stake but can't — Ines has been quietly stalling the firm valuation. Decade-long power struggle. Personality: `mild_confrontational`, `mild_impulsive`.

### Other staff present that night
- **Petra Vance**, 35, office manager. Discovered the body Wednesday morning. In office until 22:00.
- **Wyatt Knox**, 27, junior architect. Working on physical models that night. In office until 22:15.
- **Maya Chen**, 31, account director at the ad agency across the hall. Working late on her own pitch.

### Off-stage but referenced
- **Marcus Bell**, the architect Ines was negotiating the merger with. Out of state.
- **David Aronson**, Theo's attorney. Lives 4 blocks from the office; Theo went to his home Tuesday night.
- **Mitra**, neighbor of David Aronson; walks her dog around 21:30 nightly.

---

## 3. Truth chain (Sloane) — 14 events spanning ~3 weeks

| # | When | Type | Event |
|---|---|---|---|
| 1 | W-3 Mon | meeting | Sloane delivers final draft of major project to Ines, expects partnership news |
| 2 | W-2 Tue | meeting | Ines tells Sloane "let's revisit partnership after the library bid lands" — third such delay |
| 3 | W-1 Mon | communication_call (Sloane as observer) | Sloane catches fragments of Ines on phone with "Marcus" discussing "merging practices" — overheard from hallway |
| 4 | W-1 Wed | object_acquisition | Sloane finds Ines's emails on the shared server (permissions accidentally too broad) — reads merger correspondence |
| 5 | W-1 Fri | argument | Sloane confronts Ines privately in Ines's office, no observers; Ines dismisses concerns |
| 6 | W-1 Sat | communication_written | Sloane drafts and prints a resignation letter at home, places in her desk drawer (does not send) |
| 7 | T-3d Sat | meeting | Sloane meets a competitor firm (Whitfield Associates) for coffee about leaving — secret, off-site |
| 8 | T-1d Mon | meeting | Library bid all-hands meeting; visible tension between Sloane and Ines; Wyatt and Petra notice |
| 9 | T0 Tue 19:00 | meeting | Late dinner brought in for the team; everyone present in conference room |
| 10 | T0 Tue 21:00 | movement | Most staff leaves; remaining: Ines (her office), Sloane (her desk), Wyatt (model station), Petra (kitchen), Theo (just leaving — see red herring chain) |
| 11 | T0 Tue 21:15 | movement | Sloane goes to design studio under pretext of grabbing a foam cutter |
| 12 | T0 Tue 21:20 | object_acquisition | Sloane takes an X-Acto knife from the supply cabinet (interior keycode log: "studio supply" 21:20 Sloane) |
| 13 | T0 Tue 21:35 | movement | Sloane enters Ines's office (interior keycode log: "Ines's office" 21:35 Sloane) |
| 14 | T0 Tue ~21:45 | **commission** | Sloane stabs Ines once in the throat with X-Acto knife. Ines is at her desk; she half-rises, defensive posture, but the act is fast and clean. |
| 15 | T0 Tue 21:55 | object_disposal | Sloane wipes knife in the design studio sink, drops it into the shared sharps disposal bin among many other used blades |
| 16 | T0 Tue 22:10 | movement | Sloane returns to her desk and continues working on bid materials until 22:50 |
| 17 | T+1 Wed 07:30 | **discovery** | Petra arrives, notices Ines's office door ajar, finds body, calls 911 |

**Sloane's commission shape:** 21:45 ±5 min. Murder weapon: X-Acto knife (object), `physical_traits = leaves_distinctive_mark`, `portability = portable`, `origin_location = design studio`.

**ToD range** (forensic estimate): **21:30–22:30**. This is what the player is told — not 21:45.

---

## 4. Red herring chain (Theo) — 9 events

**Fake commission shape (scaffolding only):** "Theo kills Ines at 21:45 over the firm valuation block." Used to retrograde-fill Theo's chain.

| # | When | Type | Event |
|---|---|---|---|
| 1 | Years prior | relationship baseline | Holloway-Reed partnership formed; tension grows over the decade |
| 2 | W-4 | communication_call | Theo's accountant tells him firm valuation has stalled — he can't sell his stake |
| 3 | W-3 Wed | argument | Theo confronts Ines about valuation in conference room; Wyatt and Petra hear shouting |
| 4 | W-2 Thu | communication_text | Theo emails attorney David Aronson about "options to force buyout or dissolution" |
| 5 | W-1 Mon | argument | Theo publicly criticizes Ines's library bid concept in partners' meeting; Sloane and Wyatt present |
| 6 | T-3d Sat | meeting | Theo meets Aronson at his office to discuss litigation — billable record exists |
| 7 | T0 Tue 21:00 | movement | Theo leaves office "for some air" (badge log: out 21:01); does not return until 22:35 (badge log: in 22:36) |
| 8 | T0 Tue 21:10–22:25 | meeting | **ALTERNATE TRUTH:** Theo at David Aronson's home reviewing draft litigation paperwork. This is the engineered weak link. |
| 9 | T+1 | meeting | Theo learns of Ines's death, performs grief; benefits structurally — Ines's stake reverts to remaining partners pro-rata, giving Theo controlling share |

**Theo's break point:** axis = `timing`. Mechanism = the alibi at Aronson's home.

**Why it's non-binary:**
- Aronson is Theo's friend AND has financial interest (retainer for upcoming litigation) — could plausibly fudge times
- Aronson's confirmation isn't precise to the minute ("around 9:30 to 10:25 maybe")
- The neighbor saw "older man arrive around 21:30," giving Theo ~30 min from leaving the office
- ToD range is 21:30–22:30 — if Theo lingered at the office until 21:30 before walking, then arrived at attorney's at 21:35–21:40, the alibi window covers most but not all of the ToD range
- Theo refuses to volunteer the alibi initially because admitting he was prepping a lawsuit against Ines the day she died is bad optics

**Three independent paths to surface the alibi:**
1. Direct interview pressure on Theo (yields evasive partial admission)
2. Aronson's digital calendar (can be obtained via subpoena or charm — "TR mtg 21:00 home")
3. Mitra (neighbor) testimony — saw older man arrive ~21:30

---

## 5. Modifier noise (Tier 2)

These are real, unrelated events whose evidence complicates interpretation of crime evidence.

1. **Wyatt's prints on the X-Acto.** Wyatt was prepping models all afternoon; he used multiple X-Acto knives from the same cabinet. The blade Sloane took already had Wyatt's prints from earlier use. → murder weapon shows Wyatt's prints + Sloane's partial wipe.

2. **The cleaner's morning rounds.** A cleaning service mops the design studio every day at 18:00. Explains why the studio sink was already recently cleaned before Sloane wiped the knife — no anomaly to detect.

3. **Petra in the kitchen 21:00–21:30.** She was making tea, on her phone. She heard footsteps in the hallway around 21:30 but didn't look. Her testimony provides bounded-perspective audio confirmation of someone moving but no identity.

4. **Wyatt at his model station.** From his desk, he had partial line-of-sight to the hallway leading to Ines's office, but his back was mostly to it. He saw Theo "lurking near Ines's office earlier in the evening" — this was actually 19:30 (Theo was looking for Ines about a contract), not the commission window. Wyatt's time-confidence is bad: "after dinner sometime."

---

## 6. Ambient noise (Tier 3)

- **Maya Chen across the hall** working late on a pitch. She heard "raised voices and a single sharp shout sometime around 9:30 or 9:45." Assumed it was a TV. Useful for tightening ToD.
- **Coffee shop downstairs closing routine** at 21:00 — barista (Jules) saw nothing relevant; can confirm no one entered the building from the street between 21:00 and 22:30 except Theo's exit/return.
- **Lobby security camera** — covers main entrance, doesn't catch stairwell to architects' floor; confirms badge log times for everyone but adds no new info.

---

## 7. Constraint axis split

| Axis | Sloane (truth) | Theo (red herring) |
|---|---|---|
| **Motive** | ✅ acute (career erasure imminent) | ✅ structural (decade-long, blocked exit) |
| **Opportunity** | ✅ in office throughout | ✅ in office at start of ToD window |
| **Physical evidence** | ✅ keycode logs, weapon access, fibers | ❌ no physical link to office during ToD |
| **Timing** | ✅ in office throughout window | ❌ alibi covers most of window |
| **Witness sighting** | ❌ no one saw her near Ines's office | ✅ Wyatt saw him "lurking" (real, but mistimed) |

Both pass on 3 axes; Sloane fails on `witness sighting`, Theo fails on `physical evidence` AND `timing`. The split satisfies §5.5.1.

---

## 8. Evidence catalog (with tier and discovery path)

### Early tier (~40%)

**Crime scene examination:**
- E1: Body, throat wound, single thrust, defensive posture suggests she saw it coming (`who_affected, where, what_type`)
- E2: No weapon at scene (`derived_signal: missing_item`, facets `what_instrument`)
- E3: Coffee cup half-full at desk — she was working when killed (`when` partial, narrows to evening)
- E4: ToD estimate 21:30–22:30 (forensic) (`when` range)

**Initial walkthrough / facts:**
- E5: Office layout, partners' offices use interior keycodes, design studio supply cabinets use keycode
- E6: Badge log main door (everyone's in/out times) — `record`, facets `who_acted, when, where`
- E7: Petra's discovery testimony (found at 7:30 AM)

**Initial interviews:**
- E8: Petra — was in office until 22:00, in kitchen 21:00–21:30 making tea, heard footsteps in hallway ~21:30, did not see who. Bounded perspective: missing `who_acted`.
- E9: Wyatt — at desk most of evening; saw Theo "near Ines's office earlier, after dinner sometime"; left 22:15. Bounded perspective: vague `when`.
- E10: Sloane — worked at her desk most of the night, left 22:50. Does NOT volunteer studio visit or office entry. (Hostile to interrogation but cooperative on surface.)
- E11: Theo — evasive. "Stepped out for air around 9." Refuses elaboration without legal counsel.

### Mid tier (~40%)

**Pursued through targeted investigation:**
- E12: Interior keycode log for studio supply cabinet — Sloane 21:20 entry. (Pull from IT.)
- E13: Interior keycode log for Ines's office — Sloane 21:35 entry. **(Direct evidence of Sloane in the office during ToD.)** Hidden until specifically requested or comprehensive log pull.
- E14: Interior keycode log for Theo's office — none that night.
- E15: Murder weapon found in design studio sharps disposal bin — X-Acto blade with partial blood traces, partially wiped. (Surfaces if player examines the studio.)
- E16: Forensics on the X-Acto — partial Sloane prints + clear Wyatt prints + traces of Ines's blood. **(Modifier noise: Wyatt prints from his afternoon work.)**
- E17: Wyatt confirms he used X-Actos all afternoon for model prep work. (Resolves Wyatt prints.)
- E18: Sloane's desk drawer — printed resignation letter, dated 9 days before the murder.
- E19: Petra — under follow-up: "Sloane and Ines had been tense for weeks. Sloane skipped Friday-afternoon partner check-in two weeks ago."
- E20: Wyatt under follow-up: "the time I saw Theo? Maybe 7:30, actually, I'd just finished my chinese food. Yeah, before 8 probably." (Time-corrects his earlier statement, but only when pushed.)
- E21: Theo, under pressure: "Fine, I went to my attorney's house. It's a personal matter, leave it." (Surfaces alibi but evasively.)
- E22: Maya Chen testimony from across the hall — raised voices and a single sharp shout "around 9:30 or 9:45." Tightens ToD.
- E23: Lobby camera — confirms Theo's exit 21:01 and return 22:36; no other entries/exits during the window.

### Late tier (~20%)

**Requires specific lead-following:**
- E24: Ines's email server — merger correspondence with Marcus Bell. Reveals firm-merger plan that would have dissolved Sloane's partnership track. **(Reveals Sloane's motive.)**
- E25: Ines's calendar — scheduled call with Marcus the next morning, recent meetings with merger lawyer.
- E26: Sloane's coffee meeting with Whitfield Associates (T-3d) — confirms she was already preparing to leave. Found via her phone records, calendar, or Whitfield-side confirmation.
- E27: David Aronson's calendar — "TR mtg 21:00 home." Requires subpoena or persuasion. **(One path to Theo's alibi.)**
- E28: David Aronson testimony — "Theo arrived around 9:30, we worked until about 10:25." Friendly to Theo but truthful. **(Second path.)**
- E29: Mitra's testimony — neighbor walking dog at 21:30, saw "an older man arrive at David's." **(Third independent path.)**
- E30: Sloane's badge entry to Ines's office at 21:35 (E13) becomes the killer fact when combined with E24 (motive) and E18 (resignation letter).

---

## 9. Discovery dependencies

- **Theo's alibi (and his collapse as suspect)** requires reaching at least one of E27, E28, E29.
- **Sloane's lock** requires E13 (keycode log to Ines's office) + E24 (motive evidence) + ideally E18 (resignation letter) and/or E26 (already preparing to leave).
- **Tightening ToD** to ~21:45 via E22 narrows the window such that Theo's alibi becomes more solid (he was already at Aronson's by then) AND Sloane's 21:35 keycode entry becomes more damning.

---

## 10. Solving notes

The player has effectively solved the case when they can articulate:
- Sloane killed Ines around 21:45
- Motive: imminent merger would have dissolved her partnership track (E24)
- Method: X-Acto from studio (E12, E15, E16)
- Opportunity proven: keycode entry to Ines's office at 21:35 (E13)
- Theo cleared by alibi (any of E27/E28/E29)

If the player misses E13 (keycode log to Ines's office), they may suspect Sloane but lack proof. If they miss E24 (motive), they may have proof of presence but no narrative. If they miss Theo's alibi paths, they may keep Theo as a co-suspect indefinitely.

---

## 11. Tier audit

- Total evidence items: 30
- Evidence omitting `who_acted`: E2, E4, E8 (footsteps no ID), E15 (until forensics), E22 (raised voices no ID), E29 ("an older man") — 6/30 = 20%. **Below 30–50% target.** Pad with: weapon trace before forensics (E15) is genuinely identity-free until E16 surfaces; Petra's hallway hearing is identity-free; Maya's voices are identity-free. Adding bounded-perspective trims to E10 (Sloane's testimony self-omits her studio visit), E11 (Theo's testimony self-omits where he went), E20 (Wyatt's vague timing initially) brings the effective omission rate higher. Acceptable for a hand-authored case — for a generator output, this would prompt re-emission to hit the floor.

- Tier balance: 11 early / 12 mid / 7 late ≈ 37% / 40% / 23%. Within budget.

---

## 12. Player-facing opening

Use this verbatim:

> Wednesday, 7:48 AM. Holloway & Reed Architects, 4th floor of a converted warehouse downtown. Office manager Petra Vance arrived for work eighteen minutes ago and found her boss, founding partner Ines Holloway, slumped at her desk in her private office. Single deep wound to the throat. Coffee cup still half-full on the desk. The office door was ajar when Petra arrived. She called 911.
>
> You're on scene. Forensic estimate puts time of death between 21:30 and 22:30 last night. The badge access system shows five people were in the office at 21:00: Ines, three other architects (Sloane Mercier, Wyatt Knox, and the firm's other founding partner, Theo Reed), and the office manager (Petra). The murder weapon is not at the scene.
>
> What do you want to do first?
