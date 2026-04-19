# 06 — Interview System

## Overview

Interviews are one of the player's primary tools for gathering information. Every actor in the case can potentially be interviewed, but what they reveal depends on:

1. **What they know** — derived from EVT_WITNESS, EVT_MEET, and relationship data
2. **What the player has found** — evidence gating prevents brute-forcing interviews
3. **How accurate they are** — conditions during the witnessed event affect quality
4. **Whether they're cooperative** — relationship to perpetrator/victim affects willingness

---

## Interview Data Model

```sql
CREATE TABLE interview_topics (
    topic_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    actor_id        INTEGER NOT NULL REFERENCES actors(id),
    topic_key       TEXT NOT NULL,           -- unique key: "general", "argument_day5", "marco_debt"
    prompt_text     TEXT NOT NULL,           -- what the player selects: "Ask about the argument"
    response_text   TEXT NOT NULL,           -- what the actor says
    requires_evidence TEXT,                  -- JSON array of evidence_ids needed to unlock, null = always available
    unlocks_topics  TEXT,                    -- JSON array of topic_keys this reveals
    reveals_evidence_id INTEGER,            -- evidence piece this conversation produces (nullable)
    reliability     INTEGER NOT NULL DEFAULT 100,  -- 0-100, how truthful (100=fully reliable)
    is_available    INTEGER NOT NULL DEFAULT 1     -- can be turned off by difficulty
);
```

---

## Topic Generation

Topics are generated per-actor based on their role in the case.

### Topic Categories

**1. General (always available)**
Every interviewable actor has a "general" topic — an open-ended conversation starter.

```
prompt: "Tell me about yourself"
response: "I'm [name], I [occupation] at [workplace]. I live over in [district]."
unlocks: ["routine", "relationships"]
```

```
prompt: "What do you know about [event_date]?"
response: varies by actor involvement
unlocks: depends on what they witnessed
```

**2. Routine / Alibi**
Available after the "general" topic. The actor describes their normal schedule, which the player can compare against evidence of actual movements.

```
prompt: "Where were you on [event_date]?"
response: "I was at [routine_location] until about [time], then went to [next_location]."
// If the actor is lying (perpetrator, accomplice), this contradicts other evidence
```

**3. Event-Specific**
Generated from EVT_WITNESS events. Each witnessed event becomes a topic.

```
prompt: "Did you see anything unusual at [location] on [date]?"
response: varies by witness accuracy (see Accuracy section)
requires_evidence: may require player to have found related evidence first
reveals_evidence_id: creates a new TESTIMONIAL evidence piece
```

**4. Relationship / Character**
Generated from the relationship graph. Actors who know the perpetrator or victim can discuss their relationship.

```
prompt: "Tell me about [person]"
response: "We [relationship_description]. [emotional context]."
requires_evidence: often requires knowing the person exists (finding their name in evidence)
unlocks: deeper topics about motive, history
```

**5. Confrontation**
Available when the player has evidence that contradicts the actor's statements. Only relevant for suspects and unreliable witnesses.

```
prompt: "But this CCTV shows you were at [location], not [claimed_location]"
requires_evidence: [the contradicting evidence_id]
response: "...okay, I was there. But I didn't do anything."
reveals_evidence_id: creates a revised testimony
unlocks: ["confession"] or ["alibi_revised"]
```

---

## Topic Tree Structure

Each actor has a tree of topics that unlock progressively:

```
general
├── routine
│   └── alibi_night_of (requires: knowing crime date)
├── relationships
│   ├── knows_victim (requires: victim's name in found evidence)
│   │   └── victim_debts (requires: debt record evidence)
│   └── knows_suspect (requires: suspect's description in found evidence)
│       └── suspect_temper (requires: argument evidence)
├── witnessed_argument (requires: being at the same location evidence)
│   └── argument_details (requires: noise complaint evidence)
└── confrontation (requires: contradicting evidence)
    └── revised_story
```

---

