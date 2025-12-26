# Simplified Crime Simulation System Design

## Core Philosophy
- Simple, flat data structures over complex nesting
- SQLite for persistence and debugging
- Focus on essential simulation mechanics
- Easy to debug when generation fails mid-way

## Database Schema

### Characters Table
```sql
CREATE TABLE characters (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    age INTEGER,
    gender TEXT,
    occupation TEXT,
    income_level TEXT, -- low, middle, high
    stress_level REAL DEFAULT 0.5, -- 0.0 to 1.0
    impulsiveness REAL DEFAULT 0.5, -- 0.0 to 1.0
    empathy REAL DEFAULT 0.5, -- 0.0 to 1.0
    self_control REAL DEFAULT 0.5, -- 0.0 to 1.0
    location_id INTEGER, -- current location
    home_location_id INTEGER, -- where they live
    work_location_id INTEGER -- where they work
);
```

### Locations Table
```sql
CREATE TABLE locations (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    type TEXT, -- home, work, social, public
    zone TEXT, -- downtown, residential, commercial
    capacity INTEGER,
    security_level TEXT -- low, medium, high
);
```

### Relationships Table
```sql
CREATE TABLE relationships (
    id INTEGER PRIMARY KEY,
    char1_id INTEGER,
    char2_id INTEGER,
    relationship_type TEXT, -- family, friend, romantic, colleague, enemy
    strength REAL DEFAULT 0.5, -- 0.0 to 1.0
    trust_level REAL DEFAULT 0.5, -- 0.0 to 1.0
    last_interaction_date TEXT,
    UNIQUE(char1_id, char2_id),
    FOREIGN KEY(char1_id) REFERENCES characters(id),
    FOREIGN KEY(char2_id) REFERENCES characters(id)
);
```

### Events Table
```sql
CREATE TABLE events (
    id INTEGER PRIMARY KEY,
    date TEXT,
    event_type TEXT, -- interaction, conflict, life_change
    description TEXT,
    participants TEXT, -- JSON array of character IDs
    location_id INTEGER,
    emotional_impact REAL, -- -1.0 to 1.0
    stress_change REAL, -- change in stress levels
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Conflicts Table
```sql
CREATE TABLE conflicts (
    id INTEGER PRIMARY KEY,
    date_started TEXT,
    participants TEXT, -- JSON array of character IDs
    conflict_type TEXT, -- resource, romantic, professional, personal
    description TEXT,
    severity REAL, -- 0.0 to 1.0
    escalation_level REAL DEFAULT 0.0,
    resolution TEXT -- resolved, ongoing, escalated_to_crime
);
```

### Crimes Table
```sql
CREATE TABLE crimes (
    id INTEGER PRIMARY KEY,
    date TEXT,
    crime_type TEXT, -- theft, assault, fraud, murder, etc.
    perpetrator_id INTEGER,
    victim_id INTEGER,
    location_id INTEGER,
    description TEXT,
    evidence TEXT, -- JSON array of evidence items
    motive TEXT,
    method TEXT,
    solved BOOLEAN DEFAULT FALSE
);
```

## Simplified Character Generation

### Character Personality System
Instead of complex personality traits, use simple archetypes:

```sql
CREATE TABLE character_archetypes (
    id INTEGER PRIMARY KEY,
    name TEXT,
    stress_tendency REAL, -- how quickly they accumulate stress
    impulsiveness REAL, -- tendency to act without thinking
    empathy_level REAL, -- concern for others
    risk_tolerance REAL, -- willingness to break rules
    coping_mechanism TEXT -- work, social, substance, violence
);
```

### Character Generation Logic
1. **Demographics First**: Age, gender, occupation, income
2. **Location Assignment**: Home based on income, work based on occupation
3. **Archetype Selection**: Pick personality type
4. **Basic Relationships**: Family, close friends, romantic partner
5. **Stress Background**: Ongoing life pressures (debt, health, relationship issues)

## Simplified Conflict System

### Conflict Triggers
1. **Resource Competition**: Job promotions, romantic interests, money
2. **Value Conflicts**: Different beliefs, moral disagreements
3. **Personality Clashes**: Introvert/extrovert, neat/messy, etc.
4. **External Pressure**: Economic downturn, family crisis, health issues

### Conflict Escalation
- **Level 1**: Minor disagreement, slight relationship damage
- **Level 2**: Serious argument, trust broken
- **Level 3**: Major confrontation, relationship ends
- **Level 4**: Escalates to criminal behavior

### Crime Generation
When a character reaches high stress + low self-control + opportunity:

```sql
-- Crime generation conditions
stress_level > 0.8 AND 
impulsiveness > 0.6 AND 
opportunity_exists == TRUE AND
conflict_severity > 0.7
```

## Simplified Simulation Loop

### Daily Simulation
1. **Character Updates**
   - Adjust stress based on recent events
   - Move characters between locations
   - Update relationships with recent interactions

2. **Interaction Generation**
   - Check for characters in same location
   - Generate interactions based on relationship strength
   - Create small conflicts or strengthen bonds

3. **Conflict Monitoring**
   - Check for escalating conflicts
   - Apply stress from unresolved conflicts
   - Trigger crime events when conditions are met

### Event Generation Rules
```python
def generate_interaction(char_a, char_b, location):
    if relationship_type == "family":
        return generate_family_interaction(char_a, char_b)
    elif relationship_type == "romantic":
        return generate_romantic_interaction(char_a, char_b)
    elif relationship_type == "colleague":
        return generate_work_interaction(char_a, char_b)
    else:
        return generate_general_interaction(char_a, char_b)
