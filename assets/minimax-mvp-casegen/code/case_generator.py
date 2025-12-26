# Crime Case Generator - Convert Simulated Crimes to Detective Cases

import sqlite3
import random
import json
from typing import List, Dict, Optional
from dataclasses import dataclass

@dataclass
class Evidence:
    type: str  # physical, digital, testimonial, circumstantial
    description: str
    reliability: float  # 0.0 to 1.0
    connects_to: List[str]  # character IDs this evidence connects to
    
@dataclass
class Witness:
    character_id: int
    name: str
    description: str
    reliability: float
    knowledge_level: str  # direct_witness, circumstantial, hearsay
    bias_toward: Optional[str]  # character ID they favor
    
@dataclass
class DetectiveCase:
    case_id: int
    title: str
    crime_type: str
    description: str
    victim_name: str
    perpetrator_name: str
    location_name: str
    evidence: List[Evidence]
    witnesses: List[Witness]
    red_herrings: List[Dict]
    missing_pieces: List[str]  # Key unknowns player must discover
    solution_path: List[str]  # Logical steps to solve case

class CaseGenerator:
    def __init__(self, db_path: str):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)
        
    def get_unsolved_crimes(self) -> List[Dict]:
        """Get all unsolved crimes from simulation"""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT c.*, p.name as perpetrator_name, v.name as victim_name, 
                   l.name as location_name
            FROM crimes c
            LEFT JOIN characters p ON c.perpetrator_id = p.id
            LEFT JOIN characters v ON c.victim_id = v.id
            LEFT JOIN locations l ON c.location_id = l.id
            WHERE c.solved = FALSE
        """)
        
        crimes = []
        for row in cursor.fetchall():
            crime = {
                'id': row[0],
                'date': row[1],
                'crime_type': row[2],
                'perpetrator_id': row[3],
                'victim_id': row[4],
                'location_id': row[5],
                'description': row[6],
                'evidence': json.loads(row[7]) if row[7] else [],
                'motive': row[8],
                'method': row[9],
                'perpetrator_name': row[10],
                'victim_name': row[11],
                'location_name': row[12]
            }
            crimes.append(crime)
        
        return crimes
    
    def select_best_case(self, crimes: List[Dict]) -> Optional[Dict]:
        """Select the most interesting crime for a detective case"""
        if not crimes:
            return None
        
        # Score crimes based on interesting factors
        scored_crimes = []
        for crime in crimes:
            score = 0
            
            # Bonus for crimes with victims (more personal)
            if crime['victim_id']:
                score += 10
            
            # Bonus for complex crimes
            if crime['crime_type'] in ['fraud', 'assault']:
                score += 5
            
            # Bonus for interesting locations
            if any(zone in crime['location_name'].lower() for zone in ['bank', 'hospital', 'school']):
                score += 3
            
            # Bonus for multiple evidence pieces
            if len(crime['evidence']) > 2:
                score += 2
            
            scored_crimes.append((score, crime))
        
        # Return the highest scoring crime
        scored_crimes.sort(reverse=True)
        return scored_crimes[0][1]
    
    def generate_case_title(self, crime: Dict) -> str:
        """Generate an interesting title for the detective case"""
        location_name = crime['location_name'] or "Unknown Location"
        location_parts = location_name.split()
        if len(location_parts) > 2:
            location = ' '.join(location_parts[-2:])  # Last 2 words
        else:
            location = location_name
        
        crime_titles = {
            'theft': [f"Mystery at {location}", f"Missing from {location}", f"The {location} Heist"],
            'assault': [f"Trouble at {location}", f"Violence at {location}", f"Attack at {location}"],
            'fraud': [f"Scandal at {location}", f"Financial Deception at {location}", f"Conspiracy at {location}"],
            'vandalism': [f"Damage at {location}", f"Destruction at {location}", f"Vandalism at {location}"]
        }
        
        titles = crime_titles.get(crime['crime_type'], [f"Mystery Case"])
        return random.choice(titles)
    
    def expand_evidence(self, crime: Dict) -> List[Evidence]:
        """Generate additional evidence based on the crime"""
        evidence = []
        
        # Convert existing evidence
        for evidence_item in crime['evidence']:
            evidence.append(Evidence(
                type=evidence_item['type'],
                description=evidence_item['description'],
                reliability=evidence_item['reliability'],
                connects_to=[str(crime['perpetrator_id']), str(crime['victim_id'])]
            ))
        
        # Generate additional evidence based on crime type and method
        if crime['crime_type'] == 'theft':
            evidence.extend([
                Evidence("physical", "Broken window or lock at entry point", 0.9, [str(crime['perpetrator_id'])]),
                Evidence("digital", f"Security footage showing suspicious activity around {crime['location_name']}", 0.8, [str(crime['perpetrator_id'])]),
                Evidence("circumstantial", "Missing items inventory list", 0.7, [])
            ])
        
        elif crime['crime_type'] == 'assault':
            evidence.extend([
                Evidence("physical", "Weapon or object used in attack", 0.9, [str(crime['perpetrator_id'])]),
                Evidence("testimonial", "911 call audio recording", 0.8, []),
                Evidence("circumstantial", "Medical examination report", 0.9, [str(crime['victim_id'])])
            ])
        
        elif crime['crime_type'] == 'fraud':
            evidence.extend([
                Evidence("digital", "Financial transaction records", 0.9, [str(crime['perpetrator_id'])]),
                Evidence("physical", "Forged documents or signatures", 0.8, [str(crime['perpetrator_id'])]),
                Evidence("circumstantial", "Bank security footage", 0.7, [str(crime['perpetrator_id'])])
            ])
        
        elif crime['crime_type'] == 'vandalism':
            evidence.extend([
                Evidence("physical", "Paint cans or tools used for vandalism", 0.8, [str(crime['perpetrator_id'])]),
                Evidence("circumstantial", "Surveillance camera malfunction during incident", 0.6, []),
                Evidence("testimonial", "Witness report of suspicious person nearby", 0.5, [str(crime['perpetrator_id'])])
            ])
        
        return evidence
    
    def generate_witnesses(self, crime: Dict, location_id: int) -> List[Witness]:
        """Generate witnesses for the crime"""
        witnesses = []
        
        # Get characters who were at the same location around the crime time
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT DISTINCT c.id, c.name, c.age, c.gender, c.occupation
            FROM characters c
            JOIN events e ON e.participants LIKE '%' || c.id || '%'
            WHERE e.location_id = ? AND e.date <= ? 
            AND e.created_at >= datetime(?, '-1 day')
            AND c.id != ? AND c.id != ?
        """, (location_id, crime['date'], crime['date'], 
              crime['perpetrator_id'], crime['victim_id'] or 0))
        
        possible_witnesses = cursor.fetchall()
        
        # Create 2-4 witnesses
        num_witnesses = random.randint(2, 4)
        selected_witnesses = random.sample(possible_witnesses, 
                                         min(num_witnesses, len(possible_witnesses)))
        
        for witness_data in selected_witnesses:
            witness_id, name, age, gender, occupation = witness_data
            
            # Determine witness reliability based on their traits
            reliability = random.uniform(0.3, 0.9)
            
            # Determine what they know
            knowledge_levels = ['direct_witness', 'circumstantial', 'hearsay']
            knowledge = random.choices(knowledge_levels, weights=[0.3, 0.5, 0.2])[0]
            
            # Generate witness description and potential bias
            bias_toward = None
            if random.random() < 0.3:  # 30% chance of bias
                if random.random() < 0.5:
                    bias_toward = str(crime['perpetrator_id'])  # Favor perpetrator
                else:
                    bias_toward = str(crime['victim_id'])  # Favor victim
            
            witness = Witness(
                character_id=witness_id,
                name=name,
                description=f"{occupation} who was in the area",
                reliability=reliability,
                knowledge_level=knowledge,
                bias_toward=bias_toward
            )
            
            witnesses.append(witness)
        
        return witnesses
    
    def generate_red_herrings(self, crime: Dict) -> List[Dict]:
        """Generate misleading but innocent evidence"""
        red_herrings = []
        
        # Random innocent person who was in the area
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT c.id, c.name, c.occupation
            FROM characters c
            WHERE c.id != ? AND c.id != ?
            ORDER BY RANDOM()
            LIMIT 1
        """, (crime['perpetrator_id'], crime['victim_id'] or 0))
        
        innocent_person = cursor.fetchone()
        if innocent_person:
            person_id, name, occupation = innocent_person
            
            red_herrings.append({
                'type': 'evidence',
                'description': f"Fingerprints of {name} (the {occupation}) found at scene",
                'reliability': 0.8,
                'explanation': f"{name} was there earlier for legitimate reasons",
                'connects_to': [str(person_id)]
            })
        
        # Confusing timeline evidence
        red_herrings.append({
            'type': 'evidence',
            'description': 'Security camera shows someone entering area 2 hours before incident',
            'reliability': 0.7,
            'explanation': 'Different incident or routine maintenance',
            'connects_to': []
        })
        
        # Misleading witness testimony
        if random.random() < 0.5:
            red_herrings.append({
                'type': 'testimony',
                'description': 'Witness claims to have seen perpetrator with weapon earlier',
                'reliability': 0.6,
                'explanation': 'Witness misremembered or confused with different person',
                'connects_to': [str(crime['perpetrator_id'])]
            })
        
        return red_herrings
    
    def define_missing_pieces(self, crime: Dict) -> List[str]:
        """Define the key unknowns that players must discover"""
        missing_pieces = []
        
        # Always include perpetrator identification
        missing_pieces.append("Who committed the crime?")
        
        # Add missing pieces based on crime type
        if crime['crime_type'] == 'theft':
            missing_pieces.extend([
                "What was stolen?",
                "How did the perpetrator enter the location?",
                "What was the perpetrator's motive?"
            ])
        
        elif crime['crime_type'] == 'assault':
            missing_pieces.extend([
                "What triggered the assault?",
                "Was this planned or spontaneous?",
                "What was the relationship between victim and perpetrator?"
            ])
        
        elif crime['crime_type'] == 'fraud':
            missing_pieces.extend([
                "How was the fraud accomplished?",
                "Who else might be involved?",
                "What was the total amount stolen?"
            ])
        
        elif crime['crime_type'] == 'vandalism':
            missing_pieces.extend([
                "Why was this location targeted?",
                "What tools or materials were used?",
                "Was this random or targeted vandalism?"
            ])
        
        return missing_pieces
    
    def create_solution_path(self, crime: Dict, evidence: List[Evidence], 
                           witnesses: List[Witness]) -> List[str]:
        """Define the logical path to solve the case"""
        solution_steps = []
        
        # Step 1: Establish basic facts
        solution_steps.append("Review initial crime scene evidence")
        
        # Step 2: Analyze evidence connections
        key_evidence = [e for e in evidence if e.reliability > 0.7]
        if key_evidence:
            solution_steps.append("Cross-reference physical evidence with suspects")
        
        # Step 3: Interview witnesses
        reliable_witnesses = [w for w in witnesses if w.reliability > 0.6]
        if reliable_witnesses:
            solution_steps.append("Interview reliable witnesses for corroboration")
        
        # Step 4: Establish motive
        if crime['motive']:
            solution_steps.append(f"Investigate {crime['motive']} as potential motive")
        
        # Step 5: Confirm method
        if crime['method']:
            solution_steps.append(f"Verify {crime['method']} was the method used")
        
        # Step 6: Final identification
        solution_steps.append("Connect all evidence to identify the perpetrator")
        
        return solution_steps
    
    def generate_case(self, crime: Dict) -> DetectiveCase:
        """Generate a complete detective case from a crime"""
        
        # Generate case components
        title = self.generate_case_title(crime)
        evidence = self.expand_evidence(crime)
        witnesses = self.generate_witnesses(crime, crime['location_id'])
        red_herrings = self.generate_red_herrings(crime)
        missing_pieces = self.define_missing_pieces(crime)
        solution_path = self.create_solution_path(crime, evidence, witnesses)
        
        # Create the case
        case = DetectiveCase(
            case_id=crime['id'],
            title=title,
            crime_type=crime['crime_type'],
            description=crime['description'],
            victim_name=crime['victim_name'] or "Unknown Victim",
            perpetrator_name=crime['perpetrator_name'] or "Unknown Perpetrator",
            location_name=crime['location_name'] or "Unknown Location",
            evidence=evidence,
            witnesses=witnesses,
            red_herrings=red_herrings,
            missing_pieces=missing_pieces,
            solution_path=solution_path
        )
        
        return case
    
    def generate_all_cases(self) -> List[DetectiveCase]:
        """Generate detective cases from all unsolved crimes"""
        crimes = self.get_unsolved_crimes()
        cases = []
        
        for crime in crimes:
            case = self.generate_case(crime)
            cases.append(case)
        
        return cases
    
    def save_case_to_file(self, case: DetectiveCase, filename: str):
        """Save a case to a JSON file for game use"""
        case_data = {
            'case_id': case.case_id,
            'title': case.title,
            'crime_type': case.crime_type,
            'description': case.description,
            'victim_name': case.victim_name,
            'perpetrator_name': case.perpetrator_name,
            'location_name': case.location_name,
            'evidence': [
                {
                    'type': e.type,
                    'description': e.description,
                    'reliability': e.reliability,
                    'connects_to': e.connects_to
                }
                for e in case.evidence
            ],
            'witnesses': [
                {
                    'character_id': w.character_id,
                    'name': w.name,
                    'description': w.description,
                    'reliability': w.reliability,
                    'knowledge_level': w.knowledge_level,
                    'bias_toward': w.bias_toward
                }
                for w in case.witnesses
            ],
            'red_herrings': case.red_herrings,
            'missing_pieces': case.missing_pieces,
            'solution_path': case.solution_path
        }
        
        with open(filename, 'w') as f:
            json.dump(case_data, f, indent=2)

# Example usage
if __name__ == "__main__":
    # Generate cases from simulation database
    generator = CaseGenerator("crime_simulation.db")
    
    # Generate all possible cases
    cases = generator.generate_all_cases()
    
    print(f"Generated {len(cases)} detective cases")
    
    # Save the best case
    if cases:
        # Select case with most evidence and witnesses for demo
        best_case = max(cases, key=lambda c: len(c.evidence) + len(c.witnesses))
        
        print(f"\nBest case: {best_case.title}")
        print(f"Crime type: {best_case.crime_type}")
        print(f"Evidence items: {len(best_case.evidence)}")
        print(f"Witnesses: {len(best_case.witnesses)}")
        print(f"Red herrings: {len(best_case.red_herrings)}")
        
        # Save to file
        generator.save_case_to_file(best_case, f"detective_case_{best_case.case_id}.json")
        print(f"\nCase saved to detective_case_{best_case.case_id}.json")
        
        # Print solution path
        print("\nSolution Path:")
        for i, step in enumerate(best_case.solution_path, 1):
            print(f"{i}. {step}")