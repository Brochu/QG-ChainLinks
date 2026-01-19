#include "sim_db.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

//------------------------------------------------------------------------------
// Name Generation (simple for districts/locations)
//------------------------------------------------------------------------------

static const char* DISTRICT_PREFIXES[] = {
    "North", "South", "East", "West", "Upper", "Lower", "Old", "New", "Central", "Downtown"
};
static const i32 NUM_PREFIXES = sizeof(DISTRICT_PREFIXES) / sizeof(DISTRICT_PREFIXES[0]);

static const char* DISTRICT_SUFFIXES[] = {
    "Heights", "Park", "Square", "District", "Quarter", "Village", "Point", "Harbor", "Hill", "Gardens",
    "Row", "Gate", "Side", "End", "Ward", "Field", "Bridge", "Town", "Vale", "Crossing"
};
static const i32 NUM_SUFFIXES = sizeof(DISTRICT_SUFFIXES) / sizeof(DISTRICT_SUFFIXES[0]);

static const char* STREET_NAMES[] = {
    "Main", "Oak", "Maple", "Pine", "Cedar", "Elm", "Birch", "Walnut", "Cherry", "Willow",
    "Lake", "River", "Park", "Hill", "Valley", "Spring", "Mill", "Church", "School", "Market"
};
static const i32 NUM_STREETS = sizeof(STREET_NAMES) / sizeof(STREET_NAMES[0]);

static const char* BUSINESS_ADJECTIVES[] = {
    "Golden", "Silver", "Royal", "Grand", "Premier", "Elite", "Prime", "Central", "Metro", "City"
};
static const i32 NUM_BUSINESS_ADJ = sizeof(BUSINESS_ADJECTIVES) / sizeof(BUSINESS_ADJECTIVES[0]);

//------------------------------------------------------------------------------
// Random helpers (using stdlib for now, can swap to engine's rand)
//------------------------------------------------------------------------------

static i32 rnd_int(i32 max) {
    return rand() % max;
}

static i32 rnd_range(i32 min, i32 max) {
    return min + rand() % (max - min + 1);
}

static f32 rnd_float() {
    return (f32)rand() / (f32)RAND_MAX;
}

//------------------------------------------------------------------------------
// District Generation
//------------------------------------------------------------------------------

struct district_template {
    district_type type;
    i32 wealth_min, wealth_max;
    i32 crime_min, crime_max;
    i32 weight;  // Probability weight
};

static const district_template DISTRICT_TEMPLATES[] = {
    { district_type::RESIDENTIAL, 2, 4, 1, 3, 30 },
    { district_type::COMMERCIAL,  3, 5, 1, 2, 20 },
    { district_type::INDUSTRIAL,  1, 3, 2, 4, 15 },
    { district_type::FINANCIAL,   4, 5, 1, 2, 10 },
    { district_type::NIGHTLIFE,   2, 4, 3, 5, 15 },
    { district_type::MIXED,       2, 4, 2, 3, 10 }
};
static const i32 NUM_DISTRICT_TEMPLATES = sizeof(DISTRICT_TEMPLATES) / sizeof(DISTRICT_TEMPLATES[0]);

static i32 select_weighted_template() {
    i32 total = 0;
    for (i32 i = 0; i < NUM_DISTRICT_TEMPLATES; i++) {
        total += DISTRICT_TEMPLATES[i].weight;
    }

    i32 roll = rnd_int(total);
    i32 cumulative = 0;
    for (i32 i = 0; i < NUM_DISTRICT_TEMPLATES; i++) {
        cumulative += DISTRICT_TEMPLATES[i].weight;
        if (roll < cumulative) return i;
    }
    return 0;
}

static void generate_district_name(char* out, size_t max_len) {
    const char* prefix = DISTRICT_PREFIXES[rnd_int(NUM_PREFIXES)];
    const char* suffix = DISTRICT_SUFFIXES[rnd_int(NUM_SUFFIXES)];
    snprintf(out, max_len, "%s %s", prefix, suffix);
}

//------------------------------------------------------------------------------
// Location Generation
//------------------------------------------------------------------------------

struct location_template {
    location_type type;
    const char* name_format;
    bool is_public;
    i32 open_hour;
    i32 close_hour;
    security_level security;
    i32 capacity_min, capacity_max;
};

static const location_template RESIDENTIAL_LOCATIONS[] = {
    { location_type::HOUSE,     "%s House",           false, -1, -1, security_level::NONE,    2, 6 },
    { location_type::APARTMENT, "%s Apartments",      false, -1, -1, security_level::CAMERAS, 10, 50 },
};

