#include "sim_db.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

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

static i32 clamp(i32 val, i32 min, i32 max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

//------------------------------------------------------------------------------
// Motive Calculation
//------------------------------------------------------------------------------

struct crime_motive {
    motive_type type;
    i64 target_id;
    f32 strength;
};

static crime_motive calculate_strongest_motive(sim_context* ctx, i64 actor_id) {
    crime_motive best = { motive_type::NONE, 0, 0.0f };

    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return best;

    // Check grievances for revenge motive
    grievance grievances[32];
    i32 num_grievances = db_get_active_grievances(ctx, actor_id, grievances, 32);

    for (i32 i = 0; i < num_grievances; i++) {
        f32 strength = (f32)grievances[i].severity;

        // Aggression amplifies grievances
        strength *= (1.0f + a.aggression * 0.01f);

        // Multiple grievances against same person stack
        i32 total_severity = db_get_total_grievance_severity(ctx, actor_id, grievances[i].offender_id);
        if (total_severity > grievances[i].severity) {
            strength = (f32)total_severity * (1.0f + a.aggression * 0.01f);
        }

        if (strength > best.strength) {
            best.type = motive_type::REVENGE;
            best.target_id = grievances[i].offender_id;
            best.strength = strength;
        }
    }

    // Check for financial motive
    if (a.need_money < NEED_CRITICAL) {
        f32 desperation = (f32)(NEED_CRITICAL - a.need_money) * 2.0f;
        desperation *= (1.0f + a.greed * 0.01f);

        // Find wealthy target
        i64 wealthy_ids[32];
        i32 num_wealthy = db_get_wealthy_actors(ctx, 7, wealthy_ids, 32);

        for (i32 i = 0; i < num_wealthy; i++) {
            if (wealthy_ids[i] == actor_id) continue;

            actor target;
            if (!db_get_actor(ctx, wealthy_ids[i], &target)) continue;

            f32 strength = desperation + target.income * 5.0f;

            // Relationship affects targeting
            relationship rel;
            if (db_get_relationship_between(ctx, actor_id, wealthy_ids[i], &rel)) {
                if (rel.sentiment < 0) {
                    strength *= 1.5f;  // Disliked wealthy person is better target
                } else if (rel.type == relationship_type::FAMILY) {
                    // Inheritance motive!
                    if (target.age > 50) {
                        if (strength > best.strength || best.type != motive_type::INHERITANCE) {
                            best.type = motive_type::INHERITANCE;
                            best.target_id = wealthy_ids[i];
                            best.strength = strength * 1.3f;
                        }
                    }
                }
            }

            if (strength > best.strength && best.type != motive_type::INHERITANCE) {
                best.type = motive_type::FINANCIAL_GAIN;
                best.target_id = wealthy_ids[i];
                best.strength = strength;
            }
        }
    }

    // Check for jealousy motive (romantic triangles)
    relationship rels[32];
    i32 num_rels = db_get_relationships_for_actor(ctx, actor_id, rels, 32);

    for (i32 i = 0; i < num_rels; i++) {
        if (rels[i].type == relationship_type::ROMANTIC) {
            if (rels[i].subtype == relationship_subtype::EX_PARTNER) {
                // Ex with negative sentiment
                if (rels[i].sentiment < -30) {
                    f32 strength = (f32)(-rels[i].sentiment) * 0.8f;
                    strength *= (1.0f + a.aggression * 0.01f);

                    i64 target = (rels[i].actor1_id == actor_id) ?
                                rels[i].actor2_id : rels[i].actor1_id;

                    if (strength > best.strength) {
                        best.type = motive_type::JEALOUSY;
                        best.target_id = target;
                        best.strength = strength;
                    }
                }
            }
        }
    }

    // Check for rage (recent severe insult)
    event events[16];
    i32 current_ts = make_timestamp(ctx->current_day, ctx->current_hour);
    i32 recent_ts = current_ts - 48;  // Last 2 days

    i32 num_events = db_get_events_for_actor(ctx, actor_id, events, 16);
    for (i32 i = 0; i < num_events; i++) {
        if (events[i].timestamp >= recent_ts &&
            events[i].type == event_type::INSULT &&
            events[i].target_id == actor_id) {

            // Base rage is lower; only extremely aggressive/impulsive people escalate
            f32 strength = 25.0f * (1.0f + a.impulsivity * 0.02f);
            strength *= (1.0f + a.aggression * 0.02f);

            if (strength > best.strength) {
                best.type = motive_type::RAGE;
                best.target_id = events[i].actor_id;
                best.strength = strength;
            }
        }
    }

    return best;
}

//------------------------------------------------------------------------------
// Opportunity Calculation
//------------------------------------------------------------------------------

static f32 calculate_opportunity(sim_context* ctx, i64 actor_id, i64 target_id, i32 timestamp) {
    f32 opportunity = 0.0f;

    // Check if actor and target are at same location
    i64 actor_loc = db_get_actor_location_at(ctx, actor_id, timestamp);
    i64 target_loc = db_get_actor_location_at(ctx, target_id, timestamp);

    if (actor_loc == 0 || target_loc == 0) return 0.0f;

    // Same location is necessary for murder/kidnap
    if (actor_loc == target_loc) {
        opportunity += 30.0f;

        // Check location properties
        location loc;
        if (db_get_location(ctx, actor_loc, &loc)) {
            // Private locations are better
            if (!loc.is_public) {
                opportunity += 30.0f;
            }

            // Low security is better
            opportunity += (3 - (i32)loc.security) * 10.0f;

            // Night time is better
            i32 hour = timestamp_to_hour(timestamp);
            if (hour >= 22 || hour < 6) {
                opportunity += 20.0f;
            }
        }

        // Check for witnesses
        i64 others[32];
        i32 num_others = db_get_actors_at_location(ctx, actor_loc, timestamp, others, 32);
        i32 witness_count = num_others - 2;  // Exclude actor and target

        if (witness_count <= 0) {
            opportunity += 30.0f;  // No witnesses!
        } else {
            opportunity -= witness_count * 10.0f;  // Each witness reduces opportunity
        }
    }

    return opportunity;
}

//------------------------------------------------------------------------------
// Capability Calculation
//------------------------------------------------------------------------------

static f32 calculate_capability(sim_context* ctx, i64 actor_id, crime_type type) {
    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return 0.0f;

    f32 capability = 50.0f;  // Base capability

    switch (type) {
        case crime_type::MURDER: {
            // Physical strength (age, sex as rough proxy)
            if (a.sex == 'M') capability += 10.0f;
            if (a.age >= 20 && a.age <= 50) capability += 10.0f;
            if (a.aggression > 50) capability += 10.0f;

            // Check if they've acquired a weapon (scouting events)
            i32 weapon_events = db_count_events_of_type(ctx, actor_id, event_type::ACQUIRE_WEAPON);
            if (weapon_events > 0) capability += 30.0f;
            break;
        }

        case crime_type::KIDNAPPING: {
            // Need accomplices or strength
            if (a.sex == 'M') capability += 10.0f;

            // Check for close relationships who might help
            relationship rels[16];
            i32 num_rels = db_get_relationships_for_actor(ctx, actor_id, rels, 16);
            for (i32 i = 0; i < num_rels; i++) {
                if (rels[i].strength > 70 && rels[i].sentiment > 60) {
                    capability += 20.0f;
                    break;
                }
            }
            break;
        }

        case crime_type::THEFT: {
            // Skills implied by occupation
            if (strcmp(a.occupation, "Security Guard") == 0) capability += 20.0f;
            if (strcmp(a.occupation, "Electrician") == 0) capability += 15.0f;
            if (strcmp(a.occupation, "Contractor") == 0) capability += 10.0f;

            // Greed helps with theft
            capability += a.greed * 0.2f;
            break;
        }

        default:
            break;
    }

    return capability;
}

//------------------------------------------------------------------------------
// Inhibition Calculation
//------------------------------------------------------------------------------

static f32 calculate_inhibition(sim_context* ctx, i64 actor_id, i64 target_id, crime_type type) {
    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return 100.0f;

    f32 inhibition = a.morality;  // Base inhibition from morality

    // Relationship affects inhibition
    relationship rel;
    if (db_get_relationship_between(ctx, actor_id, target_id, &rel)) {
        if (rel.type == relationship_type::FAMILY) {
            inhibition += 40.0f;  // Much harder to harm family
        } else if (rel.type == relationship_type::FRIEND) {
            inhibition += 20.0f;
        } else if (rel.sentiment > 50) {
            inhibition += 15.0f;
        }
    }

    // Crime type affects inhibition
    switch (type) {
        case crime_type::MURDER:
            inhibition *= 1.5f;  // Murder has highest inhibition
            break;
        case crime_type::KIDNAPPING:
            inhibition *= 1.3f;
            break;
        case crime_type::THEFT:
            inhibition *= 0.8f;  // Theft has lower inhibition
            break;
        default:
            break;
    }

    // Low loyalty reduces inhibition
    inhibition *= (0.5f + a.loyalty * 0.005f);

    return inhibition;
}

//------------------------------------------------------------------------------
// Crime Selection
//------------------------------------------------------------------------------

static crime_type select_crime_type(const crime_motive& motive, const actor& perpetrator) {
    switch (motive.type) {
        case motive_type::REVENGE:
        case motive_type::RAGE:
        case motive_type::JEALOUSY:
            // These motives lean toward murder
            if (perpetrator.aggression > 60) {
                return crime_type::MURDER;
            } else if (rnd_float() < 0.7f) {
                return crime_type::MURDER;
            }
            return crime_type::KIDNAPPING;

        case motive_type::FINANCIAL_GAIN:
            // Financial motives lean toward theft, but can escalate
            if (perpetrator.aggression > 70 && rnd_float() < 0.3f) {
                return crime_type::MURDER;  // Robbery gone wrong
            }
            return crime_type::THEFT;

        case motive_type::INHERITANCE:
            return crime_type::MURDER;  // Inheritance requires death

        case motive_type::SILENCING:
            return crime_type::MURDER;

        case motive_type::POWER:
            return (rnd_float() < 0.5f) ? crime_type::KIDNAPPING : crime_type::MURDER;

        default:
            return crime_type::THEFT;
    }
}

//------------------------------------------------------------------------------
// Crime Execution
//------------------------------------------------------------------------------

static void execute_crime(sim_context* ctx, i64 perpetrator_id, i64 victim_id,
                         crime_type type, motive_type motive, i32 timestamp) {
    actor perp;
    if (!db_get_actor(ctx, perpetrator_id, &perp)) return;

    i64 location_id = db_get_actor_location_at(ctx, perpetrator_id, timestamp);
    if (location_id == 0) {
        location_id = perp.home_id;
    }

    // Log the crime event
    event_type evt;
    const char* details;

    switch (type) {
        case crime_type::MURDER:
            evt = event_type::MURDER;
            details = "Committed murder";
            break;
        case crime_type::KIDNAPPING:
            evt = event_type::KIDNAPPING;
            details = "Kidnapped victim";
            break;
        case crime_type::THEFT:
            evt = event_type::THEFT;
            details = "Committed theft";
            break;
        default:
            return;
    }

    db_insert_event(ctx, timestamp, evt, perpetrator_id, victim_id, location_id, details);

    // Insert crime record
    i64 crime_id = db_insert_crime(ctx, type, perpetrator_id, victim_id,
                                   location_id, timestamp, motive);

    // Resolve grievances - the perpetrator has "dealt with" their grievance against the victim
    db_resolve_grievances_against(ctx, perpetrator_id, victim_id);

    actor victim;
    if (db_get_actor(ctx, victim_id, &victim)) {
        printf("    [CRIME] %s: %s -> %s (%s)\n",
               crime_type_str(type),
               perp.name,
               victim.name,
               motive_type_str(motive));
    }
}

//------------------------------------------------------------------------------
// Planning Phase
//------------------------------------------------------------------------------

static void do_crime_planning(sim_context* ctx, i64 actor_id, i64 target_id,
                             crime_type type, i32 timestamp) {
    // Log planning event
    db_insert_event(ctx, timestamp, event_type::PLANNING_CRIME,
                   actor_id, target_id, 0, "Planning crime");

    // Maybe acquire weapon for murder
    if (type == crime_type::MURDER && rnd_float() < 0.7f) {
        db_insert_event(ctx, timestamp, event_type::ACQUIRE_WEAPON,
                       actor_id, 0, 0, "Acquired weapon");
    }

    // Scout the target location
    db_insert_event(ctx, timestamp, event_type::SCOUTING,
                   actor_id, target_id, 0, "Scouting target");
}

//------------------------------------------------------------------------------
// Main Crime Check (called from simulation)
//------------------------------------------------------------------------------

// Cooldown: actors who committed a crime won't commit another for this many hours
constexpr i32 CRIME_COOLDOWN_HOURS = 24 * 14;  // 2 weeks

bool check_crime_potential(sim_context* ctx, i64 actor_id, i32 timestamp) {
    actor a;
    if (!db_get_actor(ctx, actor_id, &a)) return false;

    // Cooldown check: skip actors who recently committed a crime
    if (db_has_recent_crime(ctx, actor_id, timestamp, CRIME_COOLDOWN_HOURS)) {
        return false;
    }

    // Calculate strongest motive
    crime_motive motive = calculate_strongest_motive(ctx, actor_id);

    if (motive.type == motive_type::NONE || motive.target_id == 0) {
        return false;
    }

    // Determine crime type
    crime_type type = select_crime_type(motive, a);

    // Calculate scores
    f32 opportunity = calculate_opportunity(ctx, actor_id, motive.target_id, timestamp);
    f32 capability = calculate_capability(ctx, actor_id, type);
    f32 inhibition = calculate_inhibition(ctx, actor_id, motive.target_id, type);

    // Crime potential formula
    f32 crime_score = motive.strength + opportunity + capability - inhibition;

    // Get threshold for crime type
    f32 threshold;
    switch (type) {
        case crime_type::MURDER:   threshold = (f32)CRIME_THRESHOLD_MURDER; break;
        case crime_type::KIDNAPPING: threshold = (f32)CRIME_THRESHOLD_KIDNAP; break;
        case crime_type::THEFT:    threshold = (f32)CRIME_THRESHOLD_THEFT; break;
        default: threshold = 200.0f;
    }

    // Add randomness based on impulsivity
    f32 impulsivity_bonus = a.impulsivity * rnd_float() * 0.5f;
    crime_score += impulsivity_bonus;

    if (crime_score > threshold) {
        // Planning or immediate execution based on impulsivity
        if (a.impulsivity > 70 || rnd_float() < 0.3f) {
            // Immediate execution
            execute_crime(ctx, actor_id, motive.target_id, type, motive.type, timestamp);
            return true;
        } else {
            // Start planning (might execute in next few days)
            do_crime_planning(ctx, actor_id, motive.target_id, type, timestamp);

            // Schedule execution in 1-5 days
            i32 delay_days = rnd_range(1, 5);
            i32 exec_timestamp = timestamp + delay_days * SIM_HOURS_PER_DAY + rnd_range(18, 23);

            // Store in context or execute later
            // For simplicity, we'll check again in future cycles
        }
    }

    return false;
}
