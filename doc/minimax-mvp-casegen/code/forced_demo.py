#!/usr/bin/env python3
"""
Quick demo that forces some crimes to show case generation works
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from crime_simulation_core import CrimeSimulation, SimulationEngine
from case_generator import CaseGenerator
import json
import random

def force_crime_generation_demo():
    print("=== Forced Crime Generation Demo ===\n")
    
    # Create basic simulation
    sim = CrimeSimulation("demo_forced.db")
    engine = SimulationEngine(sim)
    
    # Generate basic world and population
    engine.generate_world()
    engine.generate_population()
    
    # Manually create some high-stress characters to force crime generation
    high_risk_chars = []
    for char in list(engine.characters.values())[:10]:  # First 10 characters
        char.stress_level = 0.9  # Force high stress
        char.impulsiveness = 0.8  # Make them impulsive
        char.self_control = 0.2   # Low self control
        high_risk_chars.append(char)
    
    print(f"Created {len(high_risk_chars)} high-risk characters")
    
    # Force some crimes to be generated
    from datetime import datetime
    current_date = datetime(2024, 1, 15)
    
    crimes_generated = 0
    for i, char in enumerate(high_risk_chars[:5]):  # Force crimes for 5 characters
        # Simulate opportunity exists
        char.location_id = list(engine.locations.keys())[i % len(engine.locations)]
        
        # Generate crime
        engine.generate_crime(char, current_date)
        crimes_generated += 1
        print(f"Generated crime for: {char.name}")
    
    print(f"\nTotal crimes generated: {crimes_generated}")
    
    # Now generate cases from these forced crimes
    print("\n=== GENERATING DETECTIVE CASES ===")
    generator = CaseGenerator("demo_forced.db")
    cases = generator.generate_all_cases()
    
    print(f"Generated {len(cases)} detective cases")
    
    if cases:
        # Show case details
        for i, case in enumerate(cases, 1):
            print(f"\nCase {i}: {case.title}")
            print(f"Crime: {case.description}")
            print(f"Evidence items: {len(case.evidence)}")
            print(f"Witnesses: {len(case.witnesses)}")
            print(f"Red herrings: {len(case.red_herrings)}")
            
            # Save each case
            generator.save_case_to_file(case, f"demo_case_{i}.json")
            print(f"Saved to: demo_case_{i}.json")
        
        # Show detailed analysis of first case
        print(f"\n=== DETAILED ANALYSIS: {cases[0].title} ===")
        case = cases[0]
        
        print(f"Crime Type: {case.crime_type}")
        print(f"Victim: {case.victim_name}")
        print(f"Location: {case.location_name}")
        print(f"Perpetrator: {case.perpetrator_name}")
        
        print(f"\nEvidence ({len(case.evidence)} items):")
        for i, evidence in enumerate(case.evidence, 1):
            reliability = "High" if evidence.reliability > 0.7 else "Medium" if evidence.reliability > 0.5 else "Low"
            print(f"  {i}. {evidence.description} (Reliability: {reliability})")
        
        print(f"\nWitnesses ({len(case.witnesses)} people):")
        for i, witness in enumerate(case.witnesses, 1):
            knowledge = witness.knowledge_level.replace('_', ' ').title()
            reliability = "High" if witness.reliability > 0.7 else "Medium" if witness.reliability > 0.5 else "Low"
            print(f"  {i}. {witness.name}: {witness.description}")
            print(f"     Knowledge: {knowledge}, Reliability: {reliability}")
        
        print(f"\nRed Herrings ({len(case.red_herrings)} items):")
        for i, red_herring in enumerate(case.red_herrings, 1):
            print(f"  {i}. {red_herring['description']}")
            print(f"     Why misleading: {red_herring['explanation']}")
        
        print(f"\nMissing Pieces to Discover ({len(case.missing_pieces)} items):")
        for i, piece in enumerate(case.missing_pieces, 1):
            print(f"  {i}. {piece}")
        
        print(f"\nSolution Path ({len(case.solution_path)} steps):")
        for i, step in enumerate(case.solution_path, 1):
            print(f"  {i}. {step}")
        
        # Create a sample case file that could be used in a game
        game_case = {
            "case_info": {
                "title": case.title,
                "crime_type": case.crime_type,
                "difficulty": "Medium",
                "estimated_time": "30-45 minutes"
            },
            "crime_scene": {
                "location": case.location_name,
                "description": case.description,
                "victim": case.victim_name
            },
            "evidence_matrix": [
                {
                    "id": f"evidence_{i}",
                    "type": evidence.type,
                    "description": evidence.description,
                    "reliability": evidence.reliability,
                    "connections": evidence.connects_to
                }
                for i, evidence in enumerate(case.evidence)
            ],
            "witnesses": [
                {
                    "id": f"witness_{i}",
                    "name": witness.name,
                    "description": witness.description,
                    "testimony": f"Testimony from {witness.name}",
                    "reliability": witness.reliability,
                    "knowledge_level": witness.knowledge_level
                }
                for i, witness in enumerate(case.witnesses)
            ],
            "red_herrings": [
                {
                    "description": red_herring['description'],
                    "explanation": red_herring['explanation'],
                    "misleads_toward": red_herring.get('connects_to', [])
                }
                for red_herring in case.red_herrings
            ],
            "investigation_objectives": case.missing_pieces,
            "solution": {
                "perpetrator": case.perpetrator_name,
                "motive": "Stress-induced impulsive behavior",
                "method": "Opportunistic crime of desperation",
                "logical_path": case.solution_path
            }
        }
        
        with open("sample_detective_case.json", "w") as f:
            json.dump(game_case, f, indent=2)
        
        print(f"\nSample game case saved to: sample_detective_case.json")
        
    print(f"\n=== Demo Complete ===")
    print("This demonstrates how simulated crimes become playable detective cases!")

if __name__ == "__main__":
    force_crime_generation_demo()