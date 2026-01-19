#include "sim_db.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

//------------------------------------------------------------------------------
// Name Data
//------------------------------------------------------------------------------

static const char* MALE_FIRST_NAMES[] = {
    "James", "John", "Robert", "Michael", "William", "David", "Richard", "Joseph",
    "Thomas", "Charles", "Christopher", "Daniel", "Matthew", "Anthony", "Mark",
    "Donald", "Steven", "Paul", "Andrew", "Joshua", "Kenneth", "Kevin", "Brian",
    "George", "Timothy", "Ronald", "Edward", "Jason", "Jeffrey", "Ryan",
    "Jacob", "Gary", "Nicholas", "Eric", "Jonathan", "Stephen", "Larry", "Justin",
    "Scott", "Brandon", "Benjamin", "Samuel", "Raymond", "Gregory", "Frank",
    "Alexander", "Patrick", "Jack", "Dennis", "Jerry"
};
static const i32 NUM_MALE_NAMES = sizeof(MALE_FIRST_NAMES) / sizeof(MALE_FIRST_NAMES[0]);

static const char* FEMALE_FIRST_NAMES[] = {
    "Mary", "Patricia", "Jennifer", "Linda", "Barbara", "Elizabeth", "Susan",
    "Jessica", "Sarah", "Karen", "Lisa", "Nancy", "Betty", "Margaret", "Sandra",
    "Ashley", "Kimberly", "Emily", "Donna", "Michelle", "Dorothy", "Carol",
    "Amanda", "Melissa", "Deborah", "Stephanie", "Rebecca", "Sharon", "Laura",
    "Cynthia", "Kathleen", "Amy", "Angela", "Shirley", "Anna", "Brenda", "Pamela",
    "Emma", "Nicole", "Helen", "Samantha", "Katherine", "Christine", "Debra",
    "Rachel", "Carolyn", "Janet", "Catherine", "Maria", "Heather"
};
static const i32 NUM_FEMALE_NAMES = sizeof(FEMALE_FIRST_NAMES) / sizeof(FEMALE_FIRST_NAMES[0]);

static const char* LAST_NAMES[] = {
    "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis",
    "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez", "Wilson", "Anderson",
    "Thomas", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Perez", "Thompson",
    "White", "Harris", "Sanchez", "Clark", "Ramirez", "Lewis", "Robinson",
    "Walker", "Young", "Allen", "King", "Wright", "Scott", "Torres", "Nguyen",
    "Hill", "Flores", "Green", "Adams", "Nelson", "Baker", "Hall", "Rivera",
    "Campbell", "Mitchell", "Carter", "Roberts"
};
static const i32 NUM_LAST_NAMES = sizeof(LAST_NAMES) / sizeof(LAST_NAMES[0]);

//------------------------------------------------------------------------------
// Occupation Data
//------------------------------------------------------------------------------

struct occupation_info {
    const char* name;
    i32 income_min;
    i32 income_max;
    location_type workplace_type;
};

static const occupation_info OCCUPATIONS[] = {
    { "Accountant",      6, 8,  location_type::OFFICE },
    { "Lawyer",          7, 10, location_type::OFFICE },
    { "Doctor",          8, 10, location_type::HOSPITAL },
    { "Nurse",           5, 7,  location_type::HOSPITAL },
    { "Teacher",         4, 6,  location_type::OFFICE },
    { "Engineer",        6, 9,  location_type::OFFICE },
    { "Programmer",      6, 9,  location_type::OFFICE },
    { "Sales Rep",       4, 7,  location_type::STORE },
    { "Manager",         6, 9,  location_type::OFFICE },
    { "Executive",       8, 10, location_type::OFFICE },
    { "Retail Worker",   2, 4,  location_type::STORE },
    { "Bartender",       3, 5,  location_type::BAR },
    { "Waiter",          2, 4,  location_type::RESTAURANT },
    { "Chef",            4, 7,  location_type::RESTAURANT },
    { "Security Guard",  3, 5,  location_type::WAREHOUSE },
    { "Factory Worker",  3, 5,  location_type::FACTORY },
    { "Warehouse Worker",2, 4,  location_type::WAREHOUSE },
    { "Bank Teller",     4, 6,  location_type::BANK },
    { "Banker",          7, 10, location_type::BANK },
    { "Police Officer",  5, 7,  location_type::POLICE_STATION },
    { "Contractor",      5, 8,  location_type::WAREHOUSE },
    { "Electrician",     5, 7,  location_type::FACTORY },
    { "Mechanic",        4, 6,  location_type::FACTORY },
    { "Hotel Staff",     3, 5,  location_type::HOTEL },
    { "Unemployed",      1, 2,  location_type::HOUSE },
};
static const i32 NUM_OCCUPATIONS = sizeof(OCCUPATIONS) / sizeof(OCCUPATIONS[0]);

