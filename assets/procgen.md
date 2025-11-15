# Procedural Generation - Cases

I know I would like it to be based off of Dwarf Fortress

- Multiple layers of generation passes
- Pass N+1 will use the outputs of pass N
- Final outputs
    * List of locations
    * List of characters
    * Event graph of simulated history in the world
    * True chain of events of a crime

## Generation/Simulation Passes

0. GLOBAL PARAMETERS (hand-picked)
   - City size tier (village / town / city / mega)  
   - Crime rarity knob (1/50..1/5000 citizens / year)  
   - Genre sliders (violence, organised crime, supernatural, corruption)  
   - Real-world decade (affects tech: DNA 1986+, CCTV 1975+, cell-phones 1995+…)

1. FOUNDATION PASS – “City skeleton”
   Input: 0  
   Output:  
   - District table (name, type, wealth, roughness, nightlife, police-response-time)  
   - Landmark table (bar, dock, bank, hospital, casino, abandoned warehouse…)  
   - Transit table (time-of-day → travel-time matrix between every two districts)  
   - Routine table (per landmark: open/close hour, peak hour, staff head-count)

2. POPULATION PASS – “Who lives here?”
   Input: District, Landmark  
   Output:  
   - Person table (id, name, age, sex, job, home-district, workplace, daily-routine-graph, wealth, secrets-bitfield)  
   - Group table (id, type: family / street-gang / corp / police / cult, leadership-graph, home-turf, criminality-score)  
   - Membership table (person-id, group-id, role, joined-date, trust-level)  
   - Relationship table (person-a, person-b, type: friend/rival/lover/debtor, strength, last-interaction-day)

3. MOTIVE PASS – “What do they want?”
   Input: Person, Group, Relationship  
   Output:  
   - Desire table (person-id, desire-type: money, revenge, silence, status, thrill, ideology, target-id, urgency 1-10)  
   - Secret table (owner-id, secret-type: affair, fraud, prior-murder, addiction, witness-old-crime, value 1-10)  
   - Blackmail-link table (would-blackmailer-id, target-id, secret-id, probability)  
   - Feud table (group-a, group-b, casus-belli, severity)

4. CRIME SEED PASS – “Which desire boiled over?”
   Input: Desire, Secret, Feud  
   Output:  
   - Crime table (id, type: murder / arson / kidnapping / heist / blackmail, motive-id, date-slot, rough-location)  
   - Victim table (crime-id, person-id, vulnerability-score)  
   - Perpetrator table (crime-id, person-id, plan-skill, plan-time-days)  
   - Accomplice-slot table (crime-id, role: driver, hacker, lookout, cleaner, filled-by-person-id (nullable))

5. PLANNING PASS – “How did they try to succeed?”
   Input: Crime, Perpetrator, Accomplice-slot, Transit, Routine  
   Output:  
   - Timeline table (event-id, crime-id, time-offset-minutes, action: procure-weapon, scout, lure-victim, commit, flee, destroy-evidence)  
   - Evidence-slot table (event-id, type: DNA, fingerprint, CCTV, audio, eyewitness, physical-object, digital-log)  
   - Alibi-slot table (person-id, time-window, claimed-location, witness-list)  
   - Risk table (event-id, estimated-discovery-probability, mitigation-action)

6. EXECUTION PASS – “What actually happened?”
   Input: Timeline, Evidence-slot, Alibi-slot, Risk  
   Output:  
   - True-timeline table (same as Timeline but with **actual** success/failure flags)  
   - True-evidence table (evidence-id, event-id, final-state: planted / missed / destroyed / overlooked, quality: partial/clear)  
   - Witness table (person-id, event-id, visibility-level: full/partial/none, credibility, willingness-to-talk)  
   - Coincidence table (extra unrelated person present, patrol-car nearby, power-cut, weather…)

7. INVESTIGATION HOOK PASS – “What does the detective see on day 1?”
   Input: True-timeline, True-evidence, Witness  
   Output:  
   - Initial-scene table (crime-id, location, initial-evidence-list, first-responding-officer-id)  
   - Lead table (lead-id, source: scene / witness / database / anonymous-tip, relevance 1-10, decay-function)  
   - Misleading-lead table (same as Lead but pointing at innocent, relevance still 1-10)  
   - Available-test table (DNA-lab-backlog-days, CCTV-retention-days, warrant-difficulty)

8. OPTIONAL POLISH PASSES (toggle any)
   - Red-herring generator: picks a random innocent with a weak motive and plants one piece of ambiguous evidence.  
   - Corruption pass: decides whether any investigating officer, lab tech or prosecutor is compromised and will suppress/fabricate.  
   - Press-pass: generates newspaper headlines that alter witness willingness.  
   - Cold-case pass: if unsolved after X days, evidence degrades, witnesses disappear.

## First Steps of Implementation

1. Create empty SQLite file and schema for layers 1-2.  
2. Write a tiny name generator (US census list + Markov chain).  
3. Implement layer 1: generate 5 districts, 30 landmarks.  
4. Implement layer 2: spawn 200 persons, 4 gangs, 1 police precinct.  
5. Pick one knob (e.g., “gang feud severity”) and expose it in a TOML config so you can re-roll quickly.  

Once that loop feels fun to inspect in the DB browser, move on to layer 3.
