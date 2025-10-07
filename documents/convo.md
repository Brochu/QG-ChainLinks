Hey, thinking about what kind of game I could develop on my own. I would like to work on a game I could develop on my own, without a middleware like Unreal or Unity3d.

I know these days roguelikes are quite popular and they seems to usually be reasonable to work on as a solo dev.

I really liked Balatro and I wonder if I could brainstorm what other kind of board games (cards, dice, gambling) would adapt well to having roguelike elements to it?
ChatGPT said:

Absolutely, Alex — Balatro is a great example of how elegant mechanics and procedural content can create a compelling roguelike experience without needing high production values or a huge team. You're spot on: board game-inspired systems are usually rules-heavy but asset-light, which makes them perfect for solo dev projects, especially in genres like roguelike or deckbuilder.

Let’s explore some types of traditional games that could be adapted into roguelikes, and how roguelike elements could be grafted onto them effectively.
🔷 Core Roguelike Elements to Consider

Before jumping into game types, here are the key roguelike pillars worth mixing in:

    Procedural Generation: Game states, opponents, item pools, or rule modifiers.

    Permanent Death / Run-Based Structure: Keep runs short enough to be replayable.

    Meta-Progression: Unlocks, persistent upgrades, or knowledge.

    Synergy Systems: Mechanics that reward combining effects.

    Risk-Reward Choices: The player should regularly gamble on unknowns.

🎴 Board/Card/Gambling Game Templates That Could Work
1. Yahtzee (Dice Poker) / Dice Pool Mechanics

    Core Loop: Roll dice, keep some, reroll others. Score based on combinations.

    Roguelike Twist:

        Unlock new dice with different faces (like “elemental dice”).

        Gain “powers” that affect rerolls or modify dice results.

        Procedurally generated scoring challenges (e.g., a run where “straights” give extra bonuses).

    Meta-Progression: Unlock new scoring boards, passive relics that affect rolls.

2. Dominoes or Tile Matching

    Core Loop: Match tiles to chains, create scoring combos.

    Roguelike Twist:

        Randomized board conditions (e.g., lava tiles, locked zones).

        New “tiles” unlocked mid-run with special effects.

        Adversaries place competing tiles or block moves.

    Possible Tone: Think “Tetris meets Slay the Spire.”

3. Blackjack or Poker Variant

    Core Loop: Form hands that beat an opponent or pass a threshold.

    Roguelike Twist:

        Players collect special cards that have unique effects (e.g., joker that doubles your win, or a cursed ace).

        Randomized dealers with rules (like in Balatro).

        Hand modifiers and status effects: e.g., “poisoned cards,” “burn 1 card on play.”

    Risk-Reward Layer: Push-your-luck systems with escalating rewards.

4. Mahjong or Rummy-like Pattern Builders

    Core Loop: Assemble tiles or cards into legal sets or sequences.

    Roguelike Twist:

        Draft from a rotating pool of cards.

        Board state has hazards — e.g., "corrupted" tiles that destroy combos.

        Competing AI trying to complete patterns faster.

5. Craps / High-Risk Dice Games

    Core Loop: Roll dice, bet on outcomes.

    Roguelike Twist:

        Create a game where “luck manipulation” is a mechanic.

        Add trinkets or equipment that skew probability distributions.

        Turn-based combat driven by wagers and rolls.

6. Memory / Deduction (e.g., Clue, Mastermind)

    Core Loop: Uncover hidden information using logic.

    Roguelike Twist:

        Each run is a different “crime” or “mystery” with randomized clues.

        Clues can be gained via mini-games, bribery, deduction battles.

        Limited actions per turn/run increases tension.

7. Bingo / Push-the-Board Games

    Core Loop: Fill a board from a pool of numbers or icons.

    Roguelike Twist:

        Effects are triggered on row/column completion.

        “Events” corrupt spaces or add new rules.

        Modifiers like “every even number now explodes nearby tiles.”

