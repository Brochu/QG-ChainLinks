# Evidence System

## Overview

Evidence is the **gameplay mechanism** that unlocks information in the Information Matrix. It is NOT a matrix entity type itself (decided in doc 001).

**Two-phase approach:**
1. **Simulation Phase**: Events naturally generate evidence as byproduct
2. **Case Extraction Phase**: Filter useful evidence, inject more if needed for solvability

---

## Phase 1: Natural Evidence Generation (During Simulation)

Certain event types naturally create evidence trails. This happens automatically as the simulation runs.

### Event → Evidence Mapping

| Event Type | Natural Evidence Created |
|------------|-------------------------|
| CrimeStalking | Witness sightings, security footage, cell tower pings |
| CrimeStage(preparation) | Purchase records, search history, tool acquisition |
| ActorEnters/Leaves | Security logs, witness sightings, access card records |
| ActorSeenAt | Witness testimony (potentially unreliable) |
| CrimeCommitted | Physical evidence at scene (weapon, prints, DNA, damage) |
| ObjectMoved | Fingerprints on object, witnesses, missing item report |
| ObjectDestroyed | Incomplete destruction (partial burn, fragments), witnesses |
| ObjectModified | Tool marks, chemical residue, forensic traces |
| PhoneCall | Phone records, cell tower location, voicemail |
| MessageSent | Digital records, recipient testimony, metadata |
| Transaction | Receipts, bank records, witness (cashier) |
| Argument | Witnesses, noise complaints, emotional state changes |
| Threat | Witnesses, written records (if text/email), victim report |
| FinancialChange | Bank records, employment records, court filings |
| RelationshipStatusChange | Social media, witness accounts, documents |

### Evidence Properties

Each piece of evidence should have:
- **Type**: physical, testimonial, documentary, digital, forensic
- **Source Event**: which event generated it
- **Reveals**: which matrix fields it can unlock
- **Reliability**: how trustworthy (witnesses can lie, documents can be forged)
- **Discovery Method**: how player finds it (search scene, interview, request records)

---

## Phase 2: Case Extraction (Post-Selection)

After selecting a crime for export, process the evidence:

**Step 1: Filter Relevant Evidence**
- Identify all evidence connected to primary_crime events
- Discard evidence from unrelated simulation events

**Step 2: Validate Solvability**
- For each blank in the chain, count available evidence paths
- Flag blanks with < 2 evidence sources

**Step 3: Inject Additional Evidence (if needed)**
- If a blank has insufficient evidence, generate more:
  - Add a witness who "happened to see" something
  - Add a document trail (receipt, record)
  - Add physical evidence (object left behind)
- Injection must be plausible within the case context

**Step 4: Balance Evidence Quality**
- Ensure not all evidence is direct/easy
- Mix of: direct proof, circumstantial, misleading (red herrings?)

---

## Evidence Types

| Type | Description | Example |
|------|-------------|---------|
| Physical | Objects found at scene | Murder weapon, crowbar, victim's belongings |
| Testimonial | Witness statements | "I saw a tall man leaving around 9pm" |
| Documentary | Paper/official records | Purchase receipt, employment record, lease |
| Digital | Electronic records | Phone logs, emails, security footage |
| Forensic | Scientific analysis | Fingerprints, DNA, blood spatter, tool marks |

---

## Discovery Methods (How Player Finds Evidence)

| Method | Unlocks |
|--------|---------|
| Search crime scene | Physical evidence, forensic opportunities |
| Interview witness | Testimonial evidence |
| Interview suspect | Alibis (may be false), confessions, inconsistencies |
| Request records | Documentary evidence (phone, bank, employment) |
| Forensic analysis | Forensic evidence (send item to lab) |
| Canvass area | Additional witnesses, security footage |

---

## Open Questions

1. ~~**Red Herrings**: Should we intentionally generate misleading evidence?~~ → DECIDED: Natural only
2. ~~**Evidence Reliability**: How to model unreliable witnesses / false testimony?~~ → DECIDED: See below
3. ~~**Discovery Pacing**: Should some evidence be gated (need clue A to find clue B)?~~ → DECIDED: No gating
4. **Evidence Quantity**: How much total evidence per case? (TBD with playtesting)

## Information Reliability Model

**Sources of wrong/incomplete information in the matrix:**

| Source | Type | Example |
|--------|------|---------|
| Partial observation | Uncertainty | "Saw a tall man, couldn't see face" |
| Suspect testimony | Deliberate lie | False alibi, misdirection |

**Rules:**
- Non-suspect witnesses are always truthful (but may have incomplete info)
- Suspects may lie to cover for themselves
- Matrix can show conflicts when new evidence contradicts existing entries
- Player must use deduction to resolve conflicts

---

## Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-01-18 | Two-phase evidence: natural generation + extraction filtering | Realistic spread from sim, then ensure solvability |
| 2026-01-18 | Evidence injection allowed post-selection | Guarantees solvability without constraining simulation |
| 2026-01-18 | Red herrings: natural emergence only | Simulation creates incidental evidence for non-perpetrators; player deduces relevance |
| 2026-01-18 | No discovery gating | Player has open access to all discovery methods; matrix tracks unlocked state |
| 2026-01-18 | Witnesses truthful, suspects can lie | Keeps reliability simple; wrong info = partial observations or suspect misdirection |
| 2026-01-18 | Matrix supports conflicts | When new evidence contradicts existing info, player resolves via deduction |
