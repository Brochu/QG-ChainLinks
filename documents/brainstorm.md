Alibi-Project: Bainstorm
========================

- Want to try and follow the trend of board games being spun into roguelikes
    * Balatro
    * Dice with Death

# Roguelike Elements
- Procedural generation
- Perma-death
- Meta progression between runs
- Synergies
- Risk reward choices

# What board game to use as a base
- Games I liked as a kid
    * table hockey
    * don't break the ice
    * snafu
    * mastermind - guessing game
    * ! clue !

- Clue might be a good candiate for this kind of spin

## What Roguelike Elements Match Clue

- Final guess directly ends your game
    * right -> win
    * wrong -> out of the game
- Each game is independant; run based aspect
- Each game is "procedurally generated" by the cards being shuffled

## What Elements of Clue Could Cause Problems

- Deductions depend on other players
    * probably would prefer making this game played 1 player
    * could have AI controlled players to deduce from?
- Not many gameplay features to scale; create synergies with

# Possible References

- Dicey Dungeons
- Luck be a Landlord
- Balatro
- Stacklands
- Slay the Spire / Wildfrost / Roguebook
- Inscryption

# Practical Aspects for Solo Dev

This type of roguelike games are usually good for solo dev projects

- Easy to prototype
- Light on animation / art quantity
- Heavy on mechanics / systems
- Flexible in scope

# Choice of Base

I think I would like to work on a deduction and logic game based off of the board games

- Mastermind
- Clue

With some inspiration on the TV show FBI Files for cases or at least the base for the case generation

```
You’re a detective working through a network of hidden plots. Each run is a case or mystery,
and your goal is to deduce the correct solution (e.g., identity, location, method) before running out of
actions/time.
```

# Foundational Mechanics

- Procedurally generated mystery structure
    * multiplying layers help create varied cases
    * might be difficult to make sure the cases are not too easy to guess
    + WHO did it
    + WHAT they used
    + WHERE it happened
    + WHEN it happened
    + WHY or HOW (motive)

- Limit actions per sections of the case
    * want the player's decision to have weight to them
    * cannot let the player spam options until they have all the info they need

- Multiple different sources for clues and information
    * Direct Clues; very powerful, should only be a few
    * Partial Clues; probably the most common type
    * Contradictions; need to make sure not all information gained by the player is useful

    * ways to aquire clues
        + dialogs (interviews)
        + mini-games (lockpicking, swabbing, scrubbing, zooming-in)
        + items
        + random events (eavesdrop, previous failed runs)

- Uncertainty mechanics
    * some clues will provide no value to the case, or even negative value; learn the difference based on case
    * NPCs dialogs contains lies; based on fear, loyalty, bribery

# Advanced Systems (for later)

- Obfuscation system
    * noise to hide real useful information
    * red herrings

- Dynamic World State
    * Each run is a living world
    * Suspects move influence options in the world
    * Player finding/missing info will have effects on the later steps

# Run Structure

- Case starts with some introduction giving an idea of what kind of scene we are investigating and little info

- List of rooms is shown to the player
    * Each room is a step in the investigation
    * Player will go from room to room and try to gather info
    * Cannot move back in the room list

- After room is chosen; show the room to the player with possible options
    * Player will choose actions to take until resource runs out
    * Each action can or cannot bring in more info
    * Player can move around the room at set positions

- After running out of resources in the last room; need to solve the case
    * Requires the player to pick multiple choice options
    * Player either wins or loses, no inbetween
    * There will be runs where the solution might be impossible to get; because of choices made
    * Player should be able to try and solve early; but same outcomes, win or lose

- Each runs should span around 30-40 mins

# Video Study - Investigation / Detective Work
## Police Detective Elements

- Profiling suspects
- Victimology
- Forensic Evidence
- Tracking suspects
    * microphone
    * MIIM phone lines
    * extracting records from phone/internet providers
    * physical trackers w/ GPS
    * social media study (digital forensics)
- Copycats; way to create red herrings
- Compare bullet rifling to ID vs. gun which fired it
- Microscope compare?
    * do we want the player to check for matches? possibly
- Fingerprints analysis/generation
    * need to identify where lines split/merge/creates an island
    * list a series of identifying attributes
    * need to match on that
- Trace evidence
    * powders
    * fibers
    * hair samples
- Drones to help get some angles of the crime scene from the air
    * can be paired with laser scanning, infrared cams, or to create better 3d models of the scene
- Do we want to let the player take photos of the scene?
    * could have some way to score a photo to see how much info it could bring, or how much value it has
- Revolvers do not eject shell casings
    * could lead to some cases where we only have bullets without casings
    * does not mean suspects picked up casings

## Forensics Support

- Chalk drawn around body
    * no longer in use / cross contamination
    * replaced by taken high quality photos in large quantity
    * sometimes 3d models of the scene are made
- Common search methods
    * line search; in one direction; line of officers; can turn
    * grid search;
    * zone search; split area in zones; need to take notes of evience found
- Small test kits
    * can determine if a substance COULD be blood
- Bullet trajectory analysis
- Tire tracks casting analysis
- Checking gun powder residue
    * can give information about how far away was the gun when shot (close or far away)
- Lumenol testing
    * chemical to detect latent blood traces
- Shoe prints analysis

## Fundamentals of Crime Scene Investigation