🧠 Design Hooks You Could Use

    Synergy-Centric Systems: Like Balatro, make effects chain and scale wildly — this keeps runs exciting.

    Shopkeepers / Events / RNG Map Nodes: Let the player chart a course with uncertain rewards.

    Themes to Explore:

        Cyberpunk gambling dens.

        Demon-run board games (à la Inscryption).

        Cosmic horror casino.

        Prison gambling ring with shifting alliances.

✅ Games to Look At for Reference

    Dicey Dungeons: Classic roguelike + dice.

    Luck be a Landlord: Slot machine roguelike.

    Balatro: You already know.

    Stacklands: Card-based automation game.

    Slay the Spire / Wildfrost / Roguebook: Deckbuilding pacing and structure.

    Inscryption: Weird narrative + roguelike mechanics with card game base.

🛠 Practicality for Solo Dev

These types of games are often:

    Easy to prototype.

    Light on animation/art load.

    Heavy on mechanics/systems (good fit for you as a senior graphics/engine dev).

    Flexible in scope — start with a simple engine and expand.

If you want to brainstorm further toward a specific theme or mechanic (e.g., a game about building dice-powered combos in a dungeon), I’d be happy to help design something more detailed.

Would you prefer something more combat-focused, puzzle-oriented, or economy-driven?
You said:

6. Memory / Deduction (e.g., Clue, Mastermind)

    Core Loop: Uncover hidden information using logic.

    Roguelike Twist:

        Each run is a different “crime” or “mystery” with randomized clues.

        Clues can be gained via mini-games, bribery, deduction battles.

        Limited actions per turn/run increases tension.



Okay, I think this idea has potential. I used to play Clue and Mastermind with my parents when I was younger and I think these games are really fun. I would be curious to continue discussing all possible mechanics that could be added to make it interesting in a roguelike context
ChatGPT said:

Great choice — a roguelike built on deduction and logic has tons of potential for depth, especially with a system-driven approach. Let’s break down the mechanics into modular components so you can pick and expand what fits your vision. This type of game can stand out by focusing on knowledge as power instead of stats or loot.
🧩 Core Gameplay Loop (Deduction Roguelike)

Premise Example:
You’re a detective, thief, or spy working through a network of hidden plots. Each run is a case or mystery, and your goal is to deduce the correct solution (e.g., identity, location, method) before running out of actions/time.
🔍 foundational Mechanics
1. Procedurally Generated Mystery Structure

    A “case” is generated from a solution space, like:

        WHO did it (character from a pool)

        WHAT they used (weapon/item)

        WHERE it happened (location)

        WHY or HOW (motive or method modifier)

Think Clue + Mastermind combined: you’re trying to deduce a set of hidden facts from indirect clues.
2. Limited Turns or Actions per Day

    Every clue-gathering action costs time/resources.

    Pressure comes from needing to be efficient with limited time (classic roguelike tension).

3. Clue Types and Sources

    Direct Clues (e.g., “The murder did not occur in the Library”)

    Partial Clues (e.g., “The suspect was seen with a sharp object”)

    Contradictions (e.g., multiple NPCs give conflicting accounts)

Clues can be acquired via:

    Dialogues (interviews with suspects, each with limited patience or lies)

    Mini-games (codebreaking, puzzle locks, searching areas)

    Items (e.g., fingerprints, blood samples)

    Event nodes (random encounters, bribery, eavesdropping)

4. Uncertainty Mechanics

    Clues might be false, partial, or corrupted.

    NPCs could lie based on alignment, fear, loyalty, or bribery.

    You might collect “meta-clues” about how trustworthy a source is.

🧠 Progression Systems
1. Rogue-lite Progression (Meta Unlocks)

    Unlock new cases, deduction tools, locations, or perks over time.

    Example perks:

        “Always start with 1 known fact”

        “Gain 1 bonus interview per run”

        “Can double-check 1 clue per case”

