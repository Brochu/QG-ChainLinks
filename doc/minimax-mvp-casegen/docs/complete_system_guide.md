# Crime Simulation System - Complete Implementation Guide

## Overview

This system generates infinite unique detective cases through emergent simulation, inspired by Dwarf Fortress's philosophy of organic world-building. Instead of using templates with random data, it simulates realistic social dynamics where crimes naturally emerge from character interactions and life pressures.

## System Architecture

### 1. Data Storage (SQLite)
All simulation data is stored in SQLite database for:
- **Persistence**: Simulation state survives across runs
- **Debugging**: Easy to inspect when generation fails
- **Performance**: Efficient queries for large character populations
- **Analysis**: SQL queries for simulation statistics

### 2. Core Components

#### **Characters**
- Basic demographics (age, gender, occupation, income)
- Personality traits (stress level, impulsiveness, empathy, self-control)
- Current state (location, relationships, stress)
- Archetype-based personality system

#### **Locations**
- Types: home, work, social, public
- Zones: downtown, residential, commercial, suburban
- Properties: capacity, security level, current occupants

#### **Relationships**
- Types: family, romantic, friend, colleague, enemy
- Strength and trust levels
- Dynamic updates based on interactions

#### **Events & Conflicts**
- Daily interactions between characters
- Conflict escalation system
- Stress accumulation from unresolved issues

#### **Crimes**
- Generated when conditions are met (high stress + opportunity + motive)
- Types: theft, assault, fraud, vandalism
- Evidence, motive, and method tracking

## Implementation Files

### Core Simulation
- **`crime_simulation_core.py`**: Main simulation engine
- **`case_generator.py`**: Converts crimes to detective cases
- **`demo_complete_system.py`**: End-to-end demonstration

### Key Classes

```python
# Character with personality and state
class Character:
    def __init__(self, id, name, age, gender, occupation, income_level, archetype)
    def update_stress(self, change)
    def is_high_risk(self) -> bool  # Checks for criminal potential

# Location system
class Location:
    def __init__(self, id, name, location_type, zone)
    def add_character(self, character)
    
# Simulation engine
class SimulationEngine:
    def run_simulation(self, num_days=90)
    def generate_population(self)
    def create_relationships(self)
    def simulate_day(self, date)
    
# Case generation
class CaseGenerator:
    def generate_all_cases(self) -> List[DetectiveCase]
    def generate_case(self, crime) -> DetectiveCase
```

## Simulation Process

### Phase 1: World Generation
```python
# Generate locations
zones = ['downtown', 'residential', 'commercial', 'suburban']
for zone in zones:
    generate_homes(10-20 per zone)
    generate_workplaces(5-15 per zone)
    generate_social_spots(3-8 per zone)
```

### Phase 2: Population Creation
```python
# Generate 150-250 characters
for i in range(num_characters):
    character = generate_character(i)
    assign_home_location()
    assign_work_location()
    apply_archetype_traits()
```

### Phase 3: Relationship Network
```python
# Create realistic relationships
create_romantic_relationships()  # 70% of adults
create_family_relationships()    # 50% have children
create_friendships()            # 1-5 friends per person
create_work_relationships()     # Colleagues at same workplace
```

### Phase 4: Simulation Loop (90 days)
```python
for day in range(90):
    move_characters_to_locations()      # Home/work/social based on time
    generate_interactions()             # Characters in same location
    update_character_states()           # Stress, relationships
    check_for_conflicts()               # High stress + relationships
    check_for_crimes()                  # Risk + opportunity + motive
```

### Phase 5: Case Generation
```python
for crime in unsolved_crimes:
    case = generate_case(crime)
    add_evidence()                      # Expand basic evidence
    add_witnesses()                     # People who were there
    add_red_herrings()                  # Misleading but innocent clues
    define_missing_pieces()             # What player must discover
    create_solution_path()              # Logical steps to solve
```

## Crime Generation Conditions

A character becomes at risk of criminal behavior when:
```python
stress_level > 0.8 AND
impulsiveness > 0.6 AND 
self_control < 0.4 AND
opportunity_exists == TRUE AND
(has_motive == TRUE)
```

**Motive Sources:**
- Financial desperation (low income + high stress)
- Revenge (high-severity conflicts)
- Emotional stress (relationship problems, work pressure)

**Opportunity Sources:**
- Low-security locations
- Characters with access to valuable targets
- Times when security is minimal

## Case Structure

Each generated detective case contains:

### Evidence
- **Physical**: Weapons, fingerprints, broken items
- **Digital**: Security footage, transaction records, communications
- **Testimonial**: Witness accounts, 911 calls
- **Circumstantial**: Motive, opportunity, behavior changes

### Witnesses
- **Direct Witness**: Saw the crime happen
- **Circumstantial**: Saw relevant events before/after
- **Hearsay**: Heard about it from someone else
- **Reliability**: 0.0 to 1.0 based on witness traits
- **Bias**: May favor certain suspects

### Red Herrings
- Innocent people who were at the scene
- Misleading timeline evidence
- False witness testimony
- Coincidental but suspicious circumstances

### Missing Pieces
- Perpetrator identification
- Crime method
- Motive
- Timeline reconstruction
- Victim-perpetrator relationship

