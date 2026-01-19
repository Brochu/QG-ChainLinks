#pragma once

#include "shared_types.hpp"
#include <cstddef>

//------------------------------------------------------------------------------
// Configuration Constants
//------------------------------------------------------------------------------

constexpr i32 SIM_HOURS_PER_DAY = 24;
constexpr i32 SIM_DAYS_MIN = 60;
constexpr i32 SIM_DAYS_MAX = 90;

// City size ranges
constexpr i32 CITY_SMALL_ACTORS = 50;
constexpr i32 CITY_MEDIUM_ACTORS = 80;
constexpr i32 CITY_LARGE_ACTORS = 120;
constexpr i32 CITY_METRO_ACTORS = 150;

constexpr i32 CITY_SMALL_DISTRICTS = 3;
constexpr i32 CITY_MEDIUM_DISTRICTS = 5;
constexpr i32 CITY_LARGE_DISTRICTS = 6;
constexpr i32 CITY_METRO_DISTRICTS = 8;

// Need thresholds
constexpr i32 NEED_MAX = 100;
constexpr i32 NEED_CRITICAL = 20;  // Below this, actor is desperate

// Crime thresholds (higher = fewer crimes)
constexpr i32 CRIME_THRESHOLD_MURDER = 250;
constexpr i32 CRIME_THRESHOLD_KIDNAP = 200;
constexpr i32 CRIME_THRESHOLD_THEFT = 150;

// Causality chain limits (higher = richer narrative required)
constexpr i32 CHAIN_MIN_LENGTH = 7;
constexpr i32 CHAIN_MAX_LENGTH = 10;

//------------------------------------------------------------------------------
// Enumerations
//------------------------------------------------------------------------------

enum class city_size : i8 {
    SMALL,
    MEDIUM,
    LARGE,
    METRO
};

enum class district_type : i8 {
    RESIDENTIAL,
    COMMERCIAL,
    INDUSTRIAL,
    FINANCIAL,
    NIGHTLIFE,
    MIXED
};

enum class location_type : i8 {
    // Residential
    HOUSE,
    APARTMENT,

    // Commercial
    OFFICE,
    STORE,
    RESTAURANT,
    BAR,
    HOTEL,
    BANK,

    // Industrial
    WAREHOUSE,
    FACTORY,

    // Public
    PARK,
    CHURCH,
    HOSPITAL,
    POLICE_STATION,

    // Nightlife
    NIGHTCLUB,
    CASINO,

    COUNT
};

enum class security_level : i8 {
    NONE = 0,      // No security
    CAMERAS = 1,   // Security cameras
    GUARDS = 2,    // Guards + cameras
    VAULT = 3      // Maximum security
};

enum class relationship_type : i8 {
    FAMILY,
    FRIEND,
    COWORKER,
    ROMANTIC,
    RIVAL,
    ACQUAINTANCE
};

enum class relationship_subtype : i8 {
    // Family
    SPOUSE,
    SIBLING,
    PARENT,
    CHILD,
    COUSIN,

    // Romantic
    PARTNER,
    EX_PARTNER,
    AFFAIR,

    // Professional
    BOSS,
    EMPLOYEE,
    PEER,

    // Social
    CLOSE_FRIEND,
    CASUAL_FRIEND,
    NEIGHBOR,

    // Negative
    ENEMY,
    COMPETITOR,

    NONE
};

enum class event_type : i16 {
    // Routine events
    WAKE_UP,
    GO_TO_WORK,
    ARRIVE_WORK,
    LEAVE_WORK,
    GO_HOME,
    SLEEP,

    // Location events
    ENTER_LOCATION,
    EXIT_LOCATION,

    // Social events
    CONVERSATION,
    ARGUMENT,
    INSULT,
    THREAT,
    FAVOR_DONE,
    FAVOR_RECEIVED,
    BETRAYAL,
    RECONCILIATION,

    // Economic events
    PAID,
    SPENT_MONEY,
    BORROWED_MONEY,
    LENT_MONEY,
    DEBT_UNPAID,
    FIRED,
    HIRED,
    PROMOTED,
    DEMOTED,

    // Crime-related
    WITNESSED_SOMETHING,
    ACQUIRE_WEAPON,
    SCOUTING,
    PLANNING_CRIME,

    // Major crimes
    MURDER,
    KIDNAPPING,
    THEFT,

    // Post-crime
    DISCOVERED_BODY,
    REPORTED_MISSING,
    DISCOVERED_THEFT,

    COUNT
};

enum class crime_type : i8 {
    NONE,
    MURDER,
    KIDNAPPING,
    THEFT
};

enum class motive_type : i8 {
    NONE,
    REVENGE,           // Grievance-based
    FINANCIAL_GAIN,    // Money need + target wealth
    JEALOUSY,          // Romantic rivalry
    SILENCING,         // Target knows something
    INHERITANCE,       // Family + money
    POWER,             // Status need
    RAGE               // Impulsive, from recent insult
};

enum class evidence_type : i8 {
    // Physical
    WEAPON,
    FINGERPRINT,
    DNA,
    FIBER,
    FOOTPRINT,

    // Testimonial
    WITNESS_STATEMENT,

    // Documentary
    RECEIPT,
    PHONE_RECORD,
    BANK_RECORD,
    EMAIL,
    PHOTO,
    VIDEO,
    SECURITY_FOOTAGE,

    // Other
    ALIBI,

    COUNT
};

enum class blank_type : i8 {
    WHO,      // Actor identity
    WHERE,    // Location
    WHEN,     // Time/date
    WHAT      // Object (weapon, vehicle, etc.)
};