2. Character Specializations (Loadout System)

    Detective Archetypes (e.g., Analyst, Interrogator, Forensic Expert)

    Each one starts with different tools:

        Analyst: gets better logic solver tools

        Interrogator: uncovers lies more easily

        Forensic Expert: can find physical clues in rooms

3. Information Notebook

    All clues logged.

    Can draw logical links (possibly even automatically like Return of the Obra Dinn).

    “Mark as trusted,” “Mark as contradiction,” etc.

🎲 Advanced Game Systems
1. Obfuscation System

    Noise: Random, misleading clues.

    Red Herrings: Entire suspect lines that are valid on the surface but false.

    Shadow Mechanics: E.g., a third party planting false evidence.

2. Dynamic World State

    Each run is a living world.

    Suspects might move, talk to each other, destroy evidence.

    Player can miss or influence certain outcomes.

3. Consequences of Accusations

    Incorrect accusations could:

        Lose time.

        Scare off the real suspect.

        Lead to alternate endings.

📦 Presentation Ideas

    Boardgame aesthetic: Stylized rooms, tokens, and cards.

    Notebook UI: Keeps it textual and logic-driven, minimizes art asset scope.

    Stylized Noir / Steampunk / Cyberpunk themes: Easier to design distinct traits.

🧭 Run Structure Ideas

    Shorter Runs (20–40 mins)

        Solve a crime or mystery from scratch.

    Longer Campaigns

        Each run gives partial knowledge of a larger conspiracy (think Obra Dinn + Slay the Spire).

        “Chains” of mysteries: same characters return, evolving behaviors.

        Corrupted clues carry over — did you solve the right mystery?

If you're interested, I can help design:

    A case format template (how to structure mysteries in data).

    An in-game logic system (how deduction checks work).

    A clue generation system.

    Early game flow / UI mockups.

Would you want to start prototyping the mystery logic, or focus next on the theme/tone of the game world?
You said:
I am a bit concerned about the amount of efforts that would be required to generate "cases" for the procedural generation aspect of the game. But it might be an interesting challenge at the same time.

I would be to hit a fine line between something that is not too predictable but not too hard to program.... I guess if Clue the boardgame did it, it is possible lol
#-# CASE PROCEDURAL GENERATION
ChatGPT said:

You're absolutely right to pause there — procedural generation of meaningful "cases" is the toughest design and programming challenge in this kind of game. But you're also spot on: Clue managed to build endless replayability with a very simple deduction model, which proves you don’t need something wildly complex to start.

Let’s walk through how you can keep things lean, deterministic, and extensible — hitting that fine line between predictability and procedural variety.
✅ Design Philosophy: Simplicity First, Depth Later

Clue works because of three smart constraints:

    Fixed number of solution slots: 3 categories (Suspect, Weapon, Room).

    One correct item per category.

    Players gain binary information: is this suspect not guilty, is this room not the scene, etc.

Your procedural system can be modeled after this — but layered slightly to allow growth.
🔧 Minimal Case Generator: Practical Model

Let’s define a minimal system you could prototype today.
🎯 Case Structure

Each "case" consists of:

    1 Guilty Suspect

    1 Weapon (or method)

    1 Location

Your game loop is then about eliminating incorrect items in each category.

{
  "suspects": ["Alice", "Bob", "Carol"],
  "weapons": ["Knife", "Poison", "Statue"],
  "locations": ["Library", "Garden", "Study"],
  "solution": {
    "suspect": "Bob",
    "weapon": "Poison",
    "location": "Study"
  }
}

Boom — you've got a solvable logic problem.
🧠 Procedural Variety: Small Tweaks, Big Impact

Even with just 3×3×3 = 27 combinations, smart clue generation can make it feel fresh.
🔄 Procedural Elements

    Random selection of solution triplet.

    Random shuffle of clue types (some clues rule things out, some in, some are misleading).

    Random clue delivery order.

    Random suspect personalities (liar / nervous / honest).

    Optional twist modifiers (e.g. "There are two weapons involved!" or "The real suspect swapped places.")