## Evidence Gating

The `requires_evidence` field gates topics behind evidence the player has already discovered. This is the core mechanic that prevents interviewing everyone first and solving the case without exploring.

### Gating Levels

**No gate (always available):**
- "general" topic
- "routine" topic
- Surface-level "tell me about yourself"

**Light gate (requires having found 1 related evidence):**
- "What happened at [location]?" — requires having found that the actor was at that location
- "Tell me about [person]" — requires knowing that person's name from another source

**Medium gate (requires 2+ related evidence pieces):**
- "Why was [person] angry at [victim]?" — requires relationship evidence AND argument evidence
- "What time did [person] leave?" — requires knowing the person was there AND a time-related evidence

**Heavy gate (requires specific critical evidence):**
- Confrontation topics — require the exact contradicting evidence
- Deep motive questions — require financial records, threatening messages, etc.

### Implementation

```cpp
bool is_topic_available(interview_topic *topic, found_evidence_set *player_evidence) {
    if (topic->requires_evidence == NULL) return true;

    // Parse the JSON array of required evidence IDs
    // Check that all required IDs are in the player's found set
    for each required_id in topic->requires_evidence:
        if (!player_evidence->contains(required_id)) return false;

    return true;
}
```

---

## Witness Accuracy

When generating testimony text from EVT_WITNESS events, accuracy depends on conditions during the original event:

### Accuracy Levels

| Level | Conditions | What they can report |
|-------|-----------|---------------------|
| **Clear** | Same room, daytime, sober | Names (if they know the person), exact actions, exact times, physical details |
| **Partial** | Same room + night, OR same room + intoxicated, OR same landmark different room + day | Physical description (height, build, hair), approximate actions ("they were arguing"), approximate time (±30 min) |
| **Vague** | Same landmark different room + night, OR exterior + night, OR intoxicated + night | Gender, rough size, approximate time (±1 hour), "heard something" |
| **Unreliable** | Heavily intoxicated, very far away, stressed/afraid | May contain errors — wrong hair color, wrong time, wrong number of people |

### Accuracy Determination

```cpp
enum witness_accuracy : u8 { CLEAR, PARTIAL, VAGUE, UNRELIABLE };

witness_accuracy determine_accuracy(sim_event *witnessed_event, sim_actor *witness) {
    bool same_room = (witness->current_room == witnessed_event->room_id);
    bool is_night = is_night(witnessed_event->sim_time);
    bool is_drunk = (witness->intoxication > 30);
    bool is_scared = (witness->fear > 50);

    if (is_drunk && is_night) return UNRELIABLE;
    if (is_scared && !same_room) return UNRELIABLE;
    if (same_room && !is_night && !is_drunk) return CLEAR;
    if (same_room) return PARTIAL;   // same room but night or drunk
    if (!is_night) return PARTIAL;   // different room but daytime
    return VAGUE;
}
```

### Generating Testimony Text

Templates with variable substitution based on accuracy:

**CLEAR:**
```
"I saw [actor_name] and [target_name] at the [room_name].
 [Actor_name] was [action_description]. This was around [exact_time]."
```

**PARTIAL:**
```
"There was a [height] [build] [sex] with [hair] hair at the [room_name].
 They seemed to be [vague_action]. Maybe around [approximate_time]?"
```

**VAGUE:**
```
"I heard [noise_type] coming from somewhere in the [landmark_name].
 It was late, I'm not sure exactly when. Maybe around [rough_time]?"
```

**UNRELIABLE:**
```
"I think I saw someone... a [wrong_height?] figure near the [room_name].
 It was around [wrong_time?]. I'd been drinking, so I'm not really sure."
```

For UNRELIABLE witnesses, some details are deliberately wrong. The game tracks which details are incorrect so it can verify the player's deductions.

---

## Actor Cooperation

Not every actor wants to help. Cooperation depends on their relationship to the key players:

### Cooperation Levels