//------------------------------------------------------------------------------
// Data Structures (for in-memory use, mirrors DB tables)
//------------------------------------------------------------------------------

struct district {
    i64 id;
    char name[64];
    district_type type;
    i32 wealth_level;    // 1-5
    i32 crime_rate;      // 1-5
};

struct location {
    i64 id;
    i64 district_id;
    char name[64];
    location_type type;
    bool is_public;
    i32 open_hour;       // 0-23, -1 = always open
    i32 close_hour;      // 0-23, -1 = always open
    security_level security;
    i32 capacity;
};

struct actor {
    i64 id;
    char name[64];
    i32 age;
    char sex;            // 'M' or 'F'
    char occupation[32];
    i32 income;          // Monthly income level 1-10

    i64 home_id;
    i64 work_id;

    // Personality traits (0-100)
    i32 impulsivity;
    i32 morality;
    i32 greed;
    i32 aggression;
    i32 loyalty;

    // Current need levels (0-100, lower = more desperate)
    i32 need_money;
    i32 need_belonging;
    i32 need_status;
    i32 need_security;
};

struct relationship {
    i64 id;
    i64 actor1_id;
    i64 actor2_id;
    relationship_type type;
    relationship_subtype subtype;
    i32 strength;        // 1-100
    i32 sentiment;       // -100 to +100
};

struct event {
    i64 id;
    i32 timestamp;       // Simulation hour (day * 24 + hour)
    event_type type;
    i64 actor_id;
    i64 target_id;       // Other actor, if any
    i64 location_id;
    char details[256];   // JSON or text description
};

struct grievance {
    i64 id;
    i64 victim_id;       // Who was wronged
    i64 offender_id;     // Who wronged them
    i64 cause_event_id;
    i32 severity;        // 1-100
    i32 timestamp;
    bool resolved;
};

struct crime {
    i64 id;
    crime_type type;
    i64 perpetrator_id;
    i64 victim_id;
    i64 location_id;
    i32 timestamp;
    motive_type motive;
    f32 interest_score;  // For selection
};

struct evidence {
    i64 id;
    i64 crime_id;
    evidence_type type;
    i64 location_id;     // Where it was found/can be found
    i64 points_to_id;    // Actor it implicates
    i32 clarity;         // 1-100, how clear/useful
    char description[256];
};

struct witness {
    i64 id;
    i64 crime_id;
    i64 actor_id;
    char saw_what[256];
    i32 reliability;     // 1-100
    i32 willingness;     // 1-100
};

struct causality_link {
    i64 event_id;
    i32 day;
    char summary[256];
};

//------------------------------------------------------------------------------
// Simulation Context
//------------------------------------------------------------------------------

struct sim_context {
    struct sqlite3* db;
    city_size size;
    i32 seed;
    i32 current_day;
    i32 current_hour;
    i32 total_days;

    // Counts for quick access
    i32 num_districts;
    i32 num_locations;
    i32 num_actors;
    i32 num_crimes;
};

//------------------------------------------------------------------------------
// Case Output Structure
//------------------------------------------------------------------------------

struct case_file {
    crime selected_crime;

    causality_link* chain;
    i32 chain_length;

    i64* suspect_ids;
    i32 num_suspects;

    evidence* evidence_list;
    i32 num_evidence;

    witness* witnesses;
    i32 num_witnesses;

    // Blanks for player
    struct blank {
        blank_type type;
        i32 chain_index;     // Which chain link this refers to
        char correct_answer[128];
        i64* evidence_ids;   // Evidence that reveals this
        i32 num_evidence;
    };
    blank* blanks;
    i32 num_blanks;
};

//------------------------------------------------------------------------------
// Utility Functions
//------------------------------------------------------------------------------

inline i32 timestamp_to_day(i32 ts) { return ts / SIM_HOURS_PER_DAY; }
inline i32 timestamp_to_hour(i32 ts) { return ts % SIM_HOURS_PER_DAY; }
inline i32 make_timestamp(i32 day, i32 hour) { return day * SIM_HOURS_PER_DAY + hour; }

inline const char* crime_type_str(crime_type t) {
    switch (t) {
        case crime_type::MURDER: return "murder";
        case crime_type::KIDNAPPING: return "kidnapping";
        case crime_type::THEFT: return "theft";
        default: return "none";
    }
}

inline const char* motive_type_str(motive_type t) {
    switch (t) {
        case motive_type::REVENGE: return "revenge";
        case motive_type::FINANCIAL_GAIN: return "financial_gain";
        case motive_type::JEALOUSY: return "jealousy";
        case motive_type::SILENCING: return "silencing";
        case motive_type::INHERITANCE: return "inheritance";
        case motive_type::POWER: return "power";
        case motive_type::RAGE: return "rage";
        default: return "unknown";
    }
}

inline const char* evidence_type_str(evidence_type t) {
    switch (t) {
        case evidence_type::WEAPON: return "weapon";
        case evidence_type::FINGERPRINT: return "fingerprint";
        case evidence_type::DNA: return "dna";
        case evidence_type::FIBER: return "fiber";
        case evidence_type::FOOTPRINT: return "footprint";
        case evidence_type::WITNESS_STATEMENT: return "witness_statement";
        case evidence_type::RECEIPT: return "receipt";
        case evidence_type::PHONE_RECORD: return "phone_record";
        case evidence_type::BANK_RECORD: return "bank_record";
        case evidence_type::EMAIL: return "email";
        case evidence_type::PHOTO: return "photo";
        case evidence_type::VIDEO: return "video";
        case evidence_type::SECURITY_FOOTAGE: return "security_footage";
        case evidence_type::ALIBI: return "alibi";
        default: return "unknown";
    }
}