//------------------------------------------------------------------------------
// Random helpers
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

// Normal distribution using Box-Muller
static f32 rnd_normal(f32 mean, f32 stddev) {
    static bool has_spare = false;
    static f32 spare;

    if (has_spare) {
        has_spare = false;
        return mean + stddev * spare;
    }

    f32 u, v, s;
    do {
        u = rnd_float() * 2.0f - 1.0f;
        v = rnd_float() * 2.0f - 1.0f;
        s = u * u + v * v;
    } while (s >= 1.0f || s == 0.0f);

    s = sqrtf(-2.0f * logf(s) / s);
    spare = v * s;
    has_spare = true;

    return mean + stddev * u * s;
}

static i32 clamp(i32 val, i32 min, i32 max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

//------------------------------------------------------------------------------
// Actor Generation
//------------------------------------------------------------------------------

static void generate_actor_name(char* out, size_t max_len, char sex) {
    const char* first;
    if (sex == 'M') {
        first = MALE_FIRST_NAMES[rnd_int(NUM_MALE_NAMES)];
    } else {
        first = FEMALE_FIRST_NAMES[rnd_int(NUM_FEMALE_NAMES)];
    }
    const char* last = LAST_NAMES[rnd_int(NUM_LAST_NAMES)];
    snprintf(out, max_len, "%s %s", first, last);
}

static i32 generate_age() {
    // Normal distribution centered at 35, most people 20-60
    f32 age_f = rnd_normal(35.0f, 12.0f);
    return clamp((i32)age_f, 18, 75);
}

static void generate_personality(actor* a) {
    // Generate with some correlation to create "types"
    f32 base = rnd_float();

    if (base < 0.2f) {
        // Volatile type - high impulsivity, high aggression
        a->impulsivity = rnd_range(60, 90);
        a->aggression = rnd_range(50, 80);
        a->morality = rnd_range(20, 50);
        a->greed = rnd_range(40, 70);
        a->loyalty = rnd_range(30, 60);
    } else if (base < 0.4f) {
        // Greedy type - high greed, lower morality
        a->impulsivity = rnd_range(30, 60);
        a->aggression = rnd_range(20, 50);
        a->morality = rnd_range(30, 60);
        a->greed = rnd_range(70, 95);
        a->loyalty = rnd_range(20, 50);
    } else if (base < 0.6f) {
        // Loyal type - high loyalty, high morality
        a->impulsivity = rnd_range(20, 50);
        a->aggression = rnd_range(20, 40);
        a->morality = rnd_range(60, 90);
        a->greed = rnd_range(20, 50);
        a->loyalty = rnd_range(70, 95);
    } else {
        // Average type - balanced stats
        a->impulsivity = rnd_range(30, 70);
        a->aggression = rnd_range(30, 60);
        a->morality = rnd_range(40, 70);
        a->greed = rnd_range(30, 60);
        a->loyalty = rnd_range(40, 70);
    }
}

static void generate_initial_needs(actor* a) {
    // Start with mostly satisfied needs, some variation
    a->need_money = rnd_range(40, 80);
    a->need_belonging = rnd_range(50, 90);
    a->need_status = rnd_range(40, 70);
    a->need_security = rnd_range(60, 90);
}

static i64 find_workplace(sim_context* ctx, location_type type) {
    location locs[32];
    i32 count = db_get_locations_by_type(ctx, type, locs, 32);

    if (count == 0) {
        // Fallback to any office
        count = db_get_locations_by_type(ctx, location_type::OFFICE, locs, 32);
    }

    if (count > 0) {
        return locs[rnd_int(count)].id;
    }

    return 0;
}

static i64 find_home(sim_context* ctx) {
    // Find a residential location
    location locs[64];
    i32 count = 0;

    i32 house_count = db_get_locations_by_type(ctx, location_type::HOUSE, locs, 32);
    count += house_count;

    i32 apt_count = db_get_locations_by_type(ctx, location_type::APARTMENT, locs + count, 32);
    count += apt_count;

    if (count > 0) {
        return locs[rnd_int(count)].id;
    }

    return 0;
}

//------------------------------------------------------------------------------
// Relationship Generation
//------------------------------------------------------------------------------

static void generate_family_relationships(sim_context* ctx, i64* actor_ids, i32 num_actors) {
    // Create some family units (spouse pairs)
    i32 num_families = num_actors / 8;  // ~12% of actors are married

    for (i32 i = 0; i < num_families && i * 2 + 1 < num_actors; i++) {
        i64 a1 = actor_ids[rnd_int(num_actors)];
        i64 a2 = actor_ids[rnd_int(num_actors)];

        if (a1 != a2) {
            // Check they're not already related
            relationship rel;
            if (!db_get_relationship_between(ctx, a1, a2, &rel)) {
                db_insert_relationship(ctx, a1, a2,
                    relationship_type::FAMILY, relationship_subtype::SPOUSE,
                    rnd_range(60, 95), rnd_range(40, 90));
            }
        }
    }

    // Create some sibling relationships
    i32 num_siblings = num_actors / 10;
    for (i32 i = 0; i < num_siblings; i++) {
        i64 a1 = actor_ids[rnd_int(num_actors)];
        i64 a2 = actor_ids[rnd_int(num_actors)];

        if (a1 != a2) {
            relationship rel;
            if (!db_get_relationship_between(ctx, a1, a2, &rel)) {
                db_insert_relationship(ctx, a1, a2,
                    relationship_type::FAMILY, relationship_subtype::SIBLING,
                    rnd_range(50, 90), rnd_range(20, 80));
            }
        }
    }
}

static void generate_work_relationships(sim_context* ctx, i64* actor_ids, i32 num_actors) {
    // Group actors by workplace and create coworker relationships
    actor actors[256];
    i32 count = db_get_all_actors(ctx, actors, 256);

    for (i32 i = 0; i < count; i++) {
        for (i32 j = i + 1; j < count; j++) {
            if (actors[i].work_id > 0 && actors[i].work_id == actors[j].work_id) {
                // Same workplace - coworkers
                relationship rel;
                if (!db_get_relationship_between(ctx, actors[i].id, actors[j].id, &rel)) {
                    // Decide relationship type
                    f32 roll = rnd_float();
                    relationship_subtype subtype;

                    if (roll < 0.1f && actors[i].income > actors[j].income) {
                        subtype = relationship_subtype::BOSS;
                    } else if (roll < 0.2f && actors[j].income > actors[i].income) {
                        subtype = relationship_subtype::EMPLOYEE;
                    } else {
                        subtype = relationship_subtype::PEER;
                    }

                    i32 sentiment = rnd_range(-20, 60);  // Coworkers can have friction
                    db_insert_relationship(ctx, actors[i].id, actors[j].id,
                        relationship_type::COWORKER, subtype,
                        rnd_range(30, 70), sentiment);
                }
            }
        }
    }
}

static void generate_social_relationships(sim_context* ctx, i64* actor_ids, i32 num_actors) {
    // Random friendships
    i32 num_friendships = num_actors / 3;

    for (i32 i = 0; i < num_friendships; i++) {
        i64 a1 = actor_ids[rnd_int(num_actors)];
        i64 a2 = actor_ids[rnd_int(num_actors)];

        if (a1 != a2) {
            relationship rel;
            if (!db_get_relationship_between(ctx, a1, a2, &rel)) {
                f32 roll = rnd_float();
                relationship_subtype subtype;
                i32 strength, sentiment;

                if (roll < 0.3f) {
                    subtype = relationship_subtype::CLOSE_FRIEND;
                    strength = rnd_range(70, 95);
                    sentiment = rnd_range(60, 95);
                } else if (roll < 0.7f) {
                    subtype = relationship_subtype::CASUAL_FRIEND;
                    strength = rnd_range(40, 70);
                    sentiment = rnd_range(30, 70);
                } else {
                    subtype = relationship_subtype::NEIGHBOR;
                    strength = rnd_range(20, 50);
                    sentiment = rnd_range(10, 50);
                }

                db_insert_relationship(ctx, a1, a2,
                    relationship_type::FRIEND, subtype, strength, sentiment);
            }
        }
    }

    // Some rivalries
    i32 num_rivalries = num_actors / 10;
    for (i32 i = 0; i < num_rivalries; i++) {
        i64 a1 = actor_ids[rnd_int(num_actors)];
        i64 a2 = actor_ids[rnd_int(num_actors)];

        if (a1 != a2) {
            relationship rel;
            if (!db_get_relationship_between(ctx, a1, a2, &rel)) {
                db_insert_relationship(ctx, a1, a2,
                    relationship_type::RIVAL, relationship_subtype::COMPETITOR,
                    rnd_range(40, 80), rnd_range(-70, -20));
            }
        }
    }
}

static void generate_romantic_relationships(sim_context* ctx, i64* actor_ids, i32 num_actors) {
    // Some romantic partners (non-spouse)
    i32 num_partners = num_actors / 12;

    for (i32 i = 0; i < num_partners; i++) {
        i64 a1 = actor_ids[rnd_int(num_actors)];
        i64 a2 = actor_ids[rnd_int(num_actors)];

        if (a1 != a2) {
            relationship rel;
            if (!db_get_relationship_between(ctx, a1, a2, &rel)) {
                f32 roll = rnd_float();
                relationship_subtype subtype;
                i32 sentiment;

                if (roll < 0.5f) {
                    subtype = relationship_subtype::PARTNER;
                    sentiment = rnd_range(50, 90);
                } else if (roll < 0.8f) {
                    subtype = relationship_subtype::EX_PARTNER;
                    sentiment = rnd_range(-50, 30);  // Ex can be bitter
                } else {
                    subtype = relationship_subtype::AFFAIR;
                    sentiment = rnd_range(30, 70);
                }

                db_insert_relationship(ctx, a1, a2,
                    relationship_type::ROMANTIC, subtype,
                    rnd_range(50, 90), sentiment);
            }
        }
    }
}

//------------------------------------------------------------------------------
// Main Population Phase
//------------------------------------------------------------------------------

bool phase_population(sim_context* ctx) {
    printf("[PHASE 2] Population - Generating actors...\n");

    // Determine number of actors based on city size
    i32 target_actors;
    switch (ctx->size) {
        case city_size::SMALL:  target_actors = CITY_SMALL_ACTORS; break;
        case city_size::MEDIUM: target_actors = CITY_MEDIUM_ACTORS; break;
        case city_size::LARGE:  target_actors = CITY_LARGE_ACTORS; break;
        case city_size::METRO:  target_actors = CITY_METRO_ACTORS; break;
        default: target_actors = CITY_MEDIUM_ACTORS;
    }

    printf("  Creating %d actors...\n", target_actors);

    i64* actor_ids = new i64[target_actors];
    i32 created = 0;

    for (i32 i = 0; i < target_actors; i++) {
        actor a = {};

        // Demographics
        a.sex = (rnd_float() > 0.5f) ? 'M' : 'F';
        generate_actor_name(a.name, sizeof(a.name), a.sex);
        a.age = generate_age();

        // Occupation and income
        i32 occ_idx = rnd_int(NUM_OCCUPATIONS);
        const occupation_info& occ = OCCUPATIONS[occ_idx];
        strncpy(a.occupation, occ.name, sizeof(a.occupation) - 1);
        a.income = rnd_range(occ.income_min, occ.income_max);

        // Find home and workplace
        a.home_id = find_home(ctx);
        if (occ.workplace_type != location_type::HOUSE) {
            a.work_id = find_workplace(ctx, occ.workplace_type);
        } else {
            a.work_id = a.home_id;  // Unemployed stays home
        }

        // Personality
        generate_personality(&a);

        // Initial needs
        generate_initial_needs(&a);

        // Insert into database
        i64 id = db_insert_actor(ctx, &a);
        if (id > 0) {
            actor_ids[created++] = id;
        }
    }

    printf("  Created %d actors\n", created);

    // Generate relationships
    printf("  Generating relationships...\n");

    generate_family_relationships(ctx, actor_ids, created);
    generate_work_relationships(ctx, actor_ids, created);
    generate_social_relationships(ctx, actor_ids, created);
    generate_romantic_relationships(ctx, actor_ids, created);

    // Count relationships
    i32 total_relationships = 0;
    for (i32 i = 0; i < created; i++) {
        relationship rels[32];
        total_relationships += db_get_relationships_for_actor(ctx, actor_ids[i], rels, 32);
    }
    // Divide by 2 since each relationship is counted twice
    total_relationships /= 2;

    printf("  Created %d relationships\n", total_relationships);

    delete[] actor_ids;
    return true;
}
