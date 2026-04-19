# 03 — Crime Emergence: The Pressure Cooker Model

## Overview

Crimes are **never scripted**. They emerge from the simulation when psychological pressure on an actor exceeds their capacity to cope. The system has three pressure sources that each lead to different crime types. Tuning is config-driven, making it easy to adjust crime frequency and variety without code changes.

**Initial scope:** Murder and theft. Other crime types (assault, arson, break-in as a standalone crime) can be added later by extending the same pressure model.

---

## Pressure Sources

### 1. Financial Pressure → Desperation → Theft

**Mechanism:** Actors with low personal wealth living in or working around higher-wealth environments feel increasing financial pressure. This models someone who can't make rent, sees expensive things they can't afford, and gradually becomes desperate enough to steal.

**Accumulation:**
```
// Once per day (midnight tick)
daily_gain = max(0, district_wealth - actor_wealth) * DESPERATION_GAIN_PER_DAY

// After losing a job (if employment system is added later)
desperation += 20

// After a debt is created
desperation += 15

// After being refused help by an ally
desperation += 5
```

**Example timeline:**
- Actor wealth=1, district wealth=4 → gains (4-1)*3 = 9 desperation/day
- After 7 days: desperation ≈ 63
- After 10 days: desperation ≈ 90
- Combined with base greed of 40: theft_score = 90*3 + 40*4 = 430 (well above threshold)

**What triggers theft:**
```
theft_score = (desperation * 3) + (greed * 4) + noise
if (theft_score > THEFT_THRESHOLD && opportunity_exists) → EVT_THEFT
```

An "opportunity" means: actor is at a landmark where a stealable object exists and the owner is not in the same room. Objects at pawn shops, shops, and unoccupied homes are primary targets.

### 2. Interpersonal Pressure → Anger → Murder

**Mechanism:** Actors with negative relationships who keep encountering each other build anger through repeated arguments. High-roughness/high-crime-factor locations amplify this. Eventually anger crosses the violence threshold.

**Accumulation:**
```
// After EVT_ARGUMENT
anger_gain = rand_int_min(ANGER_GAIN_MIN, ANGER_GAIN_MAX)
anger_gain = anger_gain * (1.0 + crime_factor / ANGER_ESCALATION_DIVISOR)
anger += anger_gain

// After being threatened
anger += 15

// After witnessing ally being hurt
anger += 20

// Amplified by intoxication
if (intoxication > 30) anger_gain *= 1.5
```

**Natural decay:**
```
// Per tick, when NOT at same landmark as grudge target
anger = max(0, anger - 1)

// Visiting ally reduces anger
anger = max(0, anger - rand_int_min(5, 15))
```

**Example timeline:**
- Day 1: Actor A (aggression=70) and Actor B (rival, strength=-50) both at bar → EVT_ARGUMENT, anger +15
- Day 3: Both at bar again → EVT_ARGUMENT, anger +18 (crime_factor amplifies)
- Day 5: Argument at work → anger +12. Total anger ≈ 45 (after decay)
- Day 8: Another bar argument while intoxicated → anger +22. Total ≈ 55
- Day 10: Confrontation → EVT_THREAT. Anger now 65+
- Day 11: anger > VIOLENCE_THRESHOLD (80), EVT_ACQUIRE_WEAPON
- Day 12: anger > MURDER_THRESHOLD (95), finds target at isolated location → EVT_MURDER

**What triggers murder:**
```
murder_score = (anger * 5) + (impulsivity * 2) + (intoxication * 1) + noise
if (murder_score > MURDER_THRESHOLD && has_weapon && target_reachable) → EVT_MURDER
```

The weapon requirement is critical: it forces a premeditation step (ACQUIRE_WEAPON) that creates evidence and extends the causal chain.

### 3. Opportunistic Pressure → Greed → Theft

**Mechanism:** Even actors without financial desperation may steal if their greed is high and they find themselves alone with valuables. This is the "crime of opportunity" — distinct from desperation theft.

```
opportunity_score = (greed * 3) + (desperation * 1) + noise
if (opportunity_score > OPPORTUNITY_THRESHOLD
    && valuable_object_in_room
    && owner_not_present
    && no_witnesses_in_room) → EVT_THEFT
```

This check runs every tick for actors at locations with stealable objects. It creates surprise thefts that aren't preceded by long pressure buildups — harder for the player to trace because the causal chain is short.

---

## Threshold Configuration

All thresholds are in the config file, making balancing easy:

```ini
[Pressure]
# Desperation
desperation_gain_per_day = 3
desperation_debt_gain = 15

# Anger
anger_gain_min = 10
anger_gain_max = 20
anger_escalation_divisor = 50
anger_decay_per_tick = 1
anger_ally_reduction_min = 5
anger_ally_reduction_max = 15
intoxication_anger_multiplier = 1.5

# Thresholds
confront_threshold = 60       # anger needed to CONFRONT_GRUDGE
threat_threshold = 70         # anger needed for EVT_THREAT
violence_threshold = 80       # anger needed to consider violence
murder_threshold = 95         # anger needed for murder (+ weapon required)
theft_threshold = 200         # theft_score needed (desperation*3 + greed*4)
opportunity_threshold = 150   # opportunistic theft score
fear_flee_threshold = 50      # fear needed to trigger FLEE

# Crime frequency targets
# With these defaults and ~70 actors over 30 days:
# Expected murders: 1-3
# Expected thefts: 3-5
# Expected interesting cases: 5-6
```

---

## Actor Decision Flow (per tick)