You don’t need huge data — you need a modular clue logic engine.
🧰 Suggested Architecture (Lean Prototype)
1. Case Generator

    Randomly pick 1 valid item from each category.

    Track the correct triplet as the solution.

    Mark the rest as red herrings.

2. Clue Generator

    Create clues like:

        "Carol was not in the Study."

        "The murder weapon was sharp." (can apply to multiple weapons)

        "Alice saw Bob carrying Poison." (semi-indirect clue)

    Clues can be rule-based or sentence templates filled in with logic.

3. NPC System (Optional)

    Each suspect delivers 1–2 clues.

    Their trustworthiness is tagged (truthful, deceptive, unreliable) to add misdirection.

4. Notebook / Deduction UI

    Let the player eliminate items per category.

    Once they’re confident, they can accuse or confirm the full triplet.

🔁 Balancing Predictability vs. Surprise

Start small:

    Fixed set of 5–6 clues per run.

    Hand-write some first, validate deduction paths.

    Then move to templated, semi-randomized clues.

By focusing on logic consistency (rather than tons of content), you avoid content bloat while still enabling deep play.
🔍 Reference: Obra Dinn vs. Clue

    Obra Dinn uses logic-only deduction over many facts.

    Clue uses elimination with known solution types.

    You're sitting in between — start like Clue and optionally add Obra Dinn-style rule deduction later (e.g. "Only the killer had access to the locked room").

TL;DR – What to Build First

Here’s a minimal viable path:

    Define 3 categories (suspect, weapon, room).

    Write a function to randomly pick a valid triplet.

    Write 5–10 clue templates with logic (true and false variants).

    Build a simple UI (terminal or in-game) to view clues and guess the solution.

    Expand gradually: more categories, more clue types, suspect personalities.

If you want, I can help draft a very small JSON-based prototype case format + a clue generator system for it. Would that help clarify the next step?
You said:
If I am going to start a small project to work on this once in a while, what would be a good code name for this? I'll probably change it later on, I just want to have a placeholder for now.
ChatGPT said:

For a placeholder codename, you want something short, easy to type, and loosely related to mystery/deduction without locking you into a final theme.

Here are some good options:

    Project Red Herring – playful, fits the idea of misleading clues.

    Project Whodunit – classic mystery slang, easy to remember.

    Project Fog of Truth – evokes uncertainty and deduction.

    Project Cipher – short, thematic, works even if you pivot to puzzles.

    Project Glasshouse – implies seeing through lies, investigation vibe.

    Project Ink – simple, could tie to a detective’s notebook.

    Project Parallax – metaphor for seeing the same thing from different angles.

If you want something really minimal for a repo name, I’d go with Project Cipher or Project Ink — both are short and won’t feel silly if the final game changes direction.

I can also give you a list of one-word detective-style codenames if you want to keep it ultra-clean for version control. Would you like me to compile that?
You said:
Yea I think it would be better as a shorter one word codename
ChatGPT said:

Got it — here’s a list of short, one-word codenames that work well for a mystery/deduction roguelike prototype:

    Cluebit – clue + tidbit.

    Deduce – straight to the point.

    Enigma – classic, mysterious, evergreen.

    Cipher – code-breaking, fits puzzles or logic.

    Whisper – hints, secrets.

    Alibi – immediately screams “crime/mystery.”

    Eureka – the moment of realization.

    Sleuth – playful but thematic.

    Prysm – prism + pry, seeing through deception.

    Trace – both tracking and leftover evidence.

If I were picking for a repo name, Cipher, Alibi, or Trace feel the cleanest and most future-proof.

If you want, I can narrow this down further to ones that are also guaranteed low-collision for GitHub repo names so you don’t have namespace headaches later. Would you like me to do that?
You said:
Hey, so I was thinking more about this idea and I want to throw things your way for input and feedback.

