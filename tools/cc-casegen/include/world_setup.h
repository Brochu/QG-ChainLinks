#pragma once
#include "shared_types.hpp"
#include "name_cycle.h"
#include "name_gen.h"

#include <sqlite3.h>

// ============================================================================
// ENUMS
// ============================================================================

enum city_size : i8 {
    CITY_SMALL,     // 50 actors
    CITY_MEDIUM,    // 80 actors
    CITY_LARGE,     // 120 actors
    CITY_METRO,     // 150 actors
    CITY_SIZE_COUNT
};

enum district_type : i8 {
    DISTRICT_RESIDENTIAL,
    DISTRICT_COMMERCIAL,
    DISTRICT_INDUSTRIAL,
    DISTRICT_NIGHTLIFE,
    DISTRICT_DOCKS,
    DISTRICT_FINANCIAL,
    DISTRICT_TYPE_COUNT
};

enum location_type : i8 {
    LOC_RESIDENCE,
    LOC_BUSINESS,
    LOC_PUBLIC,
    LOC_OUTDOOR,
    LOC_VEHICLE,
    LOC_TYPE_COUNT
};

enum access_control : i8 {
    ACCESS_NONE,
    ACCESS_STANDARD_LOCK,
    ACCESS_KEYPAD,
    ACCESS_SECURITY_STAFF,
    ACCESS_RESTRICTED,
    ACCESS_COUNT
};

enum operation_hours : i8 {
    HOURS_24_7,
    HOURS_BUSINESS,     // 9-5
    HOURS_EVENING,      // 6pm-2am
    HOURS_MORNING,      // 6am-12pm
    HOURS_COUNT
};

enum person_height : i8 {
    HEIGHT_VERY_SHORT,
    HEIGHT_SHORT,
    HEIGHT_AVERAGE,
    HEIGHT_TALL,
    HEIGHT_VERY_TALL,
    HEIGHT_COUNT
};

enum person_build : i8 {
    BUILD_FRAIL,
    BUILD_SLIM,
    BUILD_AVERAGE,
    BUILD_ATHLETIC,
    BUILD_HEAVY,
    BUILD_COUNT
};

enum hair_color : i8 {
    HAIR_BLACK,
    HAIR_BROWN,
    HAIR_BLONDE,
    HAIR_RED,
    HAIR_GRAY,
    HAIR_WHITE,
    HAIR_BALD,
    HAIR_COUNT
};

enum personality_archetype : i8 {
    ARCHETYPE_VOLATILE,     // High impulsivity, high aggression, low morality
    ARCHETYPE_GREEDY,       // High greed, low morality, average aggression
    ARCHETYPE_LOYAL,        // High loyalty, high morality, low greed
    ARCHETYPE_AVERAGE,      // Balanced traits
    ARCHETYPE_COUNT
};

enum relationship_type : i8 {
    REL_SPOUSE,
    REL_EX_SPOUSE,
    REL_PARTNER,
    REL_EX_PARTNER,
    REL_SIBLING,
    REL_FRIEND,
    REL_COLLEAGUE,
    REL_EMPLOYER,
    REL_EMPLOYEE,
    REL_RIVAL,
    REL_DEBTOR,
    REL_CREDITOR,
    REL_TYPE_COUNT
};

enum object_type : i8 {
    OBJ_WEAPON,
    OBJ_TOOL,
    OBJ_VEHICLE,
    OBJ_DOCUMENT,
    OBJ_PERSONAL_ITEM,
    OBJ_TYPE_COUNT
};

enum object_size : i8 {
    SIZE_SMALL,
    SIZE_MEDIUM,
    SIZE_LARGE,
    SIZE_COUNT
};

enum object_color : i8 {
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_GRAY,
    COLOR_RED,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_BROWN,
    COLOR_SILVER,
    COLOR_COUNT
};

enum object_material : i8 {
    MAT_METAL,
    MAT_WOOD,
    MAT_PLASTIC,
    MAT_GLASS,
    MAT_PAPER,
    MAT_FABRIC,
    MAT_LEATHER,
    MAT_COUNT
};

enum object_condition : i8 {
    COND_NEW,
    COND_GOOD,
    COND_WORN,
    COND_DAMAGED,
    COND_COUNT
};

// ============================================================================
// STRUCTS
// ============================================================================

struct district {
    i64 id;
    const char *name;
    district_type type;
    i32 wealth;         // 1-5
    i32 roughness;      // 1-5, affects crime factor
    i32 response_time;  // Police response in minutes
};

struct location {
    i64 id;
    i64 district_id;
    const char *name;
    const char *address;
    location_type type;
    i64 owner_id;           // -1 if no owner
    access_control access;
    operation_hours hours;
    i32 crime_factor;
};

struct person {
    i64 id;
    const char *name;       // First name only
    const char *alias;      // Nickname (null if none)
    char sex;               // 'M' or 'F'
    i32 age;
    person_height height;
    person_build build;
    hair_color hair;

    i64 home_location_id;
    i64 work_location_id;   // -1 if unemployed
    const char *occupation;
    i32 income;             // 1-10

    const char *phone_number;
    const char *org_affiliation;    // Can be null

    // Personality (0-100)
    i32 impulsivity;
    i32 morality;
    i32 greed;
    i32 aggression;
    i32 loyalty;

    // Needs (0-100, lower = more desperate)
    i32 need_money;
    i32 need_belonging;
    i32 need_status;
    i32 need_security;

    personality_archetype archetype;
};

struct relationship {
    i64 person1_id;
    i64 person2_id;
    relationship_type type;
    i32 strength;           // 0-100, how strong the relationship is
};

struct sim_object {
    i64 id;
    const char *name;
    object_type type;
    const char *serial_number;  // Can be null
    object_size size;
    object_color color;
    object_material material;
    object_condition condition;
    i64 owner_id;           // -1 if no owner
    i64 location_id;        // -1 if with owner
};

// ============================================================================
// WORLD CONTEXT
// ============================================================================

struct world_ctx {
    sqlite3 *db;

    // Name generators
    name_cycle male_names;
    name_cycle female_names;
    name_gen district_names;    // Markov chain for districts
    name_gen street_names;      // Markov chain for streets

    city_size size;

    district *districts;
    i32 num_districts;

    location *locations;
    i32 num_locations;

    person *people;
    i32 num_people;

    relationship *relationships;
    i32 num_relationships;

    sim_object *objects;
    i32 num_objects;

    mem_arena _scratch;
};

// ============================================================================
// INTERFACE
// ============================================================================

// Initialize world context (opens in-memory SQLite DB, loads name CSVs)
void world_init(world_ctx *ctx, i32 seed);

// Free all resources. If db_save_path is not null, saves DB to file first.
void world_clear(world_ctx *ctx, const char *db_save_path = nullptr);

// Generate world for given city size
void world_generate(world_ctx *ctx, city_size size);

// Debug: print world summary
void world_print_summary(world_ctx *ctx);