```
for each actor (alive, not arrested, not traveling):

    1. UPDATE DRIVES
       - Apply daily desperation gain (if midnight)
       - Decay anger, fear, intoxication
       - Update grudge_target if relationships changed

    2. CHECK ROUTINE
       - If routine says "be at work" and not at work → action = GO_TO_WORK
       - If routine says "be at home" and not at home → action = GO_HOME
       - If routine says nothing and it's evening → 20% chance action = VISIT_BAR

    3. CHECK REACTIVE (overrides routine)
       - If fear > FEAR_FLEE_THRESHOLD → action = FLEE_AREA
       - If SIM_FLAG_PARANOID set → stay hidden, skip public locations

    4. EVALUATE DRIVES (only if no routine/reactive action)
       Score each possible action:

       CONFRONT_GRUDGE:
           score = anger * 5 + impulsivity * 2 + noise
           requires: grudge_target exists, target location known
           threshold: confront_threshold

       STEAL_FROM:
           score = desperation * 3 + greed * 4 + noise
           requires: stealable object accessible
           threshold: theft_threshold

       ACQUIRE_WEAPON:
           score = anger * 4 + desperation * 2 + noise
           requires: anger > violence_threshold, no weapon carried, weapon location known
           threshold: violence_threshold

       MEET_ALLY:
           score = 100 - anger + loyalty * 2 + noise
           requires: ally exists
           (this is a "cooling" action — high loyalty actors seek allies before violence)

       Pick highest-scoring action above its threshold. If none qualify → idle.

    5. CHECK FOR CRIME EXECUTION
       Separate from action selection — runs after positioning:

       MURDER check:
           if anger > murder_threshold
           AND has_weapon
           AND grudge_target in same landmark
           AND grudge_target in same room OR actor enters target's room
           → EVT_MURDER

       THEFT check (desperation):
           if desperation * 3 + greed * 4 > theft_threshold
           AND stealable object in current room
           AND owner not in room
           → EVT_THEFT

       THEFT check (opportunity):
           if greed * 3 + desperation > opportunity_threshold
           AND valuable object in room
           AND no other actors in room
           → EVT_THEFT
```

---

## Post-Crime Behavior

After committing a crime, the actor enters a **post-crime state** that generates additional events and evidence:

### After Murder

1. Actor's fear spikes: `fear += 40 + rand_int_min(0, 20)`
2. `SIM_FLAG_PARANOID` set
3. If murder weapon carried:
   - 70% chance: `EVT_HIDE_OBJECT` at a nearby location (generates evidence of concealment)
   - 20% chance: `EVT_DISPOSE_EVIDENCE` (throws weapon in water/dumpster — harder to find)
   - 10% chance: keeps weapon (sloppy — easier case for player)
4. `EVT_FLEE` from crime scene
5. Actor avoids crime landmark for rest of simulation
6. Actor avoids routines near the crime scene (changes visible in schedule deviation)

### After Theft

1. Actor's fear spikes: `fear += 20 + rand_int_min(0, 10)`
2. `EVT_FLEE` from theft location
3. Stolen object in inventory with `OBJ_FLAG_STOLEN`
4. If actor has low morality and a pawn shop exists:
   - Within 24-48 hours: travels to pawn shop → `EVT_TRANSACTION` (fencing stolen goods)
   - This creates a paper trail!
5. Otherwise: `EVT_HIDE_OBJECT` at home

---

## Ensuring Multiple Cases

Over 30 simulated days with ~70 actors, we want 5-6 crime incidents. The config is tuned to achieve this:

**Murder rate:** With default thresholds, roughly 5-10% of actors have personalities prone to violence (high aggression + high impulsivity). Of those, only actors who accumulate enough anger through repeated encounters with their grudge target will cross the murder threshold. Expected: 1-3 murders per 30-day run.

**Theft rate:** Financial pressure affects more actors. Roughly 20-30% of actors have wealth below their district's level. Of those, actors with moderate-to-high greed will eventually cross the theft threshold. Expected: 3-5 thefts per 30-day run.

**Fallback:** If the simulation produces fewer than 3 crime incidents total, we increase pressure by:
1. Lowering thresholds by 10% and re-running
2. Or re-seeding and running again

**Too many crimes:** Not a problem — more candidates means better selection. We just pick the most interesting 5-6 (see doc 04: Case Extraction).

---

## Crime Interactions

Crimes can affect other actors and trigger secondary crimes:

- **Witness intimidation**: If a crime has witnesses, a low-morality criminal might develop a grudge toward the witness → potential second crime
- **Revenge**: If Actor A murders Actor B, and Actor C was close to B (FRIEND, SPOUSE, FAMILY with strength > 60), Actor C's anger toward A spikes → potential revenge killing
- **Theft cascade**: If Actor A steals from Actor B, B's desperation increases → B might steal from someone else

These cascading effects are emergent from the drive system — no special code needed. The relationship changes from crime events naturally alter drive scores.

---

## Tuning Guide

| Problem | Adjustment |
|---------|-----------|
| Too few crimes | Lower thresholds, increase desperation_gain_per_day, add more RIVAL relationships |
| Too many murders | Raise murder_threshold, lower anger_gain_max, increase anger_decay |
| Too many thefts | Raise theft_threshold, reduce number of low-wealth actors |
| Crimes happen too early (day 1-3) | Lower initial drive values, increase thresholds |
| Crimes happen too late (day 25+) | Increase desperation_gain_per_day, reduce anger_decay |
| Crimes are too predictable | Increase noise range in score calculation, add more randomness to encounter frequency |
| Not enough causal chain depth | Lower confront_threshold (more arguments before murder), require more escalation steps |