So I am pretty sure I would like to go the route of having a series of rooms to visit as the main "progress" of the case. I am also sure I would like to go forward with the idea that this series of rooms are visited in forward order only, no backtracking will be available to force the player to memorize some information.

However, I am less sure of what the game looks like in each rooms. I know I would like to have an action limit to make sure the player feels like the decisions have a weight to them. However, I would like to avoid the easy answer to use deck building mechanics where all the actions are represented by cards and the player chooses a card per actions.

How would I limit actions in a room but in a refreshing way for players?
ChatGPT said:

Gotcha — you’re aiming for a forward-only investigation roguelike where each room offers limited interactions, forcing players to think carefully about their choices without defaulting to the overused “play a card” mechanic.

Here’s how I’d approach limiting actions in a refreshing way while fitting your memory/deduction theme:
1. The “Time Budget” With Multiple Costs

Instead of just saying “You have X actions,” you could give the player a time pool for the room, and different actions cost different amounts of time.

Example:

    Searching a desk = 2 minutes

    Talking to a witness = 5 minutes

    Picking a lock = 7 minutes

    Following a suspicious noise = 10 minutes

This makes players weigh small/quick interactions vs big/high-payoff ones — and time pressure feels more natural than an arbitrary “3 actions per room” rule.
2. Locked Opportunity Slots

Give the player 3–4 “slots” per room, but lock certain actions behind “key requirements” so they can’t just spam optimal ones.

Example:

    Slot A = Conversation

    Slot B = Physical Search

    Slot C = Observation / Surveillance

The player can fill each slot once before moving on. It’s simple but forces variety without overwhelming.
3. The “Hot/Cold” Information Hunt

Each action gives partial clues that get sharper as you “zero in,” but you can’t take too many in the same room or the lead gets cold.

Example:

    Inspect a drawer → You learn about a missing letter (vague clue).

    Question a witness → Confirms who the letter was for (specific clue).

If you want this to be refreshing, you could limit how many steps toward clarity you can take per room, meaning some leads must be remembered and resolved later.
4. The “Focus Meter”

Give the player a mental stamina bar for the run. Actions in rooms consume focus, and the cost depends on complexity. If they overexert early, they’re worse at interpreting clues later (e.g., increased chance of false interpretations).

This plays into long-term resource management — like HP in a dungeon crawler, but here it’s “mental sharpness.”
5. Physical Navigation Within the Room

If you make rooms a mini exploration space (even just a static screen with clickable zones), you can:

    Limit movement points instead of “actions.”

    Hide interactions so the player must choose where to spend limited movement.

    Add tension by making certain points of interest mutually exclusive to reach in the time available.

6. Social Risk Mechanic

Conversations could carry a trust meter per NPC — too many probing questions in a single visit could shut them down.
This way, the “limit” is not a counter but the fact that people won’t talk forever. It’s organic and fits the narrative.
7. Chain Reactions & Opportunity Costs

Instead of flat “3 things per room,” actions could consume opportunity tokens that trigger changes elsewhere.
Example:

    Spend your only “surprise” token in this room → NPC in next room reacts differently.

    Fail a risky action → You lose a future opportunity instead of just an action.

🔹 My Recommendation

If you want it to feel refreshing, avoid a hard numeric “X actions” limit.
Instead, blend time budgets + opportunity slots + social limits so every choice has immediate and delayed consequences.

That way, players:

    Make strategic trade-offs in the room.

    Must remember and plan for what they didn’t check.

    Feel tension not from an arbitrary turn counter, but from the fear of missing the one lead that breaks the case.

If you want, I can mock up a quick room flow diagram showing how “forward-only, action-limited investigation” could look with these mechanics tied together. It would help you see how a run might actually play out.

Do you want me to draw that up?
You said:
Okay so my opinion on these idea are

I really like the point 1. having different time budget for different actions makes a lot of sense and I think could work well. It would also lead most likely to have a defined set of possible action for a given room? I am just curious how I would link actions to a given room? Or do the player learn them over time? have every action available for every room doesn't feel reasonable. Player could improve in some actions as a meta-progress system.

