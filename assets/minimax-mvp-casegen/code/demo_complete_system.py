#!/usr/bin/env python3
"""
Complete Crime Simulation System Demo
This script demonstrates the full workflow from simulation to detective cases.
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from crime_simulation_core import CrimeSimulation, SimulationEngine
from case_generator import CaseGenerator
import json

def main():
    print("=== Crime Simulation System Demo ===\n")
    
    # Step 1: Run the crime simulation
    print("1. RUNNING CRIME SIMULATION")
    print("-" * 40)
    
    # Create and run simulation
    sim = CrimeSimulation("demo_crime_simulation.db")
    engine = SimulationEngine(sim)
    
    # Run simulation for demo (increase days and make crime generation more likely)
    print("Generating world and population...")
    engine.run_simulation(num_days=90)
    
    # Step 2: Generate detective cases
    print("\n2. GENERATING DETECTIVE CASES")
    print("-" * 40)
    
    generator = CaseGenerator("demo_crime_simulation.db")
    cases = generator.generate_all_cases()
    
    print(f"Generated {len(cases)} detective cases from simulation")
    
    if not cases:
        print("No crimes were generated in this simulation run.")
        print("Try running again or increasing the simulation length.")
        return
    
    # Step 3: Display case information
    print("\n3. CASE OVERVIEW")
    print("-" * 40)
    
    for i, case in enumerate(cases[:3], 1):  # Show first 3 cases
        print(f"Case {i}: {case.title}")
        print(f"  Crime: {case.crime_type}")
        print(f"  Location: {case.location_name}")
        print(f"  Evidence: {len(case.evidence)} items")
        print(f"  Witnesses: {len(case.witnesses)} people")
        print(f"  Red Herrings: {len(case.red_herrings)} misleading clues")
        print()
    
    # Step 4: Save the best case for detailed examination
    print("4. DETAILED CASE ANALYSIS")
    print("-" * 40)
    
    # Select case with most evidence for detailed analysis
    best_case = max(cases, key=lambda c: len(c.evidence) + len(c.witnesses))
    
    print(f"Analyzing: {best_case.title}")
    print(f"Crime: {best_case.description}")
    print(f"Victim: {best_case.victim_name}")
    print(f"Location: {best_case.location_name}")
    print()
    
    # Show evidence breakdown
    print("EVIDENCE:")
    for i, evidence in enumerate(best_case.evidence, 1):
        reliability = "High" if evidence.reliability > 0.7 else "Medium" if evidence.reliability > 0.5 else "Low"
        print(f"  {i}. {evidence.description} (Reliability: {reliability})")
    print()
    
    # Show witnesses
    print("WITNESSES:")
    for i, witness in enumerate(best_case.witnesses, 1):
        knowledge = witness.knowledge_level.replace('_', ' ').title()
        reliability = "High" if witness.reliability > 0.7 else "Medium" if witness.reliability > 0.5 else "Low"
        bias_info = f" (Biased toward {witness.bias_toward})" if witness.bias_toward else ""
        print(f"  {i}. {witness.name}: {witness.description}")
        print(f"     Knowledge: {knowledge}, Reliability: {reliability}{bias_info}")
    print()
    
    # Show red herrings
    print("RED HERRINGS (Misleading Evidence):")
    for i, red_herring in enumerate(best_case.red_herrings, 1):
        print(f"  {i}. {red_herring['description']}")
        print(f"     Why it's misleading: {red_herring['explanation']}")
    print()
    
    # Show missing pieces (the puzzle)
    print("MISSING PIECES TO DISCOVER:")
    for i, piece in enumerate(best_case.missing_pieces, 1):
        print(f"  {i}. {piece}")
    print()
    
    # Show solution path
    print("SOLUTION PATH (Logical steps to solve):")
    for i, step in enumerate(best_case.solution_path, 1):
        print(f"  {i}. {step}")
    print()
    
    # Step 5: Save case files
    print("5. SAVING CASE FILES")
    print("-" * 40)
    
    # Save the best case
    generator.save_case_to_file(best_case, f"detective_case_{best_case.case_id}.json")
    print(f"Saved best case to: detective_case_{best_case.case_id}.json")
    
    # Save a summary of all cases
    case_summary = {
        'total_cases': len(cases),
        'crime_types': {},
        'locations': {},
        'cases': [
            {
                'case_id': case.case_id,
                'title': case.title,
                'crime_type': case.case_type,
                'evidence_count': len(case.evidence),
                'witness_count': len(case.witnesses),
                'difficulty': 'High' if len(case.evidence) + len(case.witnesses) > 8 else 'Medium'
            }
            for case in cases
        ]
    }
    
    # Count crime types and locations
    for case in cases:
        case_summary['crime_types'][case.crime_type] = case_summary['crime_types'].get(case.crime_type, 0) + 1
        case_summary['locations'][case.location_name] = case_summary['locations'].get(case.location_name, 0) + 1
    
    with open('case_summary.json', 'w') as f:
        json.dump(case_summary, f, indent=2)
    
    print("Saved case summary to: case_summary.json")
    
    # Step 6: Simulation statistics
    print("\n6. SIMULATION STATISTICS")
    print("-" * 40)
    
    # Show some interesting stats from the simulation
    cursor = sim.conn.cursor()
    
    # Total characters
    cursor.execute("SELECT COUNT(*) FROM characters")
    total_chars = cursor.fetchone()[0]
    print(f"Total characters generated: {total_chars}")
    
    # Character stress levels
    cursor.execute("SELECT AVG(stress_level), MAX(stress_level) FROM characters")
    avg_stress, max_stress = cursor.fetchone()
    print(f"Average stress level: {avg_stress:.2f}")
    print(f"Highest stress level: {max_stress:.2f}")
    
    # High stress characters
    cursor.execute("SELECT COUNT(*) FROM characters WHERE stress_level > 0.8")
    high_stress_count = cursor.fetchone()[0]
    print(f"Characters with high stress (>0.8): {high_stress_count}")
    
    # Relationship statistics
    cursor.execute("SELECT COUNT(*) FROM relationships")
    total_relationships = cursor.fetchone()[0]
    print(f"Total relationships created: {total_relationships}")
    
    # Event count
    cursor.execute("SELECT COUNT(*) FROM events")
    total_events = cursor.fetchone()[0]
    print(f"Total events simulated: {total_events}")
    
    # Crime statistics
    cursor.execute("SELECT crime_type, COUNT(*) FROM crimes GROUP BY crime_type")
    crime_stats = cursor.fetchall()
    print("Crime breakdown:")
    for crime_type, count in crime_stats:
        print(f"  {crime_type}: {count}")
    
    print("\n=== Demo Complete ===")
    print("Files generated:")
    print(f"  - detective_case_{best_case.case_id}.json (playable case)")
    print("  - case_summary.json (all cases overview)")
    print("  - demo_crime_simulation.db (simulation data for debugging)")

if __name__ == "__main__":
    main()