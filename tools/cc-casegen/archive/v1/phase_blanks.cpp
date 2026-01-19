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

static f32 rnd_float() {
    return (f32)rand() / (f32)RAND_MAX;
}

//------------------------------------------------------------------------------
// Blank Candidate Structure
//------------------------------------------------------------------------------

struct blank_candidate {
    blank_type type;
    i32 chain_index;
    char correct_answer[128];
    i64 supporting_evidence[8];
    i32 num_evidence;
    f32 difficulty;  // 0 = easy, 1 = hard
};

//------------------------------------------------------------------------------
// Find Supporting Evidence
//------------------------------------------------------------------------------

static i32 find_who_evidence(sim_context* ctx, const case_file& cf, i64 actor_id,
                             i64* evidence_ids, i32 max_evidence) {
    i32 count = 0;

    for (i32 i = 0; i < cf.num_evidence && count < max_evidence; i++) {
        if (cf.evidence_list[i].points_to_id == actor_id) {
            evidence_ids[count++] = cf.evidence_list[i].id;
        }
    }

    // Also check witness statements
    for (i32 i = 0; i < cf.num_witnesses && count < max_evidence; i++) {
        actor a;
        if (db_get_actor(ctx, actor_id, &a)) {
            // Check if witness statement mentions this actor
            if (strstr(cf.witnesses[i].saw_what, a.name) != nullptr) {
                // Create evidence reference (using witness ID as pseudo-evidence)
                evidence_ids[count++] = -cf.witnesses[i].id;  // Negative = witness
            }
        }
    }

    return count;
}

static i32 find_where_evidence(sim_context* ctx, const case_file& cf, i64 location_id,
                               i64* evidence_ids, i32 max_evidence) {
    i32 count = 0;

    for (i32 i = 0; i < cf.num_evidence && count < max_evidence; i++) {
        if (cf.evidence_list[i].location_id == location_id) {
            evidence_ids[count++] = cf.evidence_list[i].id;
        }
    }

    return count;
}

static i32 find_when_evidence(sim_context* ctx, const case_file& cf, i32 day,
                              i64* evidence_ids, i32 max_evidence) {
    i32 count = 0;

    // Phone records, security footage, receipts can establish timing
    for (i32 i = 0; i < cf.num_evidence && count < max_evidence; i++) {
        if (cf.evidence_list[i].type == evidence_type::PHONE_RECORD ||
            cf.evidence_list[i].type == evidence_type::SECURITY_FOOTAGE ||
            cf.evidence_list[i].type == evidence_type::RECEIPT) {
            evidence_ids[count++] = cf.evidence_list[i].id;
        }
    }

    return count;
}

static i32 find_what_evidence(sim_context* ctx, const case_file& cf, const char* object,
                              i64* evidence_ids, i32 max_evidence) {
    i32 count = 0;

    for (i32 i = 0; i < cf.num_evidence && count < max_evidence; i++) {
        // Weapons and physical evidence
        if (cf.evidence_list[i].type == evidence_type::WEAPON ||
            cf.evidence_list[i].type == evidence_type::FINGERPRINT ||
            cf.evidence_list[i].type == evidence_type::DNA) {
            evidence_ids[count++] = cf.evidence_list[i].id;
        }
    }

    return count;
}

//------------------------------------------------------------------------------
// Generate Blank Candidates
//------------------------------------------------------------------------------

static i32 generate_candidates(sim_context* ctx, const case_file& cf,
                               blank_candidate* candidates, i32 max_candidates) {
    i32 num_candidates = 0;

    // For each chain event, consider what can be blanked
    for (i32 i = 0; i < cf.chain_length && num_candidates < max_candidates; i++) {
        // Skip the final crime event (too obvious)
        if (i == cf.chain_length - 1) continue;

        // WHO blank - actor in the event
        actor a;
        if (db_get_actor(ctx, cf.selected_crime.perpetrator_id, &a)) {
            blank_candidate cand;
            cand.type = blank_type::WHO;
            cand.chain_index = i;
            strncpy(cand.correct_answer, a.name, sizeof(cand.correct_answer) - 1);

            cand.num_evidence = find_who_evidence(ctx, cf, cf.selected_crime.perpetrator_id,
                                                  cand.supporting_evidence, 8);

            if (cand.num_evidence >= 1) {
                cand.difficulty = (cand.num_evidence == 1) ? 0.8f :
                                 (cand.num_evidence == 2) ? 0.5f : 0.3f;
                candidates[num_candidates++] = cand;
            }
        }

        // WHEN blank - day of event
        {
            blank_candidate cand;
            cand.type = blank_type::WHEN;
            cand.chain_index = i;
            snprintf(cand.correct_answer, sizeof(cand.correct_answer),
                    "Day %d", cf.chain[i].day);

            cand.num_evidence = find_when_evidence(ctx, cf, cf.chain[i].day,
                                                   cand.supporting_evidence, 8);

            if (cand.num_evidence >= 1) {
                cand.difficulty = 0.6f;
                candidates[num_candidates++] = cand;
            }
        }
    }

    // WHERE blank for the crime location
    {
        location loc;
        if (db_get_location(ctx, cf.selected_crime.location_id, &loc)) {
            blank_candidate cand;
            cand.type = blank_type::WHERE;
            cand.chain_index = cf.chain_length - 1;
            strncpy(cand.correct_answer, loc.name, sizeof(cand.correct_answer) - 1);

            cand.num_evidence = find_where_evidence(ctx, cf, cf.selected_crime.location_id,
                                                    cand.supporting_evidence, 8);

            if (cand.num_evidence >= 1) {
                cand.difficulty = (cand.num_evidence == 1) ? 0.7f : 0.4f;
                candidates[num_candidates++] = cand;
            }
        }
    }

    // WHAT blank for weapon (if murder)
    if (cf.selected_crime.type == crime_type::MURDER) {
        // Find weapon evidence
        for (i32 i = 0; i < cf.num_evidence; i++) {
            if (cf.evidence_list[i].type == evidence_type::WEAPON) {
                blank_candidate cand;
                cand.type = blank_type::WHAT;
                cand.chain_index = cf.chain_length - 1;
                strncpy(cand.correct_answer, "murder weapon", sizeof(cand.correct_answer) - 1);

                cand.num_evidence = find_what_evidence(ctx, cf, "weapon",
                                                       cand.supporting_evidence, 8);

                if (cand.num_evidence >= 1) {
                    cand.difficulty = 0.5f;
                    candidates[num_candidates++] = cand;
                }
                break;
            }
        }
    }

    return num_candidates;
}