I also like point 7. I do think it is necessary that some actions in one room will have chain reaction events happen in other rooms. This is a compelling part of the game I think will feel pretty good. I also had the idea that some actions might only give results at a later room, for example sending biometrics for analysis to the office could only give results 2-3 rooms later. It would add to some of the mental burden the player has which we could use to tweak the game's difficulty?

I am not sold of the locked opportunity slots, maybe because I am not sure if I understand the concept well yet. Maybe give me more examples?

I am not sure I want to go too deep into the social / interview part of this investigation process since I think this could easily become a very big time sink for development time. I think keeping the interview and social aspects of the game simple might be best for this case?
ChatGPT said:

Got it — I think you’re already honing in on the two strongest pillars for your forward-only, room-based roguelike investigation:

    Room-specific action menus with time-based costs (point 1)

    Delayed consequences and chain reactions (point 7)

Let’s go through your points in detail and address the “how” so it stays lean for a solo dev.
1. Room-specific Actions + Time Costs

    “Would a given room have a defined set of possible actions?”
    Yes — I’d make each room have a small, curated set of possible actions that are relevant to that space and context.
    This does a few good things:

    Avoids absurd actions in irrelevant spaces (no “dust for prints” in an empty street unless there’s something specific there).

    Lets you hand-craft room templates without needing to write infinite unique logic.

    Makes the learning curve natural: over multiple runs, players remember what types of actions are common in each room type.

Example:
Room	Actions	Time Cost
Crime Scene	Examine body (5m), Search desk (3m), Check window (2m), Collect fingerprint (4m)	
Suspect’s Apartment	Search bookshelf (2m), Check trash bin (4m), Unlock safe (7m), Scan documents (5m)	
Forensics Lab	Run fingerprint analysis (0m, delayed 2 rooms), Check ballistics (0m, delayed 1 room), Retrieve old reports (3m)	

Meta-Progression Hook:
Over multiple runs, players could unlock skills that reduce time costs or reveal extra hidden actions in certain room types.

    “Efficient Search” → Desk search time goes from 3m → 1m

    “Quick Lockpick” → Safe unlock goes from 7m → 4m

    “Eagle Eye” → Detects an additional hidden clue in the room

2. Chain Reactions & Delayed Results

You’re spot on — having an action’s payoff occur later is perfect for your “forward-only” rule because it:

    Adds mental load: “I’m waiting for the lab results; better remember that for when I get them.”

    Gives weight to choices: picking that action means sacrificing another immediate opportunity.

    Allows foreshadowing: maybe the player knows something will happen later, but not exactly how.

Examples:

    Room 2: Send fingerprints to lab → get results in Room 5.

    Room 3: Plant a listening device → eavesdrop on suspects in Room 6.

    Room 4: Leave an object behind → it triggers an event in Room 7.

This delayed system can also feed difficulty:

    Easy mode → Delays are shorter, more predictable.

    Hard mode → Delays are longer, less predictable, and sometimes fail entirely.

3. Clarifying “Locked Opportunity Slots”

You said you weren’t sold yet — fair. Let me give a better example:
The idea here is that instead of having a generic pool of X actions, you have categories of action, and each can only be used once per room.
It’s not about forcing variety just for the sake of it — it’s about forcing you to think about coverage.

Example:
Room has 3 categories:

    Search (physical investigation)

    Test (scientific/forensic)

    Observe (look for patterns, watch behavior)

You can do 1 from each per room, but never more.
So even if there are 4 searchable spots, you can only search 1 in that visit.
The constraint is type-based, not total number-based.

This works best if:

    The player knows categories ahead of time.

    The room has more possible actions in a category than the player can take.

    Meta-progression can add extra uses per category.

That said, if you’re leaning toward time budgets instead, you could skip this system — it’s just an alternate constraint.
4. Simplifying Social / Interviews