### Solution Path
Logical steps to solve the case:
1. Review initial evidence
2. Cross-reference with suspects
3. Interview reliable witnesses
4. Investigate motive
5. Verify method
6. Connect all evidence

## Usage Examples

### Basic Simulation
```python
# Create and run simulation
sim = CrimeSimulation("my_simulation.db")
engine = SimulationEngine(sim)
engine.run_simulation(num_days=60)

# Generate cases
generator = CaseGenerator("my_simulation.db")
cases = generator.generate_all_cases()

# Save best case
best_case = max(cases, key=lambda c: len(c.evidence))
generator.save_case_to_file(best_case, "detective_case_1.json")
```

### Debugging
```python
# Check simulation state
cursor = sim.conn.cursor()
cursor.execute("SELECT * FROM characters WHERE stress_level > 0.8")
high_stress_chars = cursor.fetchall()

cursor.execute("SELECT * FROM events ORDER BY created_at DESC")
recent_events = cursor.fetchall()

cursor.execute("SELECT crime_type, COUNT(*) FROM crimes GROUP BY crime_type")
crime_breakdown = cursor.fetchall()
```

### Customization

#### Adjust Crime Rates
```python
# In check_for_crimes method
if (char.is_high_risk() and 
    self.has_crime_opportunity(char) and 
    self.has_crime_motive(char) and
    random.random() < 0.1):  # 10% chance instead of default
    self.generate_crime(char, date)
```

#### Change Personality Distribution
```python
# In generate_character method
archetype = random.choices(
    ["perfectionist", "rebel", "carer", "achiever", "explorer", "peacekeeper"],
    weights=[0.15, 0.15, 0.2, 0.2, 0.15, 0.15]  # More achievers and carers
)[0]
```

#### Modify Relationship Formation
```python
# In create_romantic_relationships
if random.random() < 0.9:  # 90% instead of 70% have partners
    create_romantic_relationship(char)
```

## Performance Considerations

### Scalability
- **Characters**: 150-250 works well on standard hardware
- **Locations**: 50-100 locations provide good coverage
- **Simulation Days**: 60-90 days for good crime generation
- **Database**: SQLite handles this scale easily

### Optimization
- **Spatial Indexing**: Not needed for this scale
- **Lazy Loading**: Characters only processed when relevant
- **Batch Processing**: Process daily updates in batches
- **Memory Management**: Keep only active simulation in memory

### Monitoring
```python
# Track simulation health
def log_simulation_stats():
    cursor.execute("SELECT COUNT(*) FROM characters WHERE stress_level > 0.8")
    high_stress = cursor.fetchone()[0]
    
    cursor.execute("SELECT COUNT(*) FROM active_conflicts")
    conflicts = cursor.fetchone()[0]
    
    print(f"High stress characters: {high_stress}")
    print(f"Active conflicts: {conflicts}")
```

## Quality Assurance

### Ensuring Good Cases
1. **Minimum Evidence**: Only use crimes with 3+ evidence pieces
2. **Witness Diversity**: Ensure multiple witness perspectives
3. **Balanced Difficulty**: Mix of obvious and subtle clues
4. **Logical Consistency**: All evidence connects back to simulation

### Validation
```python
def validate_case(case):
    # Check evidence connects to real characters
    for evidence in case.evidence:
        if evidence.connects_to:
            for char_id in evidence.connects_to:
                assert character_exists(char_id)
    
    # Check witness reliability is reasonable
    for witness in case.witnesses:
        assert 0.3 <= witness.reliability <= 0.9
    
    # Check solution path is logical
    assert len(case.solution_path) >= 4
```

## Integration with Game

### JSON Output Format
Generated cases are saved as JSON for easy game integration:
```json
{
  "case_id": 1,
  "title": "Mystery at Central Bank",
  "crime_type": "fraud",
  "evidence": [
    {
      "type": "digital",
      "description": "Financial transaction records",
      "reliability": 0.9,
      "connects_to": ["45"]
    }
  ],
  "witnesses": [...],
  "missing_pieces": [...],
  "solution_path": [...]
}
```

### Game Mechanics Integration
- **Evidence Matrix**: Display all evidence in player's knowledge base
- **Relationship Tracking**: Show connections between characters
- **Deduction System**: Allow player to connect evidence logically
- **Red Herring Handling**: Mark misleading evidence appropriately

## Advantages of This Approach

1. **Authentic**: Crimes emerge naturally from human psychology
2. **Unique**: Each simulation generates different cases
3. **Debuggable**: SQLite allows inspection of simulation state
4. **Scalable**: Can adjust population size and complexity
5. **Consistent**: All evidence connects back to simulated reality
6. **Replayable**: Same setup can generate different outcomes

## Next Steps

1. **Run Demo**: Execute `demo_complete_system.py` to see the system in action
2. **Tune Parameters**: Adjust crime rates, personality distributions, relationship formation
3. **Add Complexity**: Implement more crime types, advanced personality traits
4. **Game Integration**: Connect case output to your detective game interface
5. **Performance Testing**: Scale up population and measure performance

This system provides a solid foundation for generating infinite unique detective cases that feel organic and authentic, moving beyond template-based generation to true emergent storytelling.