//------------------------------------------------------------------------------
// Select Final Blanks
//------------------------------------------------------------------------------

static i32 select_blanks(blank_candidate* candidates, i32 num_candidates,
                        case_file::blank* blanks, i32 max_blanks) {
    // Sort by difficulty (want a mix)
    for (i32 i = 0; i < num_candidates - 1; i++) {
        for (i32 j = i + 1; j < num_candidates; j++) {
            if (candidates[j].difficulty > candidates[i].difficulty) {
                blank_candidate temp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = temp;
            }
        }
    }

    // Select blanks with different types, ensuring solvability
    i32 num_blanks = 0;
    bool used_types[4] = {false};

    // First pass: one of each type if possible
    for (i32 i = 0; i < num_candidates && num_blanks < max_blanks; i++) {
        i32 type_idx = (i32)candidates[i].type;
        if (!used_types[type_idx] && candidates[i].num_evidence >= 1) {
            blanks[num_blanks].type = candidates[i].type;
            blanks[num_blanks].chain_index = candidates[i].chain_index;
            strncpy(blanks[num_blanks].correct_answer, candidates[i].correct_answer,
                   sizeof(blanks[num_blanks].correct_answer) - 1);

            blanks[num_blanks].evidence_ids = new i64[candidates[i].num_evidence];
            blanks[num_blanks].num_evidence = candidates[i].num_evidence;
            memcpy(blanks[num_blanks].evidence_ids, candidates[i].supporting_evidence,
                  candidates[i].num_evidence * sizeof(i64));

            used_types[type_idx] = true;
            num_blanks++;
        }
    }

    // Second pass: fill remaining slots with best remaining candidates
    for (i32 i = 0; i < num_candidates && num_blanks < max_blanks; i++) {
        // Check if this candidate is already used
        bool already_used = false;
        for (i32 j = 0; j < num_blanks; j++) {
            if (blanks[j].chain_index == candidates[i].chain_index &&
                blanks[j].type == candidates[i].type) {
                already_used = true;
                break;
            }
        }

        if (!already_used && candidates[i].num_evidence >= 1) {
            blanks[num_blanks].type = candidates[i].type;
            blanks[num_blanks].chain_index = candidates[i].chain_index;
            strncpy(blanks[num_blanks].correct_answer, candidates[i].correct_answer,
                   sizeof(blanks[num_blanks].correct_answer) - 1);

            blanks[num_blanks].evidence_ids = new i64[candidates[i].num_evidence];
            blanks[num_blanks].num_evidence = candidates[i].num_evidence;
            memcpy(blanks[num_blanks].evidence_ids, candidates[i].supporting_evidence,
                  candidates[i].num_evidence * sizeof(i64));

            num_blanks++;
        }
    }

    return num_blanks;
}

//------------------------------------------------------------------------------
// Main Blanks Phase
//------------------------------------------------------------------------------

bool phase_blanks(sim_context* ctx, case_file* cf) {
    printf("[PHASE 8] Blanks - Generating player challenge...\n");

    // Generate blank candidates
    blank_candidate candidates[32];
    i32 num_candidates = generate_candidates(ctx, *cf, candidates, 32);

    printf("  Found %d blank candidates\n", num_candidates);

    if (num_candidates == 0) {
        printf("  WARNING: No valid blanks could be generated!\n");
        return false;
    }

    // Select final blanks
    cf->blanks = new case_file::blank[8];
    cf->num_blanks = select_blanks(candidates, num_candidates, cf->blanks, 5);

    printf("  Selected %d blanks:\n", cf->num_blanks);

    const char* type_names[] = { "WHO", "WHERE", "WHEN", "WHAT" };

    for (i32 i = 0; i < cf->num_blanks; i++) {
        printf("    [%s] Chain %d: Answer='%s' (%d evidence)\n",
               type_names[(i32)cf->blanks[i].type],
               cf->blanks[i].chain_index,
               cf->blanks[i].correct_answer,
               cf->blanks[i].num_evidence);
    }

    return cf->num_blanks > 0;
}