static const location_template COMMERCIAL_LOCATIONS[] = {
    { location_type::OFFICE,     "%s Office Building", false, 8, 18, security_level::CAMERAS, 20, 100 },
    { location_type::STORE,      "%s Store",           true,  9, 21, security_level::CAMERAS, 5, 20 },
    { location_type::RESTAURANT, "%s Restaurant",      true, 11, 23, security_level::NONE,    10, 50 },
    { location_type::BANK,       "%s Bank",            true,  9, 17, security_level::VAULT,   10, 30 },
    { location_type::HOTEL,      "%s Hotel",           true, -1, -1, security_level::CAMERAS, 50, 200 },
};

static const location_template INDUSTRIAL_LOCATIONS[] = {
    { location_type::WAREHOUSE, "%s Warehouse",        false, 6, 22, security_level::CAMERAS, 5, 20 },
    { location_type::FACTORY,   "%s Factory",          false, 6, 22, security_level::GUARDS,  50, 200 },
};

static const location_template FINANCIAL_LOCATIONS[] = {
    { location_type::OFFICE,  "%s Tower",             false, 7, 20, security_level::GUARDS,  100, 500 },
    { location_type::BANK,    "%s Financial",         true,  9, 17, security_level::VAULT,   20, 50 },
};

static const location_template NIGHTLIFE_LOCATIONS[] = {
    { location_type::BAR,       "%s Bar",             true, 17, 2, security_level::NONE,    20, 50 },
    { location_type::NIGHTCLUB, "%s Club",            true, 21, 4, security_level::GUARDS,  100, 300 },
    { location_type::CASINO,    "%s Casino",          true, -1, -1, security_level::VAULT,   200, 500 },
    { location_type::RESTAURANT,"%s Diner",           true, 18, 3, security_level::NONE,    15, 40 },
};

static const location_template PUBLIC_LOCATIONS[] = {
    { location_type::PARK,           "%s Park",       true, -1, -1, security_level::NONE,    50, 200 },
    { location_type::CHURCH,         "%s Church",     true, 6, 22, security_level::NONE,    30, 100 },
    { location_type::HOSPITAL,       "%s Hospital",   true, -1, -1, security_level::CAMERAS, 100, 500 },
    { location_type::POLICE_STATION, "%s Precinct",   true, -1, -1, security_level::GUARDS,  30, 100 },
};

static void generate_location_name(char* out, size_t max_len, const char* format) {
    const char* street = STREET_NAMES[rnd_int(NUM_STREETS)];
    snprintf(out, max_len, format, street);
}

static void generate_business_name(char* out, size_t max_len, const char* format) {
    const char* adj = BUSINESS_ADJECTIVES[rnd_int(NUM_BUSINESS_ADJ)];
    snprintf(out, max_len, format, adj);
}

static i32 generate_locations_for_district(sim_context* ctx, i64 district_id,
                                           district_type type, i32 wealth) {
    i32 count = 0;

    // Every district gets some residential
    i32 num_residential = rnd_range(3, 6);
    for (i32 i = 0; i < num_residential; i++) {
        const location_template& tpl = RESIDENTIAL_LOCATIONS[rnd_int(2)];
        char name[64];
        generate_location_name(name, sizeof(name), tpl.name_format);

        i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
        db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                          tpl.open_hour, tpl.close_hour, tpl.security, cap);
        count++;
    }

    // District-specific locations
    switch (type) {
        case district_type::RESIDENTIAL: {
            // Add a few public spaces
            for (i32 i = 0; i < 2; i++) {
                const location_template& tpl = PUBLIC_LOCATIONS[rnd_int(2)]; // Park or Church
                char name[64];
                generate_location_name(name, sizeof(name), tpl.name_format);
                i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
                db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                                  tpl.open_hour, tpl.close_hour, tpl.security, cap);
                count++;
            }
            break;
        }

        case district_type::COMMERCIAL: {
            i32 num_commercial = rnd_range(4, 8);
            for (i32 i = 0; i < num_commercial; i++) {
                const location_template& tpl = COMMERCIAL_LOCATIONS[rnd_int(5)];
                char name[64];
                generate_business_name(name, sizeof(name), tpl.name_format);
                i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
                db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                                  tpl.open_hour, tpl.close_hour, tpl.security, cap);
                count++;
            }
            break;
        }

        case district_type::INDUSTRIAL: {
            i32 num_industrial = rnd_range(3, 6);
            for (i32 i = 0; i < num_industrial; i++) {
                const location_template& tpl = INDUSTRIAL_LOCATIONS[rnd_int(2)];
                char name[64];
                generate_business_name(name, sizeof(name), tpl.name_format);
                i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
                db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                                  tpl.open_hour, tpl.close_hour, tpl.security, cap);
                count++;
            }
            break;
        }

        case district_type::FINANCIAL: {
            i32 num_financial = rnd_range(3, 5);
            for (i32 i = 0; i < num_financial; i++) {
                const location_template& tpl = FINANCIAL_LOCATIONS[rnd_int(2)];
                char name[64];
                generate_business_name(name, sizeof(name), tpl.name_format);
                i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
                db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                                  tpl.open_hour, tpl.close_hour, tpl.security, cap);
                count++;
            }
            break;
        }

        case district_type::NIGHTLIFE: {
            i32 num_nightlife = rnd_range(4, 7);
            for (i32 i = 0; i < num_nightlife; i++) {
                const location_template& tpl = NIGHTLIFE_LOCATIONS[rnd_int(4)];
                char name[64];
                generate_business_name(name, sizeof(name), tpl.name_format);
                i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
                db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                                  tpl.open_hour, tpl.close_hour, tpl.security, cap);
                count++;
            }
            break;
        }

        case district_type::MIXED: {
            // Mix of commercial and nightlife
            i32 num_mixed = rnd_range(3, 5);
            for (i32 i = 0; i < num_mixed; i++) {
                if (rnd_float() > 0.5f) {
                    const location_template& tpl = COMMERCIAL_LOCATIONS[rnd_int(5)];
                    char name[64];
                    generate_business_name(name, sizeof(name), tpl.name_format);
                    i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
                    db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                                      tpl.open_hour, tpl.close_hour, tpl.security, cap);
                } else {
                    const location_template& tpl = NIGHTLIFE_LOCATIONS[rnd_int(4)];
                    char name[64];
                    generate_business_name(name, sizeof(name), tpl.name_format);
                    i32 cap = rnd_range(tpl.capacity_min, tpl.capacity_max);
                    db_insert_location(ctx, district_id, name, tpl.type, tpl.is_public,
                                      tpl.open_hour, tpl.close_hour, tpl.security, cap);
                }
                count++;
            }
            break;
        }
    }

    return count;
}

