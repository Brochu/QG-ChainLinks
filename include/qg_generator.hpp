#pragma once

#include <sqlite3.h>

#include "qg_types.hpp"
#include "qg_memory.hpp"

#define MAX_NUM_NAMES 255

//TODO: Each name_gen should have it's own memory pool for names
struct name_gen_entry {
    i8 num_options = 0;

    char options[32];
    u8 counts[32];
};

struct name_gen {
    name_gen_entry *table = nullptr;
    const char *names[MAX_NUM_NAMES];
    u8 num_names = 0;

    mem_arena _names_mem = {};
};

void name_gen_train(name_gen *gen, const char *file_path);
void name_gen_clear(name_gen *gen);

void name_gen_next(name_gen *gen, u64 num);
void name_gen_district(name_gen *gen, u64 num);

struct name_cycle {
    const char *names[MAX_NUM_NAMES * 2 * 2];
    i16 num_names = 0;

    u64 step;
    u64 next;
    mem_arena _mem;
};

void name_cycle_init(name_cycle *ctx, const char *file_path);
void name_cycle_clear(name_cycle *ctx);
const char *name_cycle_next(name_cycle *ctx);

// CASE GENERATOR ====================

enum city_size : i8 { SIZE_SMALL, SIZE_MEDIUM, SIZE_LARGE, SIZE_METRO, SIZE_COUNT };
enum district_type : i8 { RESIDENTIAL, COMMERCIAL, INDUSTRIAL, NIGHTLIFE, DOCKS, FINANCIAL, DISTRICT_COUNT };
enum landmark_type : i8 {
    LANDMARK_TYPE_RES_START,
    LANDMARK_TYPE_RES_HOUSE = LANDMARK_TYPE_RES_START,
    LANDMARK_TYPE_RES_APARTMENT,
    LANDMARK_TYPE_RES_CLINIC,
    LANDMARK_TYPE_RES_CORNERSTORE,
    LANDMARK_TYPE_RES_PARK,
    LANDMARK_TYPE_RES_CHURCH,
    LANDMARK_TYPE_RES_END,

    LANDMARK_TYPE_COMM_START = LANDMARK_TYPE_RES_END,
    LANDMARK_TYPE_COMM_RESTAURANT = LANDMARK_TYPE_COMM_START,
    LANDMARK_TYPE_COMM_BAR,
    LANDMARK_TYPE_COMM_HOTEL,
    LANDMARK_TYPE_COMM_STORE,
    LANDMARK_TYPE_COMM_BANK,
    LANDMARK_TYPE_COMM_APARTMENT,
    LANDMARK_TYPE_COMM_END,

    LANDMARK_TYPE_INDU_START = LANDMARK_TYPE_COMM_END,
    LANDMARK_TYPE_INDU_WAREHOUSE = LANDMARK_TYPE_INDU_START,
    LANDMARK_TYPE_INDU_FACTORY,
    LANDMARK_TYPE_INDU_ABANDONED,
    LANDMARK_TYPE_INDU_END,

    LANDMARK_TYPE_NIGHT_START = LANDMARK_TYPE_INDU_END,
    LANDMARK_TYPE_NIGHT_BAR = LANDMARK_TYPE_NIGHT_START,
    LANDMARK_TYPE_NIGHT_NIGHTCLUB,
    LANDMARK_TYPE_NIGHT_CASINO,
    LANDMARK_TYPE_NIGHT_STRIPCLUB,
    LANDMARK_TYPE_NIGHT_FASTFOOD,
    LANDMARK_TYPE_NIGHT_END,

    LANDMARK_TYPE_DOCKS_START = LANDMARK_TYPE_NIGHT_END,
    LANDMARK_TYPE_DOCKS_WAREHOUSE = LANDMARK_TYPE_DOCKS_START,
    LANDMARK_TYPE_DOCKS_SHIPYARD,
    LANDMARK_TYPE_DOCKS_BAR,
    LANDMARK_TYPE_DOCKS_HOTEL,
    LANDMARK_TYPE_DOCKS_END,

    LANDMARK_TYPE_FIN_START = LANDMARK_TYPE_DOCKS_END,
    LANDMARK_TYPE_FIN_BANK = LANDMARK_TYPE_FIN_START,
    LANDMARK_TYPE_FIN_OFFICE,
    LANDMARK_TYPE_FIN_HOTEL,
    LANDMARK_TYPE_FIN_COURTHOUSE,
    LANDMARK_TYPE_FIN_END,

    LANDMARK_TYPE_COUNT,
};
enum landmark_size : i8 {
    LANDMARK_SIZE_TINY,
    LANDMARK_SIZE_SMALL,
    LANDMARK_SIZE_MEDIUM,
    LANDMARK_SIZE_LARGE,
    LANDMARK_SIZE_COUNT,
};

enum travel_mode : i8 {
    TRAVEL_WALK,
    TRAVEL_DRIVE,
    TRAVEL_COUNT,
};

enum travel_phase : i8 {
    TIME_DAY,
    TIME_NIGHT,
    TIME_RUSH,
    TIME_COUNT,
};

struct district {
    i64 id;
    const char *name;
    district_type type;
    i32 wealth;
    i32 roughness;
    i32 response_time;
};

struct landmark {
    i64 id;
    i64 district_id;
    const char *name;
    landmark_type type;
    landmark_size size;
    i32 open_hour;
    i32 close_hour;
    i32 peak_hour;
    i32 num_staff;
    bool is_public;
    i32 crime_factor;
};

struct case_gen {
    sqlite3 *db;
    name_gen dist_names;

    city_size size;
    i8 num_districts;
    i8 num_landmarks;
};

void case_gen_init(case_gen *ctx);

void case_gen_fondation(case_gen *ctx, city_size s, i32 seed);
void case_gen_population(case_gen *ctx);
void case_gen_motive(case_gen *ctx);
void case_gen_crime(case_gen *ctx);
void case_gen_planning(case_gen *ctx);
void case_gen_exec(case_gen *ctx);
void case_gen_hook(case_gen *ctx);
void case_gen_polish(case_gen *ctx);

void case_gen_clear(case_gen *ctx);
