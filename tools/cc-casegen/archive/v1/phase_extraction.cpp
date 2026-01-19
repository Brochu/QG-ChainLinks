#include "sim_db.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

//------------------------------------------------------------------------------
// Causality Chain Building
//------------------------------------------------------------------------------

struct chain_event {
    i64 event_id;
    event_type type;
    i64 actor_id;
    i64 target_id;
    i64 location_id;
    i32 timestamp;
    char summary[256];
};

static i32 build_causality_chain(sim_context* ctx, const crime& c,
                                 chain_event* chain, i32 max_chain) {
    // Get all events for perpetrator leading up to the crime
    event events[512];
    i32 num_events = db_get_events_for_actor(ctx, c.perpetrator_id, events, 512);

    // Filter to relevant events
    chain_event candidates[128];
    i32 num_candidates = 0;

    actor perp, victim;
    db_get_actor(ctx, c.perpetrator_id, &perp);
    db_get_actor(ctx, c.victim_id, &victim);

    for (i32 i = 0; i < num_events && num_candidates < 128; i++) {
        if (events[i].timestamp >= c.timestamp) continue;

        bool relevant = false;
        char summary[256] = {0};

        switch (events[i].type) {
            case event_type::INSULT:
                if (events[i].actor_id == c.victim_id ||
                    events[i].target_id == c.perpetrator_id) {
                    relevant = true;
                    if (events[i].target_id == c.perpetrator_id) {
                        actor insulter;
                        db_get_actor(ctx, events[i].actor_id, &insulter);
                        snprintf(summary, sizeof(summary), "%s insulted %s",
                                insulter.name, perp.name);
                    } else {
                        snprintf(summary, sizeof(summary), "%s was insulted by %s",
                                victim.name, perp.name);
                    }
                }
                break;

            case event_type::THREAT:
                if (events[i].target_id == c.perpetrator_id) {
                    relevant = true;
                    actor threatener;
                    db_get_actor(ctx, events[i].actor_id, &threatener);
                    snprintf(summary, sizeof(summary), "%s threatened %s",
                            threatener.name, perp.name);
                }
                break;

            case event_type::ARGUMENT:
                if (events[i].target_id == c.victim_id ||
                    events[i].actor_id == c.victim_id) {
                    relevant = true;
                    snprintf(summary, sizeof(summary), "%s and %s had an argument",
                            perp.name, victim.name);
                }
                break;

            case event_type::FIRED:
                if (events[i].actor_id == c.perpetrator_id) {
                    relevant = true;
                    snprintf(summary, sizeof(summary), "%s was fired from their job",
                            perp.name);
                }
                break;

            case event_type::BETRAYAL:
                if (events[i].target_id == c.perpetrator_id) {
                    relevant = true;
                    actor betrayer;
                    db_get_actor(ctx, events[i].actor_id, &betrayer);
                    snprintf(summary, sizeof(summary), "%s betrayed %s",
                            betrayer.name, perp.name);
                }
                break;

            case event_type::PLANNING_CRIME:
                relevant = true;
                snprintf(summary, sizeof(summary), "%s began planning the crime",
                        perp.name);
                break;

            case event_type::ACQUIRE_WEAPON:
                relevant = true;
                snprintf(summary, sizeof(summary), "%s acquired a weapon",
                        perp.name);
                break;

            case event_type::SCOUTING:
                relevant = true;
                snprintf(summary, sizeof(summary), "%s scouted the target",
                        perp.name);
                break;

            case event_type::BORROWED_MONEY:
                if (events[i].actor_id == c.perpetrator_id) {
                    relevant = true;
                    actor lender;
                    db_get_actor(ctx, events[i].target_id, &lender);
                    snprintf(summary, sizeof(summary), "%s borrowed money from %s",
                            perp.name, lender.name);
                }
                break;

            case event_type::DEBT_UNPAID:
                if (events[i].actor_id == c.perpetrator_id) {
                    relevant = true;
                    snprintf(summary, sizeof(summary), "%s failed to repay debt",
                            perp.name);
                }
                break;

            default:
                // Check if involves victim directly
                if (events[i].target_id == c.victim_id &&
                    events[i].actor_id == c.perpetrator_id) {
                    relevant = true;
                    snprintf(summary, sizeof(summary), "%s interacted with %s",
                            perp.name, victim.name);
                }
                break;
        }

        if (relevant && summary[0] != 0) {
            candidates[num_candidates].event_id = events[i].id;
            candidates[num_candidates].type = events[i].type;
            candidates[num_candidates].actor_id = events[i].actor_id;
            candidates[num_candidates].target_id = events[i].target_id;
            candidates[num_candidates].location_id = events[i].location_id;
            candidates[num_candidates].timestamp = events[i].timestamp;
            strncpy(candidates[num_candidates].summary, summary, sizeof(summary) - 1);
            num_candidates++;
        }
    }

    // Sort candidates by timestamp
    for (i32 i = 0; i < num_candidates - 1; i++) {
        for (i32 j = i + 1; j < num_candidates; j++) {
            if (candidates[j].timestamp < candidates[i].timestamp) {
                chain_event temp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = temp;
            }
        }
    }

    // Select chain of appropriate length (3-7 events)
    i32 target_length = (num_candidates > CHAIN_MAX_LENGTH) ?
                        CHAIN_MAX_LENGTH : num_candidates;
    if (target_length < CHAIN_MIN_LENGTH && num_candidates >= CHAIN_MIN_LENGTH) {
        target_length = CHAIN_MIN_LENGTH;
    }

    // If we have more candidates than needed, space them out
    i32 chain_len = 0;
    if (num_candidates <= target_length) {
        // Use all candidates
        for (i32 i = 0; i < num_candidates && chain_len < max_chain; i++) {
            chain[chain_len++] = candidates[i];
        }
    } else {
        // Select evenly spaced events
        f32 step = (f32)num_candidates / (f32)target_length;
        for (i32 i = 0; i < target_length && chain_len < max_chain; i++) {
            i32 idx = (i32)(i * step);
            if (idx >= num_candidates) idx = num_candidates - 1;
            chain[chain_len++] = candidates[idx];
        }
    }

    // Add the crime itself as the final event
    if (chain_len < max_chain) {
        chain[chain_len].event_id = 0;  // Crime event
        chain[chain_len].type = (c.type == crime_type::MURDER) ? event_type::MURDER :
                               (c.type == crime_type::KIDNAPPING) ? event_type::KIDNAPPING :
                               event_type::THEFT;
        chain[chain_len].actor_id = c.perpetrator_id;
        chain[chain_len].target_id = c.victim_id;
        chain[chain_len].location_id = c.location_id;
        chain[chain_len].timestamp = c.timestamp;
        snprintf(chain[chain_len].summary, sizeof(chain[chain_len].summary),
                "%s committed %s against %s",
                perp.name, crime_type_str(c.type), victim.name);
        chain_len++;
    }

    return chain_len;
}