//------------------------------------------------------------------------------
// Main Foundation Phase
//------------------------------------------------------------------------------

bool phase_foundation(sim_context* ctx) {
    printf("[PHASE 1] Foundation - Generating world...\n");

    // Determine number of districts based on city size
    i32 num_districts;
    switch (ctx->size) {
        case city_size::SMALL:  num_districts = CITY_SMALL_DISTRICTS; break;
        case city_size::MEDIUM: num_districts = CITY_MEDIUM_DISTRICTS; break;
        case city_size::LARGE:  num_districts = CITY_LARGE_DISTRICTS; break;
        case city_size::METRO:  num_districts = CITY_METRO_DISTRICTS; break;
        default: num_districts = CITY_MEDIUM_DISTRICTS;
    }

    printf("  Creating %d districts...\n", num_districts);

    // Track which district types we've used to ensure variety
    bool used_types[(i32)district_type::MIXED + 1] = {false};

    for (i32 i = 0; i < num_districts; i++) {
        // Select template, prefer unused types for first few
        i32 template_idx;
        if (i < 3) {
            // Force variety for first 3 districts
            do {
                template_idx = select_weighted_template();
            } while (used_types[(i32)DISTRICT_TEMPLATES[template_idx].type] && i > 0);
        } else {
            template_idx = select_weighted_template();
        }

        const district_template& tpl = DISTRICT_TEMPLATES[template_idx];
        used_types[(i32)tpl.type] = true;

        char name[64];
        generate_district_name(name, sizeof(name));

        i32 wealth = rnd_range(tpl.wealth_min, tpl.wealth_max);
        i32 crime = rnd_range(tpl.crime_min, tpl.crime_max);

        i64 district_id = db_insert_district(ctx, name, tpl.type, wealth, crime);

        if (district_id > 0) {
            i32 loc_count = generate_locations_for_district(ctx, district_id, tpl.type, wealth);
            printf("    [%lld] %s (%d locations)\n", district_id, name, loc_count);
        }
    }

    // Add one hospital and one police station to a random district
    district districts[16];
    i32 num_d = db_get_all_districts(ctx, districts, 16);
    if (num_d > 0) {
        i64 hospital_district = districts[rnd_int(num_d)].id;
        i64 police_district = districts[rnd_int(num_d)].id;

        db_insert_location(ctx, hospital_district, "City General Hospital",
                          location_type::HOSPITAL, true, -1, -1, security_level::CAMERAS, 300);
        db_insert_location(ctx, police_district, "Central Police Station",
                          location_type::POLICE_STATION, true, -1, -1, security_level::GUARDS, 100);
    }

    printf("  Total: %d districts, %d locations\n", ctx->num_districts, ctx->num_locations);
    return true;
}
