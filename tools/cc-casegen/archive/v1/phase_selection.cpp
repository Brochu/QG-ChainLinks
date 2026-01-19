#include "sim_db.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

//------------------------------------------------------------------------------
// Crime Interest Scoring
//------------------------------------------------------------------------------

static i32 count_chain_events(sim_context* ctx, const crime& c) {
    // Count events leading up to the crime - must match extraction logic exactly
    event events[256];
    i32 num_events = db_get_events_for_actor(ctx, c.perpetrator_id, events, 256);

    i32 chain_count = 0;
    for (i32 i = 0; i < num_events; i++) {
        if (events[i].timestamp >= c.timestamp) continue;

        bool relevant = false;

        switch (events[i].type) {
            case event_type::INSULT:
                // Insult involving victim, or perpetrator was insulted
                if (events[i].actor_id == c.victim_id ||
                    events[i].target_id == c.perpetrator_id) {
                    relevant = true;
                }
                break;

            case event_type::THREAT:
                // Perpetrator was threatened
                if (events[i].target_id == c.perpetrator_id) {
                    relevant = true;
                }
                break;

            case event_type::ARGUMENT:
                // Argument involving victim
                if (events[i].target_id == c.victim_id ||
                    events[i].actor_id == c.victim_id) {
                    relevant = true;
                }
                break;

            case event_type::FIRED:
                // Perpetrator was fired
                if (events[i].actor_id == c.perpetrator_id) {
                    relevant = true;
                }
                break;

            case event_type::BETRAYAL:
                // Perpetrator was betrayed
                if (events[i].target_id == c.perpetrator_id) {
                    relevant = true;
                }
                break;

            case event_type::PLANNING_CRIME:
            case event_type::ACQUIRE_WEAPON:
            case event_type::SCOUTING:
                relevant = true;
                break;

            case event_type::BORROWED_MONEY:
            case event_type::DEBT_UNPAID:
                if (events[i].actor_id == c.perpetrator_id) {
                    relevant = true;
                }
                break;

            default:
                // Direct interaction with victim
                if (events[i].target_id == c.victim_id &&
                    events[i].actor_id == c.perpetrator_id) {
                    relevant = true;
                }
                break;
        }

        if (relevant) {
            chain_count++;
        }
    }

    return chain_count;
}

static f32 calculate_chain_length_score(sim_context* ctx, const crime& c) {
    // Score: more chain events = more interesting
    return (f32)count_chain_events(ctx, c) * 2.0f;
}

static f32 calculate_suspect_score(sim_context* ctx, const crime& c) {
    // Count actors who also had motive against the victim
    i64 grudge_holders[32];
    i32 num_grudges = db_get_actors_with_grudge_against(ctx, c.victim_id, grudge_holders, 32);

    // Count actors with negative relationships to victim
    actor victim;
    if (!db_get_actor(ctx, c.victim_id, &victim)) return 0.0f;

    relationship rels[64];
    i32 num_rels = db_get_relationships_for_actor(ctx, c.victim_id, rels, 64);

    i32 negative_rels = 0;
    for (i32 i = 0; i < num_rels; i++) {
        if (rels[i].sentiment < -20) {
            negative_rels++;
        }
    }

    // More potential suspects = more interesting case
    i32 total_suspects = num_grudges + negative_rels;
    return (f32)total_suspects * 1.5f;
}

static f32 calculate_evidence_variety_score(sim_context* ctx, const crime& c) {
    // Estimate potential evidence based on location and circumstances
    f32 score = 10.0f;  // Base score

    location loc;
    if (db_get_location(ctx, c.location_id, &loc)) {
        // Higher security = more evidence
        score += (i32)loc.security * 10.0f;

        // Public locations have more witnesses
        if (loc.is_public) {
            score += 15.0f;
        }
    }

    // Check for witnesses (other actors at location at time of crime)
    i64 witnesses[32];
    i32 num_witnesses = db_get_actors_at_location(ctx, c.location_id, c.timestamp, witnesses, 32);
    score += num_witnesses * 5.0f;

    return score;
}

static f32 calculate_relationship_complexity(sim_context* ctx, const crime& c) {
    // How interconnected are the perpetrator and victim?
    actor perp, victim;
    if (!db_get_actor(ctx, c.perpetrator_id, &perp)) return 0.0f;
    if (!db_get_actor(ctx, c.victim_id, &victim)) return 0.0f;

    f32 score = 0.0f;

    // Direct relationship
    relationship rel;
    if (db_get_relationship_between(ctx, c.perpetrator_id, c.victim_id, &rel)) {
        score += 20.0f;

        // Family relationships are more dramatic
        if (rel.type == relationship_type::FAMILY) {
            score += 30.0f;
        } else if (rel.type == relationship_type::ROMANTIC) {
            score += 25.0f;
        }
    }

    // Same workplace
    if (perp.work_id > 0 && perp.work_id == victim.work_id) {
        score += 15.0f;
    }

    // Check for mutual connections
    relationship perp_rels[32], victim_rels[32];
    i32 num_perp_rels = db_get_relationships_for_actor(ctx, c.perpetrator_id, perp_rels, 32);
    i32 num_victim_rels = db_get_relationships_for_actor(ctx, c.victim_id, victim_rels, 32);

    i32 mutual = 0;
    for (i32 i = 0; i < num_perp_rels; i++) {
        i64 perp_contact = (perp_rels[i].actor1_id == c.perpetrator_id) ?
                          perp_rels[i].actor2_id : perp_rels[i].actor1_id;

        for (i32 j = 0; j < num_victim_rels; j++) {
            i64 victim_contact = (victim_rels[j].actor1_id == c.victim_id) ?
                                victim_rels[j].actor2_id : victim_rels[j].actor1_id;

            if (perp_contact == victim_contact) {
                mutual++;
            }
        }
    }

    score += mutual * 5.0f;

    return score;
}