I think you’re right to keep it light for a solo project.
Instead of branching dialogue trees, social interaction can be:

    Single-action “interrogate” with variable info quality based on time cost spent (quick vs thorough).

    Tied to skill checks: “Spend 3 minutes” → get vague info, “Spend 6 minutes” → get solid fact.

    Mostly used as information delivery rather than gameplay core.

This way, it’s just another action type, not a separate deep system.
5. Suggested MVP Flow

Here’s what I’d suggest for a first prototype:

    Room Templates: Create 3–4 rooms with 3–5 actions each, each with a time cost.

    Forward-Only Progression: Player moves room to room, carrying info forward.

    Action Limits: Use the time pool mechanic; actions have different costs.

    Delayed Effects: At least 2 actions per run give results 1–3 rooms later.

    Solution: After final room, player makes their accusation based on remembered info.

Once this works, you can add:

    Skill upgrades for time efficiency.

    More room templates.

    More action variety.

If you want, I can map a sample run with 6 rooms, showing where time is spent, where delayed actions resolve, and how chain reactions work. That would give you a concrete picture of player decision-making in your system.

Do you want me to draw that out?
You said:
Okay, I think I would not go in the locked opportunity slots for now, I think linking rooms to the possible actions will lead to a feeling of more variety compared to always having an action of each type per room, even if it isn't in reality.

I like the direction where this is going but there is one last aspect of roguelikes that I am not sure we can fit in this (I think it's not the end of the world if it cannot fit but it would be nice). I really like in Balatro that some specific "builds" will have some synergies that compound a become very very powerful. With the current idea/design there isn't really a scoring system where the points can shoot for the moon or something. There might be some ways to synergize some powers that will give large amounts of info on a case, but then the game becomes solved in 1 or 2 rooms? However, is this a bad thing? Would the feel-good of solving a case very fast be good? But maybe it's bad for replayability?
ChatGPT said:

I see the tension you’re describing — “Balatro-style runaway synergy” is fun because you get that "oh wow, this is broken" dopamine hit, but in a memory/deduction roguelike, too much synergy too early risks short-circuiting the mystery.

Here’s how I’d break it down:
Why instant-synergy might be good

    Feel-good moments: It’s extremely satisfying to pull off a "case cracked in record time" win, even if it’s rare.

    Variety of pacing: Some runs could be slow, tense grinds, others lightning-fast sprints.

    Player bragging rights: "I nailed it in 2 rooms!" is a great anecdote.

Why it could be bad

    Reduced replayability: If players learn a high-power combo and it becomes a consistent shortcut, the “mystery” loses tension.

    Solving before engaging: You lose that middle phase where the player is piecing things together and making tough choices.

    Synergy overshadows variety: Players may start chasing the same combos instead of exploring new approaches.

Potential middle-ground solutions

    Frontload the feeling of power, not the outcome

        Synergies give you huge leaps of insight… but they only apply to one aspect of the mystery.

        You still need to gather a few other critical pieces elsewhere.

        Example: A combo that instantly reveals all suspects’ alibis… but you still need to find the murder weapon and motive.

    Introduce “partial solves” as power moments

        Balatro-like synergies could let you skip 2–3 steps instead of solving the whole case.

        This keeps the run alive while still rewarding you for getting the combo.

    Diminishing returns after the first power spike

        Your first big synergy is a huge leap forward.

        Any subsequent synergy in the same run gives smaller benefits.

        Encourages variety while preserving that first hit thrill.

    Multiple win conditions

        The “fast solve” becomes an alternate win (like a speed bonus) instead of the only win.

        Players can choose between a risky, fast route or a safer, slow one.

If you want my gut take:
I’d allow those “solved in 2 rooms” moments, but balance the rest of the game so they’re rare, celebrated, and still leave some work to do. The key is to make them feel like “I broke the game for one glorious moment” without actually breaking the game loop.

If you want, I can sketch a Balatro-like power system for this deduction format that enables those moments without making them the default outcome. That might give you a clearer vision of how the two styles can mesh.