| Actor Relationship | Cooperation | Behavior |
|-------------------|-------------|----------|
| Neutral bystander | High | Answers honestly, volunteers information |
| Friend of victim | High | Eager to help, may over-share suspicions |
| Friend of perpetrator | Low | Evasive, omits details, might lie about alibis |
| The perpetrator | Very Low | Lies about alibi, denies involvement, only breaks under confrontation with hard evidence |
| Rival of victim | Medium | Cooperative but may try to deflect suspicion onto someone else |
| Afraid witness | Medium-Low | Reluctant, requires reassurance (multiple visits or specific evidence that they're protected) |

### Cooperation in Practice

Low-cooperation actors:
- Have fewer available topics
- Give shorter, less detailed responses
- Require more evidence to unlock deeper topics
- May provide false alibis (creates misleading evidence that the player must cross-check)

The perpetrator specifically:
- Claims to have been at their routine location during the crime
- This alibi can be broken by finding CCTV/witness evidence that they were elsewhere
- Breaking their alibi unlocks the "confrontation" topic tree

---

## Interview Flow (from the player's perspective)

1. Player visits a location where an interviewable actor is present
2. Player selects "Interview [actor_name]"
3. Game shows available topics (filtered by evidence gating)
4. Player selects a topic
5. Actor responds (text displayed to player)
6. New topics may unlock
7. New evidence may be added to player's inventory
8. Player can ask more topics or leave

Each interview visit takes in-game time (not real time — there's no time pressure in this design). The player can revisit actors after finding new evidence to unlock new topics.

---

## Interview Generation Algorithm

```
function generate_interviews(case):
    for each actor in case.actors:
        topics = []

        // General topic (always)
        topics.add(create_general_topic(actor))

        // Routine/alibi topic (always)
        topics.add(create_routine_topic(actor))

        // Event-specific topics (from witnessed events)
        witnessed = query sim_events WHERE event_type = EVT_WITNESS AND actor_id = actor.id
                    AND event_id IN case.relevant_events
        for each witness_event in witnessed:
            original_event = query sim_events WHERE event_id = witness_event.detail_int
            accuracy = determine_accuracy(original_event, actor_sim_state_at_time)
            topics.add(create_event_topic(actor, original_event, accuracy))

        // Relationship topics (for actors who know other case actors)
        for each other_actor in case.actors:
            if other_actor.id == actor.id: continue
            rel = query relationships WHERE from_actor = actor.id AND to_actor = other_actor.id
            if rel exists:
                topics.add(create_relationship_topic(actor, other_actor, rel))

        // Confrontation topics (for suspects)
        if actor.id == case.perpetrator_id:
            topics.add(create_alibi_topic(actor, case))  // false alibi
            topics.add(create_confrontation_topic(actor, case))

        save_topics(actor, topics)
```

---

## Text Template System

To generate natural-sounding dialogue, we use a simple template system with variable substitution:

```
templates/testimony_argument.txt:
"I was at {landmark_name} and {actor_name} started going at it with {target_name}.
 {actor_name} was really {emotion_word} — {escalation_detail}.
 This was around {time_display}."

templates/testimony_argument_partial.txt:
"There were two people arguing at {landmark_name}.
 One was {actor_description}, the other was {target_description}.
 It got pretty heated. This was sometime {time_approximate}."
```

Variables are filled from event data, actor data, and accuracy level. Templates are loaded from text files so they can be edited without recompiling.

Template files per testimony type:
- `testimony_argument_clear.txt`, `testimony_argument_partial.txt`, `testimony_argument_vague.txt`
- `testimony_murder_clear.txt`, `testimony_murder_partial.txt`
- `testimony_presence_clear.txt`, `testimony_presence_partial.txt`
- `character_friend.txt`, `character_rival.txt`, `character_employer.txt`
- `alibi_true.txt`, `alibi_false.txt`
- `confrontation_caught.txt`, `confrontation_deny.txt`

Having 3-5 variants per template prevents repetition across interviews.