- Crime scene investigation / Forensic evidence can help with conflicting testimonies
- Investigation is done wearing special equipement to avoid cross-scene contamination
    * shoe covers
    * gloves
    * special uniform
- What is visible by the eye: visible traces
    * there are also invisible traces
- Different visible traces can only be seen in certain lighting
    * like hair, wool, fiber evidence
    * give the option to the player to inspect using a flashlight? blacklight?
- Serious fingerprint analysis needs to be done in a lab (send, results come back later)
    * could task the user of actually comparing prints to identify a match or no (or sometimes give task only)
    * Fingerprints on objects are half the story, also need to grab fingerprints off of suspects
    * ID factors for analysis and comparaison; see screenshots `IDFactors0` and `IDFactors1`
- Very important to take HD pictures of the scene
- Examining the suspect's and victim's clothes can lead to some evidence and clues
    * micro particles / micro fibers
    * can be collected with a piece of tape
    * compared / studied under microscope
- Gun shot residue
    * checking on hands to check if someone fired a weapon
    * can also be checked on victims to check if shot was done at close range?
    * powder is usually all burnt and won't be visible
    * looking for other chemicals
    * can also differentiate between actually firing and just holding the gun after it was shot
        + particles on palms only, but way less in other parts
- Using ultra-violet light to find biological traces / human activity
    * this light can be destructive on some biological material, need to be careful
- Using DNA to help identify suspects at different areas of the scene
    * if DNA samples are found on the scene, we can then grab samples from suspects for comparaison
    * in order to capture DNA, need to have live cells; compared to dead cells in hair or nails
    * compare is done at the sequences level (ex A-T-G-C)

## Crime Scene Investigation & Photography 2015
- It is very much a 2 step process
    * 1st: respond to crime scene; preserve and collect the evidence on site
    * 2nd: lab analysis; uses the collected evidence to find information about the crime
    * both steps not done by the same people
    * for the game, it is okay to blurry the line a little bit to get better entertainment value
- Big connections with ...
    * photography
    * forensic science
- Just entering the crime scene, it gets changed
    * touch things
    * hair falls
    * have to control the crime scene the best you can
- Investigation needs to stay neutral; go against speculations

### Crime Scene Photography

- Need to have a camera with you; phone doesn't count
    * pictures are required, not for people there, but for the jury and people that weren't there
- Need to not sensationalize a crime scene
- Shots taken from outside of the scene first
    * then move in closer and closer to the center of the scene, taking other shots on the way
    * shots taken from general to specific
- Main component: NOT TOUCH ANYTHING
- Cannot mix cases into photo rolls; irrelevant for the game
    * digital makes this step much easier

# Demo Project - What to Progress On

## Movement System v0

- I can place waypoints in the world and teleport move to them
    * will need to have some fade in/out to make it less jarring
    * make sure to place a waypoint on `Player Start` actor as well (to walk back)
- I should be able to rotate the camera by bringing the cursor close to the side of the screen?
    * are there better controls
    * would like to avoid free walking around in the room
    * still give possibility to player to look around from the different waypoints in the room
- Tried to implement a very simple first pass version to test the feel
    * Feels pretty bad
    * sweetspots around the screen to rotate camera doesn't work well with exploration/examination aspects
    * I don't think polish later on can fix these issues
- Should look into a different movement system altogether

## Movement System v1

- Want to come up with a new system that will give the player 100% focus on evidence when examining one
- Need to also make sure the movement system won't lead to players missing details
    * if they miss a detail it's on them, not the camera/movement system
- I think we might need to give two different views to the player
    * Scene View: far away view, give an overview of the whole scene; orbit+pan controls
    * Examination View: in the room view to explore one evidence specifically; static camera
- The overview should give a very good chance to the player to find everything that's in the room
- The Exam view should give the player a chance to look at each focus in details
- Focus points placed in the room
- Selecting a focus point will transition from Scene view to Exam view

## Other Systems

- Need to build interaction system
    * already have a start with IInteract to get which icon to show
    * system needs to be able to
        + give evidence
        + goto minigame
        + show UI
    * this will be a large system in the actual game
    * for now, we can build it as we need it
    * this will help identify all cases for the actual implementation
- Need to build a dialog system
    * different context: phone, in-person, recordings, eavesdropping
    * can be w/ a person that needs an avatar on screen (can be without too)
    * need to control the pace of text shown on screen
    * look into rich text, but not needed for now
    * need to give response options to the player
    * keep track of option trees for progress
    * need to handle some procedural content as well as pre-authored content
- NPCs in the game should have a mood value? (Look into this, do we need this?)
    * ranging from hostile to neutral to ally
    * actions taken during the case will have an effect on those values (Maybe stays static per case?)
    * more likely to get helpful info/actions out of allies
    * still possible with hostile, may need some quick thinking?
- Very important to take HD pictures of the scene
    * give access to a camera to the player
    * helps them remember some details about past rooms
    * might need to save metadata in each picture to unlock some more information later?
    * look into saving the visibility buffer with objectIDs for this
    * maybe custom stencil buffer?
    * possible to implement picture taking just as a helper for player memory
    * in that case, no need to have special render system, just saving pictures for reference later in the case
- Maybe it would be interesting to focus on finding information about motives
    * find out which suspects would have a motive to commit the crime at hand
    * maybe one of the main information to provide to win/lose a case is the motive