static f32 calculate_motive_strength(sim_context* ctx, const crime& c) {
    f32 score = 0.0f;

    switch (c.motive) {
        case motive_type::REVENGE:
            score = 30.0f;
            break;
        case motive_type::FINANCIAL_GAIN:
            score = 25.0f;
            break;
        case motive_type::JEALOUSY:
            score = 35.0f;  // Passionate crimes are interesting
            break;
        case motive_type::INHERITANCE:
            score = 40.0f;  // Classic motive
            break;
        case motive_type::RAGE:
            score = 20.0f;  // Simpler motive
            break;
        case motive_type::SILENCING:
            score = 45.0f;  // Implies deeper story
            break;
        default:
            score = 10.0f;
    }

    return score;
}

//------------------------------------------------------------------------------
// Main Selection Phase
//------------------------------------------------------------------------------

bool phase_selection(sim_context* ctx, crime* selected_crime) {
    printf("[PHASE 5] Selection - Grading crimes...\n");

    crime all_crimes[64];
    i32 total_crimes = db_get_all_crimes(ctx, all_crimes, 64);

    if (total_crimes == 0) {
        printf("  No crimes occurred during simulation!\n");
        return false;
    }

    // Filter crimes that meet minimum chain length requirement
    // (chain includes the crime itself as final event, so we need chain_count >= MIN - 1)
    crime crimes[64];
    i32 num_crimes = 0;
    i32 min_events_needed = CHAIN_MIN_LENGTH - 1;

    for (i32 i = 0; i < total_crimes; i++) {
        i32 chain_count = count_chain_events(ctx, all_crimes[i]);
        if (chain_count >= min_events_needed) {
            crimes[num_crimes++] = all_crimes[i];
        }
    }

    printf("  %d crimes total, %d meet minimum chain length of %d\n",
           total_crimes, num_crimes, CHAIN_MIN_LENGTH);

    if (num_crimes == 0) {
        printf("  No crimes have sufficient narrative depth!\n");
        return false;
    }

    printf("  Evaluating %d qualifying crimes...\n", num_crimes);

    for (i32 i = 0; i < num_crimes; i++) {
        f32 chain_score = calculate_chain_length_score(ctx, crimes[i]);
        f32 suspect_score = calculate_suspect_score(ctx, crimes[i]);
        f32 evidence_score = calculate_evidence_variety_score(ctx, crimes[i]);
        f32 complexity_score = calculate_relationship_complexity(ctx, crimes[i]);
        f32 motive_score = calculate_motive_strength(ctx, crimes[i]);

        f32 total = chain_score * 2.0f +
                   suspect_score * 1.5f +
                   evidence_score * 1.5f +
                   complexity_score * 1.0f +
                   motive_score * 1.0f;

        crimes[i].interest_score = total;
        db_update_crime_score(ctx, crimes[i].id, total);

        actor perp, victim;
        db_get_actor(ctx, crimes[i].perpetrator_id, &perp);
        db_get_actor(ctx, crimes[i].victim_id, &victim);

        printf("    Crime %lld: %s -> %s, Score: %.1f\n",
               crimes[i].id, perp.name, victim.name, total);
    }

    // Sort by score (already sorted by DB, but let's be sure)
    for (i32 i = 0; i < num_crimes - 1; i++) {
        for (i32 j = i + 1; j < num_crimes; j++) {
            if (crimes[j].interest_score > crimes[i].interest_score) {
                crime temp = crimes[i];
                crimes[i] = crimes[j];
                crimes[j] = temp;
            }
        }
    }

    // Select the most interesting crime
    *selected_crime = crimes[0];

    actor perp, victim;
    db_get_actor(ctx, selected_crime->perpetrator_id, &perp);
    db_get_actor(ctx, selected_crime->victim_id, &victim);

    printf("\n  SELECTED CRIME:\n");
    printf("    Type: %s\n", crime_type_str(selected_crime->type));
    printf("    Perpetrator: %s\n", perp.name);
    printf("    Victim: %s\n", victim.name);
    printf("    Motive: %s\n", motive_type_str(selected_crime->motive));
    printf("    Day: %d\n", timestamp_to_day(selected_crime->timestamp));
    printf("    Interest Score: %.1f\n", selected_crime->interest_score);

    return true;
}