```

## Case Extraction System

### Crime to Case Conversion
1. **Select Crime**: Choose unsolved crime from crimes table
2. **Evidence Generation**: Create realistic evidence based on crime details
3. **Witness Creation**: Generate believable witness testimonies
4. **Red Herrings**: Add confusing but innocent evidence
5. **Mystery Structure**: Select key unknowns for player to solve

### Evidence Types
- **Physical**: Fingerprints, DNA, weapons, documents
- **Digital**: Phone calls, emails, texts, transactions
- **Testimonial**: Witness accounts, suspect interviews
- **Circumstantial**: Motive, opportunity, behavior changes

## Debugging Features

### State Snapshots
```sql
CREATE TABLE simulation_snapshots (
    id INTEGER PRIMARY KEY,
    date TEXT,
    character_count INTEGER,
    active_conflicts INTEGER,
    high_stress_characters TEXT, -- JSON array
    active_crimes INTEGER,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Error Logging
```sql
CREATE TABLE simulation_errors (
    id INTEGER PRIMARY KEY,
    date TEXT,
    error_type TEXT,
    description TEXT,
    character_ids TEXT, -- JSON array
    simulation_state TEXT -- JSON of relevant state
);
```

### Progress Tracking
```sql
CREATE TABLE generation_progress (
    id INTEGER PRIMARY KEY,
    phase TEXT, -- world_gen, population, conflicts, crimes, cases
    step TEXT, -- character_gen, relationship_gen, etc.
    completed BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

## Example Generation Flow

### Phase 1: World Setup
1. Create 50 locations (homes, workplaces, social spots)
2. Assign zones and types
3. Set up basic infrastructure

### Phase 2: Population Creation
1. Generate 200 characters with basic demographics
2. Assign archetypes and personalities
3. Create home/work locations
4. Generate initial relationships (family, close friends)

### Phase 3: Simulation Run
1. Run 90 days of simulation
2. Track interactions, conflicts, and stress
3. Monitor for crime conditions
4. Log significant events

### Phase 4: Case Selection
1. Review generated crimes
2. Select interesting case
3. Generate evidence and witnesses
4. Create mystery structure

This simplified approach gives you a robust foundation that's much easier to debug, understand, and extend. You can always add complexity later if needed, but this gets you to a working simulation quickly.