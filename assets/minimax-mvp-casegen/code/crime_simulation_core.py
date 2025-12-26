# Crime Simulation - Core Classes

import sqlite3
import random
from datetime import datetime, timedelta
from typing import List, Dict, Optional
import json

class CrimeSimulation:
    def __init__(self, db_path: str):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)
        self.setup_database()
        
    def setup_database(self):
        """Create all tables if they don't exist"""
        cursor = self.conn.cursor()
        
        # Characters table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS characters (
                id INTEGER PRIMARY KEY,
                name TEXT NOT NULL,
                age INTEGER,
                gender TEXT,
                occupation TEXT,
                income_level TEXT,
                stress_level REAL DEFAULT 0.5,
                impulsiveness REAL DEFAULT 0.5,
                empathy REAL DEFAULT 0.5,
                self_control REAL DEFAULT 0.5,
                location_id INTEGER,
                home_location_id INTEGER,
                work_location_id INTEGER
            )
        ''')
        
        # Locations table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS locations (
                id INTEGER PRIMARY KEY,
                name TEXT NOT NULL,
                type TEXT,
                zone TEXT,
                capacity INTEGER,
                security_level TEXT
            )
        ''')
        
        # Relationships table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS relationships (
                id INTEGER PRIMARY KEY,
                char1_id INTEGER,
                char2_id INTEGER,
                relationship_type TEXT,
                strength REAL DEFAULT 0.5,
                trust_level REAL DEFAULT 0.5,
                last_interaction_date TEXT,
                UNIQUE(char1_id, char2_id),
                FOREIGN KEY(char1_id) REFERENCES characters(id),
                FOREIGN KEY(char2_id) REFERENCES characters(id)
            )
        ''')
        
        # Events table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS events (
                id INTEGER PRIMARY KEY,
                date TEXT,
                event_type TEXT,
                description TEXT,
                participants TEXT,
                location_id INTEGER,
                emotional_impact REAL,
                stress_change REAL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Conflicts table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS conflicts (
                id INTEGER PRIMARY KEY,
                date_started TEXT,
                participants TEXT,
                conflict_type TEXT,
                description TEXT,
                severity REAL,
                escalation_level REAL DEFAULT 0.0,
                resolution TEXT
            )
        ''')
        
        # Crimes table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS crimes (
                id INTEGER PRIMARY KEY,
                date TEXT,
                crime_type TEXT,
                perpetrator_id INTEGER,
                victim_id INTEGER,
                location_id INTEGER,
                description TEXT,
                evidence TEXT,
                motive TEXT,
                method TEXT,
                solved BOOLEAN DEFAULT FALSE
            )
        ''')
        
        self.conn.commit()

class Character:
    def __init__(self, id: int, name: str, age: int, gender: str, occupation: str, 
                 income_level: str, archetype: str):
        self.id = id
        self.name = name
        self.age = age
        self.gender = gender
        self.occupation = occupation
        self.income_level = income_level
        self.archetype = archetype
        
        # Personality traits (0.0 to 1.0)
        self.stress_level = 0.3
        self.impulsiveness = 0.5
        self.empathy = 0.5
        self.self_control = 0.5
        
        # Current state
        self.location_id = None
        self.home_location_id = None
        self.work_location_id = None
        
        # Relationships
        self.relationships = {}  # {character_id: relationship_strength}
        
    def update_stress(self, change: float):
        """Update stress level, clamping between 0.0 and 1.0"""
        self.stress_level = max(0.0, min(1.0, self.stress_level + change))
        
    def is_high_risk(self) -> bool:
        """Check if character is at risk of criminal behavior"""
        return (self.stress_level > 0.6 and  # Lowered from 0.8
                self.impulsiveness > 0.4 and  # Lowered from 0.6
                self.self_control < 0.5)  # Raised from 0.4
    
    def add_relationship(self, other_character, relationship_type: str, strength: float = 0.5):
        """Add or update relationship with another character"""
        self.relationships[other_character.id] = {
            'type': relationship_type,
            'strength': strength,
            'last_interaction': datetime.now()
        }

class Location:
    def __init__(self, id: int, name: str, location_type: str, zone: str):
        self.id = id
        self.name = name
        self.type = location_type
        self.zone = zone
        self.capacity = 50
        self.security_level = "medium"
        self.current_occupants = []
        
    def add_character(self, character: Character):
        """Add character to location if capacity allows"""
        if len(self.current_occupants) < self.capacity:
            self.current_occupants.append(character.id)
            character.location_id = self.id
            return True
        return False
    
    def remove_character(self, character: Character):
        """Remove character from location"""
        if character.id in self.current_occupants:
            self.current_occupants.remove(character.id)
            character.location_id = None

class SimulationEngine:
    def __init__(self, simulation: CrimeSimulation):
        self.sim = simulation
        self.characters = {}
        self.locations = {}
        self.current_date = datetime(2024, 1, 1)
        self.active_conflicts = []
        
    def generate_character_name(self) -> str:
        """Generate a random character name"""
        first_names = ["Sarah", "Mike", "Jennifer", "David", "Lisa", "James", "Maria", "Robert"]
        last_names = ["Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis"]
        return f"{random.choice(first_names)} {random.choice(last_names)}"
    
    def generate_character(self, char_id: int) -> Character:
        """Generate a new character with random attributes"""
        name = self.generate_character_name()
        age = random.randint(18, 70)
        gender = random.choice(["male", "female", "non_binary"])
        occupation = random.choice(["teacher", "engineer", "retail_worker", "manager", "student", "retired"])
        income_level = random.choice(["low", "middle", "high"])
        archetype = random.choice(["perfectionist", "rebel", "carer", "achiever", "explorer", "peacekeeper"])
        
        character = Character(char_id, name, age, gender, occupation, income_level, archetype)
        
        # Set personality traits based on archetype
        self.apply_archetype_traits(character)
        
        return character
    
    def apply_archetype_traits(self, character: Character):
        """Apply personality traits based on character archetype"""
        traits = {
            "perfectionist": {"impulsiveness": 0.2, "self_control": 0.9, "stress_tendency": 0.8},
            "rebel": {"impulsiveness": 0.8, "self_control": 0.3, "stress_tendency": 0.6},
            "carer": {"empathy": 0.9, "self_control": 0.7, "stress_tendency": 0.4},
            "achiever": {"impulsiveness": 0.6, "self_control": 0.8, "stress_tendency": 0.7},
            "explorer": {"impulsiveness": 0.7, "self_control": 0.4, "stress_tendency": 0.3},
            "peacekeeper": {"empathy": 0.8, "self_control": 0.9, "stress_tendency": 0.2}
        }
        
        if character.archetype in traits:
            archetype_traits = traits[character.archetype]
            character.impulsiveness = archetype_traits.get("impulsiveness", 0.5)
            character.self_control = archetype_traits.get("self_control", 0.5)
            # Note: stress_tendency would affect how quickly they accumulate stress
    
    def generate_location(self, loc_id: int, location_type: str, zone: str) -> Location:
        """Generate a new location"""
        names = {
            "home": ["Oak Street Apartments", "Pine View Condos", "Maple Heights"],
            "work": ["Tech Solutions Inc", "Downtown Bank", "Central Hospital", "Riverside Mall"],
            "social": ["The Corner Cafe", "Community Center", "Central Park", "Local Bar"],
            "public": ["City Library", "Main Bus Station", "Shopping District", "University Campus"]
        }
        
        name = random.choice(names.get(location_type, ["Generic Location"]))
        location = Location(loc_id, name, location_type, zone)
        
        # Set capacity and security based on type
        if location_type == "home":
            location.capacity = random.randint(20, 100)
            location.security_level = random.choice(["low", "medium"])
        elif location_type == "work":
            location.capacity = random.randint(50, 200)
            location.security_level = "medium"
        elif location_type == "social":
            location.capacity = random.randint(30, 150)
            location.security_level = random.choice(["low", "medium", "high"])
        
        return location
    
    def create_relationships(self):
        """Create initial relationships between characters"""
        # Family relationships
        for char in list(self.characters.values()):
            if char.age > 25:  # Adults might have families
                if random.random() < 0.7:  # 70% chance of having a partner
                    self.create_romantic_relationship(char)
                
                if random.random() < 0.5:  # 50% chance of having children
                    self.create_family_relationships(char)
        
        # Friendships and work relationships
        self.create_friendships()
        self.create_work_relationships()
    
    def create_romantic_relationship(self, char: Character):
        """Create a romantic relationship"""
        available_partners = [c for c in self.characters.values() 
                            if c.id != char.id and 18 <= c.age <= 65]
        if available_partners:
            partner = random.choice(available_partners)
            char.add_relationship(partner, "romantic", random.uniform(0.6, 0.9))
            partner.add_relationship(char, "romantic", random.uniform(0.6, 0.9))
    
    def create_family_relationships(self, parent: Character):
        """Create family relationships (parent-child)"""
        child_ages = random.randint(5, 25)
        for _ in range(random.randint(0, 2)):  # 0-2 children
            child_id = max(self.characters.keys()) + 1
            child = self.generate_character(child_id)
            child.age = random.randint(5, 25)
            child.parent_id = parent.id
            
            parent.add_relationship(child, "parent", random.uniform(0.8, 1.0))
            child.add_relationship(parent, "child", random.uniform(0.8, 1.0))
            
            self.characters[child_id] = child
    
    def create_friendships(self):
        """Create friendship relationships"""
        for char in self.characters.values():
            possible_friends = [c for c in self.characters.values() 
                              if c.id != char.id and 
                              abs(c.age - char.age) <= 10 and
                              c.id not in char.relationships]
            
            num_friends = random.randint(1, 5)
            for _ in range(num_friends):
                if possible_friends:
                    friend = random.choice(possible_friends)
                    char.add_relationship(friend, "friend", random.uniform(0.4, 0.8))
                    friend.add_relationship(char, "friend", random.uniform(0.4, 0.8))
    
    def create_work_relationships(self):
        """Create work-related relationships"""
        # Group characters by work location
        work_groups = {}
        for char in self.characters.values():
            if char.work_location_id and char.occupation != "retired" and char.occupation != "student":
                if char.work_location_id not in work_groups:
                    work_groups[char.work_location_id] = []
                work_groups[char.work_location_id].append(char)
        
        # Create relationships within work groups
        for location_id, workers in work_groups.items():
            for char in workers:
                colleagues = [w for w in workers if w.id != char.id]
                for colleague in random.sample(colleagues, min(len(colleagues), 3)):
                    char.add_relationship(colleague, "colleague", random.uniform(0.3, 0.7))
    
    def simulate_day(self, date: datetime):
        """Simulate one day of interactions"""
        # Move characters to appropriate locations
        self.move_characters_to_locations()
        
        # Generate interactions between characters in same location
        self.generate_interactions()
        
        # Update stress levels and check for conflicts
        self.update_character_states()
        
        # Check for crime conditions
        self.check_for_crimes(date)
    
    def move_characters_to_locations(self):
        """Move characters to their home/work locations based on time"""
        for char in self.characters.values():
            # During work hours, move to work location
            if 9 <= self.current_date.hour <= 17 and char.work_location_id:
                char.location_id = char.work_location_id
            # Otherwise, move to home or random social location
            elif char.home_location_id and random.random() < 0.7:
                char.location_id = char.home_location_id
            else:
                # Random social location
                social_locations = [loc for loc in self.locations.values() 
                                  if loc.type == "social"]
                if social_locations:
                    char.location_id = random.choice(social_locations).id
    
    def generate_interactions(self):
        """Generate interactions between characters in same location"""
        location_groups = {}
        for char in self.characters.values():
            if char.location_id:
                if char.location_id not in location_groups:
                    location_groups[char.location_id] = []
                location_groups[char.location_id].append(char)
        
        for location_id, chars in location_groups.items():
            if len(chars) > 1:
                # Generate random interactions between characters in same location
                for i in range(len(chars)):
                    for j in range(i + 1, len(chars)):
                        if random.random() < 0.3:  # 30% chance of interaction
                            self.create_interaction(chars[i], chars[j])
    
    def create_interaction(self, char1: Character, char2: Character):
        """Create an interaction between two characters"""
        # Check existing relationship
        relationship = char1.relationships.get(char2.id)
        
        if relationship:
            # Existing relationship - can strengthen or weaken
            if random.random() < 0.6:  # 60% chance of positive interaction
                relationship['strength'] = min(1.0, relationship['strength'] + 0.1)
                stress_change = -0.1  # Positive interaction reduces stress
            else:
                relationship['strength'] = max(0.0, relationship['strength'] - 0.1)
                stress_change = 0.1  # Negative interaction increases stress
        else:
            # No existing relationship - create new one
            char1.add_relationship(char2, "acquaintance", 0.3)
            char2.add_relationship(char1, "acquaintance", 0.3)
            stress_change = 0.0
        
        # Apply stress changes
        char1.update_stress(stress_change)
        char2.update_stress(stress_change)
    
    def update_character_states(self):
        """Update character stress levels and check for conflicts"""
        for char in self.characters.values():
            # Natural stress decay
            if char.stress_level > 0.2:
                char.update_stress(-0.05)
            
            # Check for high-stress characters
            if char.stress_level > 0.8:
                self.check_for_conflict_escalation(char)
    
    def check_for_conflict_escalation(self, character: Character):
        """Check if high stress character might escalate to conflict"""
        # Find other high-stress characters they have relationships with
        potential_conflicts = []
        for other_char_id, relationship in character.relationships.items():
            other_char = self.characters.get(other_char_id)
            if other_char and other_char.stress_level > 0.6:
                potential_conflicts.append(other_char)
        
        if potential_conflicts and random.random() < 0.3:  # 30% chance
            conflict_partner = random.choice(potential_conflicts)
            self.create_conflict(character, conflict_partner)
    
    def create_conflict(self, char1: Character, char2: Character):
        """Create a new conflict between two characters"""
        conflict_types = ["resource", "romantic", "professional", "personal"]
        conflict_type = random.choice(conflict_types)
        
        # Add to active conflicts list
        conflict = {
            'char1_id': char1.id,
            'char2_id': char2.id,
            'type': conflict_type,
            'severity': random.uniform(0.3, 0.7),
            'escalation_level': 0.0,
            'date_started': self.current_date.isoformat()
        }
        
        self.active_conflicts.append(conflict)
        
        # Increase stress for both characters
        char1.update_stress(0.2)
        char2.update_stress(0.2)
    
    def check_for_crimes(self, date: datetime):
        """Check for characters that might commit crimes"""
        for char in self.characters.values():
            if char.is_high_risk():
                # Check if there's an opportunity and a reason
                if self.has_crime_opportunity(char) and self.has_crime_motive(char):
                    # Add some randomness for demo purposes
                    if random.random() < 0.3:  # 30% chance when all conditions met
                        self.generate_crime(char, date)
    
    def has_crime_opportunity(self, character: Character) -> bool:
        """Check if character has opportunity to commit crime"""
        # Simple check - if they're in a location with valuable items
        if not character.location_id:
            return False
        
        location = self.locations.get(character.location_id)
        if not location:
            return False
        
        # Higher opportunity in low-security locations (more generous for demo)
        if location.security_level == "low" and random.random() < 0.6:
            return True
        elif location.security_level == "medium" and random.random() < 0.4:
            return True
        elif location.security_level == "high" and random.random() < 0.2:
            return True
        
        return False
    
    def has_crime_motive(self, character: Character) -> bool:
        """Check if character has motive for crime"""
        # Financial stress (more generous)
        if character.income_level == "low" and character.stress_level > 0.5:  # Lowered threshold
            return random.random() < 0.8  # Increased probability
        
        # Revenge for conflict (more generous)
        for conflict in self.active_conflicts:
            if (conflict['char1_id'] == character.id or conflict['char2_id'] == character.id):
                if conflict['severity'] > 0.4:  # Lowered threshold
                    return random.random() < 0.6  # Increased probability
        
        # Add some random motive generation for demo
        if character.stress_level > 0.7:  # High stress can create motive
            return random.random() < 0.3
        
        return False
    
    def generate_crime(self, perpetrator: Character, date: datetime):
        """Generate a crime event"""
        crime_types = {
            'theft': 0.4,
            'assault': 0.3,
            'fraud': 0.2,
            'vandalism': 0.1
        }
        
        crime_type = random.choices(list(crime_types.keys()), 
                                  weights=list(crime_types.values()))[0]
        
        # Determine victim
        victim = self.select_crime_victim(perpetrator)
        
        # Generate crime details
        crime = {
            'type': crime_type,
            'perpetrator_id': perpetrator.id,
            'victim_id': victim.id if victim else None,
            'location_id': perpetrator.location_id,
            'date': date.isoformat(),
            'description': self.generate_crime_description(crime_type, perpetrator, victim),
            'evidence': json.dumps(self.generate_evidence(crime_type, perpetrator, victim)),
            'motive': self.generate_crime_motive(crime_type, perpetrator),
            'method': self.generate_crime_method(crime_type, perpetrator)
        }
        
        # Store in database
        self.store_crime(crime)
        
        # Reset perpetrator stress (crime might provide temporary relief)
        perpetrator.update_stress(-0.3)
    
    def select_crime_victim(self, perpetrator: Character):
        """Select a victim for the crime"""
        # Prefer people they have conflicts with
        conflict_partners = []
        for conflict in self.active_conflicts:
            if conflict['char1_id'] == perpetrator.id:
                conflict_partners.append(self.characters.get(conflict['char2_id']))
            elif conflict['char2_id'] == perpetrator.id:
                conflict_partners.append(self.characters.get(conflict['char1_id']))
        
        if conflict_partners and random.random() < 0.7:
            return random.choice([c for c in conflict_partners if c])
        
        # Otherwise random victim
        possible_victims = [c for c in self.characters.values() if c.id != perpetrator.id]
        return random.choice(possible_victims) if possible_victims else None
    
    def generate_crime_description(self, crime_type: str, perpetrator: Character, victim: Character) -> str:
        """Generate a description of the crime"""
        descriptions = {
            'theft': f"{perpetrator.name} stole valuable items from {victim.name if victim else 'a location'}",
            'assault': f"{perpetrator.name} physically assaulted {victim.name if victim else 'someone'}",
            'fraud': f"{perpetrator.name} committed financial fraud against {victim.name if victim else 'the system'}",
            'vandalism': f"{perpetrator.name} damaged property belonging to {victim.name if victim else 'unknown owners'}"
        }
        return descriptions.get(crime_type, f"{perpetrator.name} committed {crime_type}")
    
    def generate_evidence(self, crime_type: str, perpetrator: Character, victim: Character) -> List[Dict]:
        """Generate evidence for the crime"""
        evidence = []
        
        # Always generate some basic evidence
        if perpetrator.location_id:
            evidence.append({
                'type': 'location',
                'description': f"Security camera footage from {self.locations[perpetrator.location_id].name}",
                'reliability': 0.8
            })
        
        # Type-specific evidence
        if crime_type in ['theft', 'assault']:
            evidence.append({
                'type': 'physical',
                'description': f"Fingerprints found at crime scene",
                'reliability': 0.9
            })
        
        if victim:
            evidence.append({
                'type': 'testimonial',
                'description': f"{victim.name}'s account of events",
                'reliability': 0.7
            })
        
        return evidence
    
    def generate_crime_motive(self, crime_type: str, perpetrator: Character) -> str:
        """Generate motive for the crime"""
        motives = {
            'theft': "financial desperation" if perpetrator.income_level == "low" else "greed",
            'assault': "revenge" if perpetrator.stress_level > 0.8 else "impulsive violence",
            'fraud': "financial gain",
            'vandalism': "anger and frustration"
        }
        return motives.get(crime_type, "unknown")
    
    def generate_crime_method(self, crime_type: str, perpetrator: Character) -> str:
        """Generate method used in the crime"""
        methods = {
            'theft': "forced entry" if perpetrator.impulsiveness > 0.6 else "planned break-in",
            'assault': "spontaneous attack" if perpetrator.impulsiveness > 0.7 else "premeditated violence",
            'fraud': "forged documents",
            'vandalism': "random property damage"
        }
        return methods.get(crime_type, "unknown method")
    
    def store_crime(self, crime: Dict):
        """Store crime in database"""
        cursor = self.sim.conn.cursor()
        cursor.execute('''
            INSERT INTO crimes (date, crime_type, perpetrator_id, victim_id, 
                              location_id, description, evidence, motive, method)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            crime['date'], crime['type'], crime['perpetrator_id'], crime['victim_id'],
            crime['location_id'], crime['description'], crime['evidence'],
            crime['motive'], crime['method']
        ))
        self.sim.conn.commit()
    
    def run_simulation(self, num_days: int = 90):
        """Run the complete simulation"""
        print("Starting crime simulation...")
        
        # Phase 1: Generate world
        self.generate_world()
        
        # Phase 2: Generate population
        self.generate_population()
        
        # Phase 3: Create initial relationships
        self.create_relationships()
        
        # Phase 4: Run simulation
        for day in range(num_days):
            self.simulate_day(self.current_date)
            self.current_date += timedelta(days=1)
            
            if day % 10 == 0:
                print(f"Day {day}: {len(self.active_conflicts)} active conflicts, "
                      f"{sum(1 for c in self.characters.values() if c.stress_level > 0.8)} high-stress characters")
        
        print(f"Simulation complete! Generated crimes: {self.get_crime_count()}")
    
    def generate_world(self):
        """Generate the initial world (locations)"""
        print("Generating world...")
        
        # Generate locations
        location_id = 1
        zones = ['downtown', 'residential', 'commercial', 'suburban']
        
        for zone in zones:
            # Add homes
            for _ in range(random.randint(10, 20)):
                location = self.generate_location(location_id, "home", zone)
                self.locations[location_id] = location
                location_id += 1
            
            # Add workplaces
            for _ in range(random.randint(5, 15)):
                location = self.generate_location(location_id, "work", zone)
                self.locations[location_id] = location
                location_id += 1
            
            # Add social spots
            for _ in range(random.randint(3, 8)):
                location = self.generate_location(location_id, "social", zone)
                self.locations[location_id] = location
                location_id += 1
        
        # Add some public spaces
        for _ in range(random.randint(5, 10)):
            location = self.generate_location(location_id, "public", "mixed")
            self.locations[location_id] = location
            location_id += 1
    
    def generate_population(self):
        """Generate the initial population"""
        print("Generating population...")
        
        num_characters = random.randint(150, 250)
        
        for i in range(1, num_characters + 1):
            character = self.generate_character(i)
            
            # Assign home location
            home_locations = [loc for loc in self.locations.values() 
                            if loc.type == "home"]
            if home_locations:
                character.home_location_id = random.choice(home_locations).id
            
            # Assign work location (for working age adults)
            if 22 <= character.age <= 65 and character.occupation not in ["retired", "student"]:
                work_locations = [loc for loc in self.locations.values() 
                                if loc.type == "work"]
                if work_locations:
                    character.work_location_id = random.choice(work_locations).id
            
            self.characters[i] = character
    
    def get_crime_count(self) -> int:
        """Get the number of crimes generated"""
        cursor = self.sim.conn.cursor()
        cursor.execute("SELECT COUNT(*) FROM crimes")
        return cursor.fetchone()[0]

# Example usage
if __name__ == "__main__":
    # Create simulation
    sim = CrimeSimulation("crime_simulation.db")
    engine = SimulationEngine(sim)
    
    # Run simulation
    engine.run_simulation(num_days=60)
    
    # Print some results
    print(f"\nGenerated {engine.get_crime_count()} crimes")
    
    # Show some high-stress characters
    high_stress_chars = [c for c in engine.characters.values() if c.stress_level > 0.8]
    print(f"Characters with high stress: {len(high_stress_chars)}")
    
    for char in high_stress_chars[:5]:  # Show first 5
        print(f"  {char.name}: stress={char.stress_level:.2f}, archetype={char.archetype}")