//------------------------------------------------------------------------------
// Suspect Identification
//------------------------------------------------------------------------------

static i32 identify_suspects(sim_context* ctx, const crime& c,
                             i64* suspect_ids, i32 max_suspects) {
    i32 num_suspects = 0;

    // The perpetrator is always a suspect
    suspect_ids[num_suspects++] = c.perpetrator_id;

    // Find others with motive
    i64 grudge_holders[32];
    i32 num_grudges = db_get_actors_with_grudge_against(ctx, c.victim_id, grudge_holders, 32);

    for (i32 i = 0; i < num_grudges && num_suspects < max_suspects; i++) {
        bool already_added = false;
        for (i32 j = 0; j < num_suspects; j++) {
            if (suspect_ids[j] == grudge_holders[i]) {
                already_added = true;
                break;
            }
        }
        if (!already_added) {
            suspect_ids[num_suspects++] = grudge_holders[i];
        }
    }

    // Add actors with negative relationships to victim
    relationship rels[32];
    i32 num_rels = db_get_relationships_for_actor(ctx, c.victim_id, rels, 32);

    for (i32 i = 0; i < num_rels && num_suspects < max_suspects; i++) {
        if (rels[i].sentiment < -30) {
            i64 suspect = (rels[i].actor1_id == c.victim_id) ?
                         rels[i].actor2_id : rels[i].actor1_id;

            bool already_added = false;
            for (i32 j = 0; j < num_suspects; j++) {
                if (suspect_ids[j] == suspect) {
                    already_added = true;
                    break;
                }
            }
            if (!already_added) {
                suspect_ids[num_suspects++] = suspect;
            }
        }
    }

    return num_suspects;
}

//------------------------------------------------------------------------------
// Main Extraction Phase
//------------------------------------------------------------------------------

bool phase_extraction(sim_context* ctx, const crime& selected_crime,
                      case_file* output) {
    printf("[PHASE 7] Extraction - Building case file...\n");

    memset(output, 0, sizeof(case_file));
    output->selected_crime = selected_crime;

    // Build causality chain
    chain_event chain[16];
    i32 chain_len = build_causality_chain(ctx, selected_crime, chain, 16);

    output->chain = new causality_link[chain_len];
    output->chain_length = chain_len;

    printf("\n  Causality Chain (%d events):\n", chain_len);
    for (i32 i = 0; i < chain_len; i++) {
        output->chain[i].event_id = chain[i].event_id;
        output->chain[i].day = timestamp_to_day(chain[i].timestamp);
        strncpy(output->chain[i].summary, chain[i].summary, sizeof(output->chain[i].summary) - 1);

        printf("    Day %d: %s\n", output->chain[i].day, output->chain[i].summary);
    }

    // Identify suspects
    i64 suspect_ids[32];
    i32 num_suspects = identify_suspects(ctx, selected_crime, suspect_ids, 32);

    output->suspect_ids = new i64[num_suspects];
    output->num_suspects = num_suspects;
    memcpy(output->suspect_ids, suspect_ids, num_suspects * sizeof(i64));

    printf("\n  Suspects (%d):\n", num_suspects);
    for (i32 i = 0; i < num_suspects; i++) {
        actor s;
        if (db_get_actor(ctx, suspect_ids[i], &s)) {
            const char* marker = (suspect_ids[i] == selected_crime.perpetrator_id) ?
                                " [PERPETRATOR]" : "";
            printf("    %s%s\n", s.name, marker);
        }
    }

    // Get all evidence
    evidence ev[64];
    i32 num_evidence = db_get_evidence_for_crime(ctx, selected_crime.id, ev, 64);

    output->evidence_list = new evidence[num_evidence];
    output->num_evidence = num_evidence;
    memcpy(output->evidence_list, ev, num_evidence * sizeof(evidence));

    // Get all witnesses
    witness wit[32];
    i32 num_witnesses = db_get_witnesses_for_crime(ctx, selected_crime.id, wit, 32);

    output->witnesses = new witness[num_witnesses];
    output->num_witnesses = num_witnesses;
    memcpy(output->witnesses, wit, num_witnesses * sizeof(witness));

    printf("\n  Evidence: %d pieces\n", num_evidence);
    printf("  Witnesses: %d\n", num_witnesses);

    return chain_len >= CHAIN_MIN_LENGTH;
}
