#include "sim_db.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

//------------------------------------------------------------------------------
// Random helpers
//------------------------------------------------------------------------------

static i32 rnd_int(i32 max) {
    if (max <= 0) return 0;
    return rand() % max;
}

static i32 rnd_range(i32 min, i32 max) {
    if (min >= max) return min;
    return min + rand() % (max - min + 1);
}

static f32 rnd_float() {
    return (f32)rand() / (f32)RAND_MAX;
}

//------------------------------------------------------------------------------
// Physical Evidence Generation
//------------------------------------------------------------------------------

static void generate_physical_evidence(sim_context* ctx, const crime& c) {
    actor perp;
    if (!db_get_actor(ctx, c.perpetrator_id, &perp)) return;

    location loc;
    if (!db_get_location(ctx, c.location_id, &loc)) return;

    // Impulsive perpetrators leave more evidence
    f32 evidence_chance = 0.5f + perp.impulsivity * 0.005f;

    // Murder typically leaves more physical evidence
    if (c.type == crime_type::MURDER) {
        evidence_chance += 0.2f;
    }

    // Weapon evidence
    if (c.type == crime_type::MURDER && rnd_float() < evidence_chance) {
        const char* weapons[] = {
            "Kitchen knife found at scene",
            "Blunt object with blood traces",
            "Firearm recovered nearby",
            "Rope with fibers matching suspect",
            "Broken bottle with prints"
        };
        const char* weapon = weapons[rnd_int(5)];

        db_insert_evidence(ctx, c.id, evidence_type::WEAPON, c.location_id,
                          c.perpetrator_id, rnd_range(60, 95), weapon);
    }

    // Fingerprints
    if (rnd_float() < evidence_chance * 0.8f) {
        char desc[256];
        snprintf(desc, sizeof(desc), "Fingerprints found at crime scene matching %s", perp.name);
        db_insert_evidence(ctx, c.id, evidence_type::FINGERPRINT, c.location_id,
                          c.perpetrator_id, rnd_range(50, 90), desc);
    }

    // DNA
    if (rnd_float() < evidence_chance * 0.6f) {
        const char* dna_sources[] = {
            "Blood sample found at scene",
            "Hair follicle recovered",
            "Skin cells under victim's nails",
            "Saliva trace on object"
        };
        db_insert_evidence(ctx, c.id, evidence_type::DNA, c.location_id,
                          c.perpetrator_id, rnd_range(70, 99), dna_sources[rnd_int(4)]);
    }

    // Fiber evidence
    if (rnd_float() < evidence_chance * 0.5f) {
        char desc[256];
        snprintf(desc, sizeof(desc), "Clothing fibers consistent with %s's wardrobe", perp.name);
        db_insert_evidence(ctx, c.id, evidence_type::FIBER, c.location_id,
                          c.perpetrator_id, rnd_range(40, 70), desc);
    }

    // Footprints
    if (rnd_float() < evidence_chance * 0.4f) {
        db_insert_evidence(ctx, c.id, evidence_type::FOOTPRINT, c.location_id,
                          c.perpetrator_id, rnd_range(30, 60),
                          "Shoe prints found at scene");
    }
}

//------------------------------------------------------------------------------
// Witness Generation
//------------------------------------------------------------------------------

