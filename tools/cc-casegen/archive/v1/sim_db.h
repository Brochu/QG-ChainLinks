#pragma once

#include "sim_types.h"
#include <sqlite3.h>

//------------------------------------------------------------------------------
// Database Lifecycle
//------------------------------------------------------------------------------

// Initialize in-memory database with all tables
bool db_init(sim_context* ctx);

// Export database to file for debugging
bool db_export(sim_context* ctx, const char* filepath);

// Close and cleanup
void db_close(sim_context* ctx);

//------------------------------------------------------------------------------
// District Operations
//------------------------------------------------------------------------------

i64 db_insert_district(sim_context* ctx, const char* name, district_type type,
                       i32 wealth_level, i32 crime_rate);

bool db_get_district(sim_context* ctx, i64 id, district* out);
i32 db_get_all_districts(sim_context* ctx, district* out, i32 max_count);

//------------------------------------------------------------------------------
// Location Operations
//------------------------------------------------------------------------------

i64 db_insert_location(sim_context* ctx, i64 district_id, const char* name,
                       location_type type, bool is_public, i32 open_hour,
                       i32 close_hour, security_level security, i32 capacity);

bool db_get_location(sim_context* ctx, i64 id, location* out);
i32 db_get_locations_by_district(sim_context* ctx, i64 district_id,
                                  location* out, i32 max_count);
i32 db_get_locations_by_type(sim_context* ctx, location_type type,
                              location* out, i32 max_count);
i32 db_count_locations(sim_context* ctx);

//------------------------------------------------------------------------------
// Actor Operations
//------------------------------------------------------------------------------

i64 db_insert_actor(sim_context* ctx, const actor* a);
bool db_update_actor_needs(sim_context* ctx, i64 id, i32 money, i32 belonging,
                           i32 status, i32 security);
bool db_get_actor(sim_context* ctx, i64 id, actor* out);
i32 db_get_all_actors(sim_context* ctx, actor* out, i32 max_count);
i32 db_get_actors_at_location(sim_context* ctx, i64 location_id, i32 timestamp,
                               i64* out_ids, i32 max_count);

//------------------------------------------------------------------------------
// Relationship Operations
//------------------------------------------------------------------------------

i64 db_insert_relationship(sim_context* ctx, i64 actor1, i64 actor2,
                           relationship_type type, relationship_subtype subtype,
                           i32 strength, i32 sentiment);
bool db_update_relationship_sentiment(sim_context* ctx, i64 id, i32 new_sentiment);
i32 db_get_relationships_for_actor(sim_context* ctx, i64 actor_id,
                                    relationship* out, i32 max_count);
bool db_get_relationship_between(sim_context* ctx, i64 actor1, i64 actor2,
                                  relationship* out);

//------------------------------------------------------------------------------
// Event Operations
//------------------------------------------------------------------------------

i64 db_insert_event(sim_context* ctx, i32 timestamp, event_type type,
                    i64 actor_id, i64 target_id, i64 location_id,
                    const char* details);
i32 db_get_events_for_actor(sim_context* ctx, i64 actor_id,
                             event* out, i32 max_count);
i32 db_get_events_at_location(sim_context* ctx, i64 location_id, i32 time_start,
                               i32 time_end, event* out, i32 max_count);
i32 db_get_events_in_range(sim_context* ctx, i32 time_start, i32 time_end,
                            event* out, i32 max_count);
bool db_get_event(sim_context* ctx, i64 id, event* out);

//------------------------------------------------------------------------------
// Grievance Operations
//------------------------------------------------------------------------------

i64 db_insert_grievance(sim_context* ctx, i64 victim_id, i64 offender_id,
                        i64 cause_event_id, i32 severity, i32 timestamp);
bool db_resolve_grievance(sim_context* ctx, i64 id);
i32 db_get_grievances_for_actor(sim_context* ctx, i64 victim_id,
                                 grievance* out, i32 max_count);
i32 db_get_active_grievances(sim_context* ctx, i64 victim_id,
                              grievance* out, i32 max_count);
i32 db_get_total_grievance_severity(sim_context* ctx, i64 victim_id, i64 offender_id);

//------------------------------------------------------------------------------
// Crime Operations
//------------------------------------------------------------------------------

i64 db_insert_crime(sim_context* ctx, crime_type type, i64 perpetrator_id,
                    i64 victim_id, i64 location_id, i32 timestamp,
                    motive_type motive);
bool db_update_crime_score(sim_context* ctx, i64 id, f32 score);
i32 db_get_all_crimes(sim_context* ctx, crime* out, i32 max_count);
bool db_get_crime(sim_context* ctx, i64 id, crime* out);

//------------------------------------------------------------------------------
// Evidence Operations
//------------------------------------------------------------------------------

i64 db_insert_evidence(sim_context* ctx, i64 crime_id, evidence_type type,
                       i64 location_id, i64 points_to_id, i32 clarity,
                       const char* description);
i32 db_get_evidence_for_crime(sim_context* ctx, i64 crime_id,
                               evidence* out, i32 max_count);

//------------------------------------------------------------------------------
// Witness Operations
//------------------------------------------------------------------------------

i64 db_insert_witness(sim_context* ctx, i64 crime_id, i64 actor_id,
                      const char* saw_what, i32 reliability, i32 willingness);
i32 db_get_witnesses_for_crime(sim_context* ctx, i64 crime_id,
                                witness* out, i32 max_count);

//------------------------------------------------------------------------------
// Actor State Tracking (for simulation)
//------------------------------------------------------------------------------

bool db_update_actor_location(sim_context* ctx, i64 actor_id, i64 location_id,
                               i32 timestamp, const char* activity);
i64 db_get_actor_location_at(sim_context* ctx, i64 actor_id, i32 timestamp);

//------------------------------------------------------------------------------
// Query Helpers
//------------------------------------------------------------------------------

// Get actors who have grievances against a specific actor
i32 db_get_actors_with_grudge_against(sim_context* ctx, i64 target_id,
                                       i64* out_ids, i32 max_count);

// Get wealthy actors (for theft targeting)
i32 db_get_wealthy_actors(sim_context* ctx, i32 min_income,
                           i64* out_ids, i32 max_count);

// Count events of a specific type for an actor
i32 db_count_events_of_type(sim_context* ctx, i64 actor_id, event_type type);

// Check if actor committed a crime within cooldown_hours of current_timestamp
bool db_has_recent_crime(sim_context* ctx, i64 actor_id, i32 current_timestamp, i32 cooldown_hours);

// Resolve all grievances that victim_id has against offender_id
bool db_resolve_grievances_against(sim_context* ctx, i64 victim_id, i64 offender_id);
