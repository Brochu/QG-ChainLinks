#include "sim_db.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------

bool check_crime_potential(sim_context* ctx, i64 actor_id, i32 timestamp);

//------------------------------------------------------------------------------
// Random helpers
//------------------------------------------------------------------------------

static i32 rnd_int(i32 max) {
    return rand() % max;
}

static i32 rnd_range(i32 min, i32 max) {
    if (min >= max) return min;
    return min + rand() % (max - min + 1);
}

static f32 rnd_float() {
    return (f32)rand() / (f32)RAND_MAX;
}

static i32 clamp(i32 val, i32 min, i32 max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

//------------------------------------------------------------------------------
// Time Helpers
//------------------------------------------------------------------------------

static bool is_work_hours(i32 hour) {
    return hour >= 8 && hour < 18;
}

static bool is_evening(i32 hour) {
    return hour >= 18 && hour < 23;
}

static bool is_night(i32 hour) {
    return hour >= 23 || hour < 6;
}

static bool is_location_open(const location& loc, i32 hour) {
    if (loc.open_hour < 0) return true;  // Always open

    if (loc.open_hour < loc.close_hour) {
        return hour >= loc.open_hour && hour < loc.close_hour;
    } else {
        // Wraps around midnight (e.g., bar open 17-02)
        return hour >= loc.open_hour || hour < loc.close_hour;
    }
}

//------------------------------------------------------------------------------
// Action Types
//------------------------------------------------------------------------------

enum class action_type {
    STAY,
    GO_WORK,
    GO_HOME,
    GO_SOCIALIZE,
    GO_SHOPPING,
    WORK,
    SOCIALIZE,
    REST,
    EAT
};

struct action_option {
    action_type type;
    i64 location_id;
    f32 score;
};

//------------------------------------------------------------------------------
// Need Decay
//------------------------------------------------------------------------------

static void decay_needs(actor& a, i32 hours_passed) {
    // Money decays slowly (daily expenses)
    if (hours_passed >= 24) {
        i32 expense = rnd_range(1, 3);
        a.need_money = clamp(a.need_money - expense, 0, NEED_MAX);
    }

    // Belonging decays without social contact
    a.need_belonging = clamp(a.need_belonging - 1, 0, NEED_MAX);

    // Status is relatively stable
    if (rnd_float() < 0.1f) {
        a.need_status = clamp(a.need_status - 1, 0, NEED_MAX);
    }

    // Security is stable unless something happens
    // (decayed by events, not time)
}

//------------------------------------------------------------------------------
// Action Scoring
//------------------------------------------------------------------------------

static f32 score_go_work(const actor& a, i32 hour) {
    if (!is_work_hours(hour)) return 0.0f;
    if (a.work_id <= 0) return 0.0f;

    f32 score = 50.0f;

    // Low money increases work motivation
    if (a.need_money < 40) score += 30.0f;
    if (a.need_money < 20) score += 20.0f;

    return score;
}

static f32 score_go_home(const actor& a, i32 hour) {
    f32 score = 20.0f;

    // Evening/night strongly prefer home
    if (is_night(hour)) score += 60.0f;
    if (is_evening(hour) && hour >= 21) score += 30.0f;

    // Low security makes home appealing
    if (a.need_security < 50) score += 20.0f;

    return score;
}

static f32 score_socialize(const actor& a, i32 hour) {
    if (is_night(hour) && hour >= 2) return 0.0f;  // Too late

    f32 score = 20.0f;

    // Low belonging increases desire to socialize
    if (a.need_belonging < 50) score += 30.0f;
    if (a.need_belonging < 30) score += 30.0f;

    // High impulsivity more likely to socialize
    score += a.impulsivity * 0.2f;

    // Evening is social time
    if (is_evening(hour)) score += 20.0f;

    return score;
}

static f32 score_stay(const actor& a, i64 current_loc, i32 hour) {
    f32 score = 30.0f;

    // At work during work hours - stay
    if (current_loc == a.work_id && is_work_hours(hour)) {
        score += 50.0f;
    }

    // At home at night - stay
    if (current_loc == a.home_id && is_night(hour)) {
        score += 70.0f;
    }

    return score;
}

//------------------------------------------------------------------------------
// Action Selection
//------------------------------------------------------------------------------

static action_option select_action(sim_context* ctx, const actor& a, i64 current_loc, i32 hour) {
    action_option options[8];
    i32 num_options = 0;

    // Option: Stay where you are
    options[num_options++] = { action_type::STAY, current_loc, score_stay(a, current_loc, hour) };

    // Option: Go to work
    if (a.work_id > 0 && current_loc != a.work_id) {
        options[num_options++] = { action_type::GO_WORK, a.work_id, score_go_work(a, hour) };
    }

    // Option: Go home
    if (a.home_id > 0 && current_loc != a.home_id) {
        options[num_options++] = { action_type::GO_HOME, a.home_id, score_go_home(a, hour) };
    }

    // Option: Socialize (find a bar or restaurant)
    location social_locs[16];
    i32 num_bars = db_get_locations_by_type(ctx, location_type::BAR, social_locs, 8);
    i32 num_restaurants = db_get_locations_by_type(ctx, location_type::RESTAURANT, social_locs + num_bars, 8);
    i32 num_social = num_bars + num_restaurants;

    if (num_social > 0) {
        i32 idx = rnd_int(num_social);
        if (is_location_open(social_locs[idx], hour)) {
            options[num_options++] = {
                action_type::GO_SOCIALIZE,
                social_locs[idx].id,
                score_socialize(a, hour)
            };
        }
    }

    // Weighted random selection
    f32 total_score = 0.0f;
    for (i32 i = 0; i < num_options; i++) {
        total_score += options[i].score;
    }

    if (total_score <= 0.0f) {
        return options[0];  // Default to stay
    }

    f32 roll = rnd_float() * total_score;
    f32 cumulative = 0.0f;

    for (i32 i = 0; i < num_options; i++) {
        cumulative += options[i].score;
        if (roll < cumulative) {
            return options[i];
        }
    }

    return options[0];
}

//------------------------------------------------------------------------------
// Social Interaction
//------------------------------------------------------------------------------

static void simulate_social_interaction(sim_context* ctx, i64 actor_id, i64 location_id, i32 timestamp) {
    // Find other actors at this location
    i64 others[32];
    i32 num_others = db_get_actors_at_location(ctx, location_id, timestamp, others, 32);

    if (num_others == 0) return;

    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return;

    // Pick someone to interact with
    i64 target_id = others[rnd_int(num_others)];
    if (target_id == actor_id) return;

    actor target;
    if (!db_get_actor(ctx, target_id, &target)) return;

    // Check existing relationship
    relationship rel;
    bool has_rel = db_get_relationship_between(ctx, actor_id, target_id, &rel);
    bool is_strained = has_rel && rel.sentiment < -20;  // Relationship is already troubled

    // Three-way interaction type: positive, neutral, negative
    // Base distribution: 30% positive, 55% neutral, 15% negative
    f32 positive_chance = 0.30f;
    f32 negative_chance = 0.15f;

    if (has_rel) {
        // Good relationships shift toward positive
        positive_chance += rel.sentiment * 0.002f;
        // Bad relationships shift toward negative
        if (rel.sentiment < 0) {
            negative_chance += (-rel.sentiment) * 0.002f;
        }
    }

    // Aggression increases negative chance
    negative_chance += (a.aggression - 50) * 0.002f;
    negative_chance += (target.aggression - 50) * 0.001f;

    // Clamp values
    positive_chance = clamp((i32)(positive_chance * 100), 5, 60) / 100.0f;
    negative_chance = clamp((i32)(negative_chance * 100), 5, 40) / 100.0f;

    f32 roll = rnd_float();

    if (roll < positive_chance) {
        // === POSITIVE INTERACTION ===
        f32 sub_roll = rnd_float();

        if (sub_roll < 0.7f) {
            // Simple conversation
            db_insert_event(ctx, timestamp, event_type::CONVERSATION,
                           actor_id, target_id, location_id, "Had a conversation");

            // Boost belonging
            a.need_belonging = clamp(a.need_belonging + 5, 0, NEED_MAX);
            db_update_actor_needs(ctx, actor_id, a.need_money, a.need_belonging,
                                 a.need_status, a.need_security);

            // Small relationship improvement
            if (has_rel) {
                db_update_relationship_sentiment(ctx, rel.id,
                    clamp(rel.sentiment + rnd_range(1, 5), -100, 100));
            }

        } else {
            // Did a favor
            db_insert_event(ctx, timestamp, event_type::FAVOR_DONE,
                           actor_id, target_id, location_id, "Did a favor");

            // Larger relationship improvement
            if (has_rel) {
                db_update_relationship_sentiment(ctx, rel.id,
                    clamp(rel.sentiment + rnd_range(5, 15), -100, 100));
            } else {
                db_insert_relationship(ctx, actor_id, target_id,
                    relationship_type::ACQUAINTANCE, relationship_subtype::NONE,
                    rnd_range(20, 40), rnd_range(20, 40));
            }
        }

    } else if (roll < positive_chance + (1.0f - positive_chance - negative_chance)) {
        // === NEUTRAL INTERACTION ===
        // Small talk, passing acknowledgment - nothing significant happens
        // Tiny belonging boost, no relationship change, no grievance
        a.need_belonging = clamp(a.need_belonging + 1, 0, NEED_MAX);
        db_update_actor_needs(ctx, actor_id, a.need_money, a.need_belonging,
                             a.need_status, a.need_security);

    } else {
        // === NEGATIVE INTERACTION ===
        f32 sub_roll = rnd_float();

        if (sub_roll < 0.6f) {
            // Argument - most common negative interaction
            db_insert_event(ctx, timestamp, event_type::ARGUMENT,
                           actor_id, target_id, location_id, "Had an argument");

            // Arguments only create grievances if relationship is already strained
            if (is_strained && rnd_float() < 0.3f) {
                db_insert_grievance(ctx, target_id, actor_id, 0,
                                   rnd_range(5, 15), timestamp);
            }

            // Hurt relationship
            if (has_rel) {
                db_update_relationship_sentiment(ctx, rel.id,
                    clamp(rel.sentiment - rnd_range(5, 15), -100, 100));
            } else {
                // Create new strained relationship
                db_insert_relationship(ctx, actor_id, target_id,
                    relationship_type::ACQUAINTANCE, relationship_subtype::NONE,
                    rnd_range(10, 30), rnd_range(-20, -5));
            }

        } else if (sub_roll < 0.9f) {
            // Insult
            i64 event_id = db_insert_event(ctx, timestamp, event_type::INSULT,
                           actor_id, target_id, location_id, "Insulted them");

            // Insults only create grievances if relationship is already strained
            // This is the key change - random insults don't lead to murder
            if (is_strained) {
                i32 severity = rnd_range(15, 40);
                severity = (i32)(severity * (1.0f + target.aggression * 0.01f));
                db_insert_grievance(ctx, target_id, actor_id, event_id, severity, timestamp);
            }

            // Hurt relationship
            if (has_rel) {
                db_update_relationship_sentiment(ctx, rel.id,
                    clamp(rel.sentiment - rnd_range(10, 25), -100, 100));
            } else {
                // Create new strained relationship
                db_insert_relationship(ctx, actor_id, target_id,
                    relationship_type::ACQUAINTANCE, relationship_subtype::NONE,
                    rnd_range(10, 30), rnd_range(-30, -10));
            }

            // Reduce target's status need
            target.need_status = clamp(target.need_status - 5, 0, NEED_MAX);
            db_update_actor_needs(ctx, target_id, target.need_money, target.need_belonging,
                                 target.need_status, target.need_security);

        } else {
            // Threat - rare but serious
            i64 event_id = db_insert_event(ctx, timestamp, event_type::THREAT,
                           actor_id, target_id, location_id, "Made a threat");

            // Threats always create grievances - they're serious enough
            i32 severity = rnd_range(40, 70);
            db_insert_grievance(ctx, target_id, actor_id, event_id, severity, timestamp);

            // Severely hurt relationship
            if (has_rel) {
                db_update_relationship_sentiment(ctx, rel.id,
                    clamp(rel.sentiment - rnd_range(20, 40), -100, 100));
            } else {
                db_insert_relationship(ctx, actor_id, target_id,
                    relationship_type::ACQUAINTANCE, relationship_subtype::NONE,
                    rnd_range(10, 30), rnd_range(-50, -30));
            }

            // Reduce target's security
            target.need_security = clamp(target.need_security - 15, 0, NEED_MAX);
            db_update_actor_needs(ctx, target_id, target.need_money, target.need_belonging,
                                 target.need_status, target.need_security);
        }
    }
}

//------------------------------------------------------------------------------
// Work Events
//------------------------------------------------------------------------------

static void simulate_work(sim_context* ctx, i64 actor_id, i64 location_id, i32 timestamp) {
    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return;

    // Work generates income (simplified: money need increases)
    a.need_money = clamp(a.need_money + 2, 0, NEED_MAX);

    // Random work events
    f32 roll = rnd_float();

    if (roll < 0.01f) {
        // Got fired!
        i64 event_id = db_insert_event(ctx, timestamp, event_type::FIRED,
                       actor_id, 0, location_id, "Was fired from job");

        // Major hit to money and status
        a.need_money = clamp(a.need_money - 30, 0, NEED_MAX);
        a.need_status = clamp(a.need_status - 20, 0, NEED_MAX);
        a.need_security = clamp(a.need_security - 25, 0, NEED_MAX);

        // Find who the boss might be
        i64 others[16];
        i32 num_others = db_get_actors_at_location(ctx, location_id, timestamp, others, 16);
        for (i32 i = 0; i < num_others; i++) {
            relationship rel;
            if (db_get_relationship_between(ctx, actor_id, others[i], &rel)) {
                if (rel.subtype == relationship_subtype::BOSS ||
                    rel.subtype == relationship_subtype::EMPLOYEE) {
                    // Blame the boss
                    db_insert_grievance(ctx, actor_id, others[i], event_id,
                                       rnd_range(50, 80), timestamp);
                    break;
                }
            }
        }

    } else if (roll < 0.02f) {
        // Got promoted!
        db_insert_event(ctx, timestamp, event_type::PROMOTED,
                       actor_id, 0, location_id, "Was promoted");

        a.need_money = clamp(a.need_money + 20, 0, NEED_MAX);
        a.need_status = clamp(a.need_status + 25, 0, NEED_MAX);

    } else if (roll < 0.05f) {
        // Coworker conflict
        i64 others[16];
        i32 num_others = db_get_actors_at_location(ctx, location_id, timestamp, others, 16);
        if (num_others > 0) {
            i64 target = others[rnd_int(num_others)];
            if (target != actor_id) {
                simulate_social_interaction(ctx, actor_id, location_id, timestamp);
            }
        }
    }

    db_update_actor_needs(ctx, actor_id, a.need_money, a.need_belonging,
                         a.need_status, a.need_security);
}

//------------------------------------------------------------------------------
// Economic Events
//------------------------------------------------------------------------------

static void simulate_economic_pressure(sim_context* ctx, i64 actor_id, i32 timestamp) {
    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return;

    // If money is critically low, desperate actions become possible
    if (a.need_money < NEED_CRITICAL) {
        // Try to borrow money from friends/family
        relationship rels[16];
        i32 num_rels = db_get_relationships_for_actor(ctx, actor_id, rels, 16);

        for (i32 i = 0; i < num_rels; i++) {
            if (rels[i].sentiment > 30 && rnd_float() < 0.2f) {
                i64 lender_id = (rels[i].actor1_id == actor_id) ?
                               rels[i].actor2_id : rels[i].actor1_id;

                i64 event_id = db_insert_event(ctx, timestamp, event_type::BORROWED_MONEY,
                               actor_id, lender_id, 0, "Borrowed money");

                a.need_money = clamp(a.need_money + 20, 0, NEED_MAX);
                db_update_actor_needs(ctx, actor_id, a.need_money, a.need_belonging,
                                     a.need_status, a.need_security);

                // Creates obligation - if not repaid, will create grievance later
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
// Main Simulation Step
//------------------------------------------------------------------------------

static void simulate_actor_hour(sim_context* ctx, i64 actor_id, i32 timestamp) {
    i32 hour = timestamp_to_hour(timestamp);

    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return;

    // Get current location
    i64 current_loc = db_get_actor_location_at(ctx, actor_id, timestamp);
    if (current_loc == 0) {
        current_loc = a.home_id;  // Default to home
    }

    // Select action
    action_option action = select_action(ctx, a, current_loc, hour);

    // Execute action
    switch (action.type) {
        case action_type::GO_WORK:
        case action_type::GO_HOME:
        case action_type::GO_SOCIALIZE:
        case action_type::GO_SHOPPING:
            // Move to new location
            if (action.location_id != current_loc && action.location_id > 0) {
                db_update_actor_location(ctx, actor_id, action.location_id,
                                        timestamp, "moving");

                event_type evt = (action.type == action_type::GO_WORK) ?
                                event_type::GO_TO_WORK :
                                (action.type == action_type::GO_HOME) ?
                                event_type::GO_HOME : event_type::ENTER_LOCATION;

                db_insert_event(ctx, timestamp, evt, actor_id, 0,
                               action.location_id, nullptr);

                current_loc = action.location_id;
            }
            break;

        case action_type::STAY:
            // Already at location
            break;

        default:
            break;
    }

    // Location-based activities
    if (current_loc == a.work_id && is_work_hours(hour)) {
        simulate_work(ctx, actor_id, current_loc, timestamp);
    }

    // Social interactions at social locations
    location loc;
    if (db_get_location(ctx, current_loc, &loc)) {
        if (loc.type == location_type::BAR ||
            loc.type == location_type::RESTAURANT ||
            loc.type == location_type::NIGHTCLUB) {

            if (rnd_float() < 0.3f) {
                simulate_social_interaction(ctx, actor_id, current_loc, timestamp);
            }
        }
    }

    // Economic pressure check (once per day, in morning)
    if (hour == 8) {
        simulate_economic_pressure(ctx, actor_id, timestamp);
    }
}

//------------------------------------------------------------------------------
// Main Simulation Loop
//------------------------------------------------------------------------------

bool phase_simulation(sim_context* ctx) {
    printf("[PHASE 3] Simulation - Running daily life...\n");

    // Determine simulation length
    ctx->total_days = rnd_range(SIM_DAYS_MIN, SIM_DAYS_MAX);
    printf("  Simulating %d days...\n", ctx->total_days);

    // Get all actors
    actor actors[256];
    i32 num_actors = db_get_all_actors(ctx, actors, 256);

    // Initialize actor locations (everyone starts at home)
    for (i32 i = 0; i < num_actors; i++) {
        db_update_actor_location(ctx, actors[i].id, actors[i].home_id,
                                0, "sleeping");
    }

    // Simulation loop
    i32 decision_hours[] = { 7, 9, 12, 14, 17, 19, 21, 23 };  // Key decision points
    i32 num_decision_hours = sizeof(decision_hours) / sizeof(decision_hours[0]);

    for (i32 day = 0; day < ctx->total_days; day++) {
        ctx->current_day = day;

        // Progress indicator
        if (day % 10 == 0) {
            printf("    Day %d/%d...\n", day, ctx->total_days);
        }

        for (i32 h = 0; h < num_decision_hours; h++) {
            i32 hour = decision_hours[h];
            ctx->current_hour = hour;
            i32 timestamp = make_timestamp(day, hour);

            // Each actor takes action
            for (i32 a = 0; a < num_actors; a++) {
                simulate_actor_hour(ctx, actors[a].id, timestamp);
            }

            // Check crime potential for each actor
            for (i32 a = 0; a < num_actors; a++) {
                if (check_crime_potential(ctx, actors[a].id, timestamp)) {
                    // Crime occurred - continue simulation anyway
                }
            }

            // Decay needs (simplified: once per day at midnight)
            if (hour == 23) {
                for (i32 a = 0; a < num_actors; a++) {
                    actor act;
                    if (db_get_actor(ctx, actors[a].id, &act)) {
                        decay_needs(act, 24);
                        db_update_actor_needs(ctx, actors[a].id,
                            act.need_money, act.need_belonging,
                            act.need_status, act.need_security);
                    }
                }
            }
        }

        // Refresh actor data periodically
        if (day % 10 == 0) {
            num_actors = db_get_all_actors(ctx, actors, 256);
        }
    }

    printf("  Simulation complete. %d crimes occurred.\n", ctx->num_crimes);
    return true;
}