static void generate_witnesses(sim_context* ctx, const crime& c) {
    // Find actors who were at or near the location
    i64 nearby_actors[64];
    i32 num_nearby = db_get_actors_at_location(ctx, c.location_id, c.timestamp,
                                                nearby_actors, 64);

    actor perp, victim;
    db_get_actor(ctx, c.perpetrator_id, &perp);
    db_get_actor(ctx, c.victim_id, &victim);

    for (i32 i = 0; i < num_nearby; i++) {
        if (nearby_actors[i] == c.perpetrator_id ||
            nearby_actors[i] == c.victim_id) continue;

        actor witness_actor;
        if (!db_get_actor(ctx, nearby_actors[i], &witness_actor)) continue;

        // Chance they actually saw something
        if (rnd_float() < 0.4f) {
            char saw_what[256];
            i32 reliability = rnd_range(40, 90);
            i32 willingness = rnd_range(30, 80);

            f32 detail_roll = rnd_float();
            if (detail_roll < 0.3f) {
                // Saw the perpetrator
                snprintf(saw_what, sizeof(saw_what),
                        "Saw someone matching %s's description near the scene", perp.name);
                reliability = rnd_range(60, 95);
            } else if (detail_roll < 0.5f) {
                // Heard something
                snprintf(saw_what, sizeof(saw_what),
                        "Heard suspicious sounds around the time of the crime");
                reliability = rnd_range(30, 60);
            } else if (detail_roll < 0.7f) {
                // Saw victim beforehand
                snprintf(saw_what, sizeof(saw_what),
                        "Saw %s earlier that day", victim.name);
            } else {
                // Saw suspicious behavior
                snprintf(saw_what, sizeof(saw_what),
                        "Noticed unusual activity in the area");
            }

            db_insert_witness(ctx, c.id, nearby_actors[i], saw_what,
                             reliability, willingness);

            // Also create a witness statement as evidence
            char stmt[256];
            snprintf(stmt, sizeof(stmt), "Statement from %s: %s",
                    witness_actor.name, saw_what);
            db_insert_evidence(ctx, c.id, evidence_type::WITNESS_STATEMENT,
                              0, 0, reliability, stmt);
        }
    }

    // Check for witnesses from related events (people who saw planning, etc.)
    event events[32];
    i32 num_events = db_get_events_for_actor(ctx, c.perpetrator_id, events, 32);

    for (i32 i = 0; i < num_events; i++) {
        if (events[i].type == event_type::PLANNING_CRIME ||
            events[i].type == event_type::ACQUIRE_WEAPON ||
            events[i].type == event_type::SCOUTING) {

            if (events[i].timestamp < c.timestamp) {
                // Potential witnesses to planning
                i64 planning_witnesses[16];
                i32 num_pw = db_get_actors_at_location(ctx, events[i].location_id,
                                                        events[i].timestamp,
                                                        planning_witnesses, 16);

                for (i32 j = 0; j < num_pw && j < 2; j++) {
                    if (planning_witnesses[j] == c.perpetrator_id) continue;

                    if (rnd_float() < 0.2f) {
                        actor w;
                        if (db_get_actor(ctx, planning_witnesses[j], &w)) {
                            char saw_what[256];
                            snprintf(saw_what, sizeof(saw_what),
                                    "Saw %s acting suspiciously days before the crime", perp.name);

                            db_insert_witness(ctx, c.id, planning_witnesses[j],
                                             saw_what, rnd_range(30, 60), rnd_range(40, 70));
                        }
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
// Documentary Evidence Generation
//------------------------------------------------------------------------------

static void generate_documentary_evidence(sim_context* ctx, const crime& c) {
    actor perp;
    if (!db_get_actor(ctx, c.perpetrator_id, &perp)) return;

    location loc;
    if (!db_get_location(ctx, c.location_id, &loc)) return;

    // Security footage
    if (loc.security >= security_level::CAMERAS) {
        f32 quality = (loc.security == security_level::VAULT) ? 0.9f : 0.6f;

        if (rnd_float() < quality) {
            char desc[256];
            snprintf(desc, sizeof(desc),
                    "Security camera footage showing %s at location", perp.name);
            db_insert_evidence(ctx, c.id, evidence_type::SECURITY_FOOTAGE, c.location_id,
                              c.perpetrator_id, rnd_range(70, 95), desc);
        }
    }

    // Phone records
    if (rnd_float() < 0.6f) {
        char desc[256];
        snprintf(desc, sizeof(desc),
                "Phone records showing %s's location near crime scene", perp.name);
        db_insert_evidence(ctx, c.id, evidence_type::PHONE_RECORD, 0,
                          c.perpetrator_id, rnd_range(60, 85), desc);
    }

    // Bank records (for financial crimes or motive)
    if (c.motive == motive_type::FINANCIAL_GAIN || c.motive == motive_type::INHERITANCE) {
        if (rnd_float() < 0.5f) {
            char desc[256];
            snprintf(desc, sizeof(desc),
                    "Bank records showing %s's financial difficulties", perp.name);
            db_insert_evidence(ctx, c.id, evidence_type::BANK_RECORD, 0,
                              c.perpetrator_id, rnd_range(50, 80), desc);
        }
    }

    // Receipts
    if (rnd_float() < 0.3f) {
        db_insert_evidence(ctx, c.id, evidence_type::RECEIPT, c.location_id,
                          c.perpetrator_id, rnd_range(40, 70),
                          "Receipt placing suspect near crime scene");
    }

    // Email evidence (threats, planning)
    if (rnd_float() < 0.2f) {
        actor victim;
        if (db_get_actor(ctx, c.victim_id, &victim)) {
            char desc[256];
            snprintf(desc, sizeof(desc),
                    "Emails between %s and %s showing hostility", perp.name, victim.name);
            db_insert_evidence(ctx, c.id, evidence_type::EMAIL, 0,
                              c.perpetrator_id, rnd_range(60, 85), desc);
        }
    }
}

//------------------------------------------------------------------------------
// Alibi Evidence (for red herrings)
//------------------------------------------------------------------------------

static void generate_alibis(sim_context* ctx, const crime& c) {
    // Find other actors who had motive but have alibis
    i64 grudge_holders[32];
    i32 num_grudges = db_get_actors_with_grudge_against(ctx, c.victim_id, grudge_holders, 32);

    for (i32 i = 0; i < num_grudges && i < 5; i++) {
        if (grudge_holders[i] == c.perpetrator_id) continue;

        actor suspect;
        if (!db_get_actor(ctx, grudge_holders[i], &suspect)) continue;

        // Check where this suspect was at time of crime
        i64 suspect_loc = db_get_actor_location_at(ctx, grudge_holders[i], c.timestamp);

        if (suspect_loc != c.location_id && suspect_loc > 0) {
            // They have an alibi!
            location alibi_loc;
            if (db_get_location(ctx, suspect_loc, &alibi_loc)) {
                char desc[256];
                snprintf(desc, sizeof(desc),
                        "%s was at %s at time of crime (alibi)", suspect.name, alibi_loc.name);
                db_insert_evidence(ctx, c.id, evidence_type::ALIBI, suspect_loc,
                                  grudge_holders[i], rnd_range(60, 90), desc);
            }
        }
    }
}

//------------------------------------------------------------------------------
// Main Evidence Phase
//------------------------------------------------------------------------------

bool phase_evidence(sim_context* ctx, const crime& selected_crime) {
    printf("[PHASE 6] Evidence - Generating evidence...\n");

    generate_physical_evidence(ctx, selected_crime);
    generate_witnesses(ctx, selected_crime);
    generate_documentary_evidence(ctx, selected_crime);
    generate_alibis(ctx, selected_crime);

    // Count generated evidence
    evidence ev[64];
    i32 num_evidence = db_get_evidence_for_crime(ctx, selected_crime.id, ev, 64);

    witness wit[32];
    i32 num_witnesses = db_get_witnesses_for_crime(ctx, selected_crime.id, wit, 32);

    printf("  Generated %d pieces of evidence\n", num_evidence);
    printf("  Generated %d witness statements\n", num_witnesses);

    // Print evidence summary
    printf("\n  Evidence Summary:\n");
    for (i32 i = 0; i < num_evidence && i < 10; i++) {
        printf("    [%s] %s\n", evidence_type_str(ev[i].type), ev[i].description);
    }
    if (num_evidence > 10) {
        printf("    ... and %d more\n", num_evidence - 10);
    }

    return num_evidence > 0;
}
