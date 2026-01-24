#include "world_setup.h"
#include "sim_db.h"
#include "qg_memory.hpp"
#include "qg_random.hpp"

#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

// ============================================================================
// CONSTANTS
// ============================================================================

#define WORLD_SCRATCH_SIZE (1024 * 1024 * 4)  // 4MB scratch arena
#define ASSETS_DIR "../../assets/"

static const i32 size_to_actors[CITY_SIZE_COUNT] = { 50, 80, 120, 150 };
static const i32 size_to_districts[CITY_SIZE_COUNT] = { 2, 4, 6, 9 };
static const i32 size_to_locations[CITY_SIZE_COUNT] = { 20, 35, 50, 70 };

static const i32 district_weights[DISTRICT_TYPE_COUNT] = { 3, 2, 1, 2, 1, 1 };

static const i32 archetype_weights[ARCHETYPE_COUNT] = { 10, 15, 15, 60 };  // volatile, greedy, loyal, average
static const char *location_type_prefixes[] = {
    "Residence",    // LOC_RESIDENCE
    "Business",     // LOC_BUSINESS
    "Public",       // LOC_PUBLIC
    "Outdoor",      // LOC_OUTDOOR
    "Vehicle"       // LOC_VEHICLE
};

static const char *occupations[] = {
    // Civilian baseline
    "Accountant", "Bartender", "Chef", "Doctor", "Engineer",
    "Factory Worker", "Graphic Designer", "Hotel Manager", "IT Specialist", "Journalist",
    "Lawyer", "Mechanic", "Nurse", "Office Worker", "Pharmacist",
    "Real Estate Agent", "Security Guard", "Teacher", "Waiter",
    // FBI-relevant: jurisdictional tension, financial crimes, institutional power
    "Police Officer", "Investment Banker", "Truck Driver",
    "Small Business Owner", "Politician", "Private Investigator"
};
static const i32 num_occupations = sizeof(occupations) / sizeof(occupations[0]);

static const char *nickname_suffixes[] = { "y", "ie", "o", "s" };
static const i32 num_nickname_suffixes = sizeof(nickname_suffixes) / sizeof(nickname_suffixes[0]);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static const char *alloc_string(mem_arena *arena, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    char *str = (char *)mem_arena_alloc(arena, len + 1, 1).p;
    memcpy(str, buf, len + 1);
    return str;
}

static const char *generate_phone_number(mem_arena *arena) {
    return alloc_string(arena, "555-%03d-%04d", rand_int(1000), rand_int(10000));
}

static const char *generate_address(world_ctx *ctx) {
    const char *street = ctx->street_names.names[rand_int(ctx->street_names.num_names)];
    return alloc_string(&ctx->_scratch, "%d %s", rand_int_min(100, 9999), street);
}

static const char *generate_nickname(mem_arena *arena, const char *name) {
    i32 name_len = (i32)strlen(name);
    if (name_len < 4) return nullptr;

    i32 base_len = std::min(name_len - 2, 4 + rand_int(2));  // 4-5 chars
    const char *suffix = nickname_suffixes[rand_int(num_nickname_suffixes)];

    char buf[32];
    snprintf(buf, sizeof(buf), "%.*s%s", base_len, name, suffix);
    buf[0] = (char)toupper(buf[0]);

    return alloc_string(arena, "%s", buf);
}

// ============================================================================
// GENERATION FUNCTIONS
// ============================================================================

// Street counts by city size
static const i32 size_to_streets[CITY_SIZE_COUNT] = { 8, 15, 25, 40 };

static void generate_streets(world_ctx *ctx) {
    //TODO: Do we need a separate function for this?
    const i32 num_streets = size_to_streets[ctx->size];
    name_gen_street(&ctx->street_names, num_streets);

    printf("[WORLD] Generated %d streets\n", num_streets);
}

static void generate_districts(world_ctx *ctx) {
    ctx->num_districts = size_to_districts[ctx->size];
    ctx->districts = (district *)mem_arena_alloc(&ctx->_scratch,
        sizeof(district) * ctx->num_districts, sizeof(void *)).p;

    name_gen_district(&ctx->district_names, ctx->num_districts);

    for (i32 i = 0; i < ctx->num_districts; i++) {
        district *d = &ctx->districts[i];
        d->id = i;

        d->name = ctx->district_names.names[i];
        d->type = (district_type)rand_weighted_index(rand_float01(), district_weights, DISTRICT_TYPE_COUNT);
        d->wealth = rand_int_min(1, 6);
        d->roughness = rand_int_min(1, 6);
        d->response_time = 5 + (6 - d->wealth) + rand_int(5);

        db_insert_district(ctx->db, d);
    }

    printf("[WORLD] Generated %d districts\n", ctx->num_districts);
}

static location_type district_to_location_type(district_type dt, bool force_residential) {
    if (force_residential) return LOC_RESIDENCE;

    switch (dt) {
        case DISTRICT_RESIDENTIAL:
            return (rand_int(3) == 0) ? LOC_PUBLIC : LOC_RESIDENCE;
        case DISTRICT_COMMERCIAL:
        case DISTRICT_FINANCIAL:
            return LOC_BUSINESS;
        case DISTRICT_INDUSTRIAL:
        case DISTRICT_DOCKS:
            return (rand_int(2) == 0) ? LOC_BUSINESS : LOC_OUTDOOR;
        case DISTRICT_NIGHTLIFE:
            return LOC_BUSINESS;
        default:
            return LOC_BUSINESS;
    }
}

static void generate_locations(world_ctx *ctx) {
    ctx->num_locations = size_to_locations[ctx->size];
    ctx->locations = (location *)mem_arena_alloc(&ctx->_scratch,
        sizeof(location) * ctx->num_locations, sizeof(void *)).p;

    // Ensure at least 1/3 of locations are residences
    i32 min_residences = ctx->num_locations / 3;
    i32 residence_count = 0;

    for (i32 i = 0; i < ctx->num_locations; i++) {
        location *loc = &ctx->locations[i];
        loc->id = i;
        loc->district_id = rand_int(ctx->num_districts);

        district *dist = &ctx->districts[loc->district_id];
        bool force_res = (residence_count < min_residences && i >= ctx->num_locations - min_residences + residence_count);
        loc->type = district_to_location_type(dist->type, force_res);

        if (loc->type == LOC_RESIDENCE) residence_count++;

        loc->name = alloc_string(&ctx->_scratch, "%s #%d", location_type_prefixes[loc->type], i);
        loc->address = generate_address(ctx);
        loc->owner_id = -1;

        if (loc->type == LOC_RESIDENCE) {
            loc->access = (dist->wealth > 3) ? ACCESS_KEYPAD : ACCESS_STANDARD_LOCK;
        } else if (loc->type == LOC_BUSINESS) {
            loc->access = (dist->wealth > 4) ? ACCESS_SECURITY_STAFF : ACCESS_STANDARD_LOCK;
        } else {
            loc->access = ACCESS_NONE;
        }

        if (loc->type == LOC_RESIDENCE) {
            loc->hours = HOURS_24_7;
        } else if (dist->type == DISTRICT_NIGHTLIFE) {
            loc->hours = HOURS_EVENING;
        } else {
            loc->hours = HOURS_BUSINESS;
        }

        loc->crime_factor = std::max(0, (dist->roughness * 5) + rand_int_min(-10, 10));

        db_insert_location(ctx->db, loc);
    }

    printf("[WORLD] Generated %d locations (%d residences)\n", ctx->num_locations, residence_count);
}

static void apply_archetype(person *p, personality_archetype arch) {
    p->archetype = arch;

    switch (arch) {
        case ARCHETYPE_VOLATILE:
            p->impulsivity = rand_int_min(70, 100);
            p->morality = rand_int_min(10, 40);
            p->greed = rand_int_min(30, 70);
            p->aggression = rand_int_min(70, 100);
            p->loyalty = rand_int_min(20, 50);
            break;

        case ARCHETYPE_GREEDY:
            p->impulsivity = rand_int_min(40, 70);
            p->morality = rand_int_min(20, 50);
            p->greed = rand_int_min(80, 100);
            p->aggression = rand_int_min(30, 60);
            p->loyalty = rand_int_min(20, 40);
            break;

        case ARCHETYPE_LOYAL:
            p->impulsivity = rand_int_min(20, 50);
            p->morality = rand_int_min(60, 90);
            p->greed = rand_int_min(10, 40);
            p->aggression = rand_int_min(20, 50);
            p->loyalty = rand_int_min(80, 100);
            break;

        case ARCHETYPE_AVERAGE:
        default:
            p->impulsivity = rand_int_min(30, 70);
            p->morality = rand_int_min(40, 70);
            p->greed = rand_int_min(30, 60);
            p->aggression = rand_int_min(30, 60);
            p->loyalty = rand_int_min(40, 70);
            break;
    }
}

static i64 find_residence(world_ctx *ctx) {
    for (i32 attempts = 0; attempts < 100; attempts++) {
        i32 idx = rand_int(ctx->num_locations);
        if (ctx->locations[idx].type == LOC_RESIDENCE) {
            return idx;
        }
    }
    for (i32 i = 0; i < ctx->num_locations; i++) {
        if (ctx->locations[i].type == LOC_RESIDENCE) return i;
    }
    return 0;
}

static i64 find_workplace(world_ctx *ctx) {
    for (i32 attempts = 0; attempts < 100; attempts++) {
        i32 idx = rand_int(ctx->num_locations);
        if (ctx->locations[idx].type != LOC_RESIDENCE) {
            return idx;
        }
    }
    return -1;
}

static void generate_people(world_ctx *ctx) {
    ctx->num_people = size_to_actors[ctx->size];
    ctx->people = (person *)mem_arena_alloc(&ctx->_scratch,
        sizeof(person) * ctx->num_people, sizeof(void *)).p;

    i32 archetype_counts[ARCHETYPE_COUNT] = {0};

    for (i32 i = 0; i < ctx->num_people; i++) {
        person *p = &ctx->people[i];
        p->id = i;

        p->sex = (rand_int(2) == 0) ? 'M' : 'F';
        p->name = (p->sex == 'M')
            ? name_cycle_next(&ctx->male_names)
            : name_cycle_next(&ctx->female_names);

        p->alias = nullptr;
        if (rand_int(10) < 3 && p->name) {
            p->alias = generate_nickname(&ctx->_scratch, p->name);
        }

        p->age = rand_actor_age();
        p->height = (person_height)rand_int(HEIGHT_COUNT);
        p->build = (person_build)rand_int(BUILD_COUNT);
        p->hair = (hair_color)rand_int(HAIR_COUNT);

        p->home_location_id = find_residence(ctx);
        p->work_location_id = (rand_int(10) < 7) ? find_workplace(ctx) : -1;  // 70% employed
        p->occupation = (p->work_location_id >= 0)
            ? occupations[rand_int(num_occupations)]
            : "Unemployed";
        p->income = (p->work_location_id >= 0) ? rand_int_min(3, 10) : rand_int_min(1, 4);

        p->phone_number = generate_phone_number(&ctx->_scratch);
        p->org_affiliation = nullptr;

        personality_archetype arch = (personality_archetype)rand_weighted_index(
            rand_float01(), archetype_weights, ARCHETYPE_COUNT);
        apply_archetype(p, arch);
        archetype_counts[arch]++;

        p->need_money = rand_int_min(40, 80);
        p->need_belonging = rand_int_min(40, 80);
        p->need_status = rand_int_min(40, 80);
        p->need_security = rand_int_min(40, 80);

        db_insert_person(ctx->db, p);
        db_add_resident(ctx->db, p->home_location_id, p->id);
    }

    printf("[WORLD] Generated %d people (archetypes: V=%d, G=%d, L=%d, A=%d)\n",
        ctx->num_people,
        archetype_counts[ARCHETYPE_VOLATILE],
        archetype_counts[ARCHETYPE_GREEDY],
        archetype_counts[ARCHETYPE_LOYAL],
        archetype_counts[ARCHETYPE_AVERAGE]);
}

static void add_relationship(world_ctx *ctx, i64 p1, i64 p2, relationship_type type, i32 strength = -1) {
    if (ctx->num_relationships >= ctx->num_people * 4) return;  // Safety limit

    relationship *r = &ctx->relationships[ctx->num_relationships++];
    r->person1_id = p1;
    r->person2_id = p2;
    r->type = type;

    if (strength < 0) {
        switch (type) {
            case REL_SPOUSE:
            case REL_SIBLING:
                strength = rand_int_min(60, 100);  // Family bonds are strong
                break;
            case REL_EX_SPOUSE:
            case REL_EX_PARTNER:
                strength = rand_int_min(20, 60);   // Complicated history
                break;
            case REL_FRIEND:
            case REL_PARTNER:
                strength = rand_int_min(40, 80);
                break;
            case REL_COLLEAGUE:
                strength = rand_int_min(20, 50);   // Acquaintances
                break;
            case REL_RIVAL:
                strength = rand_int_min(50, 90);   // Strong negative
                break;
            case REL_DEBTOR:
            case REL_CREDITOR:
                strength = rand_int_min(30, 70);   // Depends on debt size
                break;
            default:
                strength = rand_int_min(30, 70);
                break;
        }
    }
    r->strength = strength;

    db_insert_relationship(ctx->db, r);
}

static void generate_relationships(world_ctx *ctx) {
    // Allocate space for relationships (max ~4 per person)
    i32 max_rels = ctx->num_people * 4;
    ctx->relationships = (relationship *)mem_arena_alloc(&ctx->_scratch,
        sizeof(relationship) * max_rels, sizeof(void *)).p;
    ctx->num_relationships = 0;

    // Track who has relationships
    bool *has_spouse = (bool *)mem_arena_alloc(&ctx->_scratch, sizeof(bool) * ctx->num_people, 1).p;
    memset(has_spouse, 0, sizeof(bool) * ctx->num_people);

    //TODO: Should we add reverse relationships in all/most cases?

    // 1. Family clusters (~12% married)
    i32 target_marriages = ctx->num_people / 8;
    i32 marriages = 0;
    for (i32 i = 0; i < ctx->num_people && marriages < target_marriages; i++) {
        if (has_spouse[i]) continue;
        if (ctx->people[i].age < 22) continue;  // Too young

        // Find a suitable partner
        for (i32 j = i + 1; j < ctx->num_people; j++) {
            if (has_spouse[j]) continue;
            if (ctx->people[j].age < 22) continue;
            if (abs(ctx->people[i].age - ctx->people[j].age) > 15) continue;
            if (ctx->people[i].sex == ctx->people[j].sex && rand_int(5) != 0) continue;  // 20% same-sex

            //TODO: Move people so they live in the same residence?
            //TODO: Portion of the relationships start as REL_EX_SPOUSE
            add_relationship(ctx, i, j, REL_SPOUSE);
            has_spouse[i] = has_spouse[j] = true;
            marriages++;
            break;
        }
    }
    //TODO: Selecting siblings, ones that don't have spouse

    // 2. Colleagues (same workplace)
    for (i32 i = 0; i < ctx->num_people; i++) {
        if (ctx->people[i].work_location_id < 0) continue;

        for (i32 j = i + 1; j < ctx->num_people; j++) {
            if (ctx->people[j].work_location_id != ctx->people[i].work_location_id) continue;
            if (rand_int(3) != 0) continue;  // Only 33% become close colleagues

            add_relationship(ctx, i, j, REL_COLLEAGUE);
        }
    }

    // 3. Friends (random, same neighborhood)
    i32 target_friendships = ctx->num_people / 4;
    for (i32 f = 0; f < target_friendships; f++) {
        i32 p1 = rand_int(ctx->num_people);
        i32 p2 = rand_int(ctx->num_people);
        if (p1 == p2) continue;

        add_relationship(ctx, p1, p2, REL_FRIEND);
    }

    // 4. Rivals (~5% of people have rivals)
    i32 target_rivals = ctx->num_people / 20;
    for (i32 r = 0; r < target_rivals; r++) {
        i32 p1 = rand_int(ctx->num_people);
        i32 p2 = rand_int(ctx->num_people);
        if (p1 == p2) continue;

        add_relationship(ctx, p1, p2, REL_RIVAL);
    }

    // 5. Debtor/Creditor (~8% of people have debts)
    i32 target_debts = ctx->num_people / 12;
    for (i32 d = 0; d < target_debts; d++) {
        i32 debtor = rand_int(ctx->num_people);
        i32 creditor = rand_int(ctx->num_people);
        if (debtor == creditor) continue;

        // Greedy/wealthy people more likely to be creditors
        if (ctx->people[creditor].income < 5 && rand_int(2) == 0) continue;

        add_relationship(ctx, debtor, creditor, REL_DEBTOR);
        //TODO: Should we add REL_CREDITOR with reverse people ids?
    }

    // 6. Ex-partners (~6% of adults)
    i32 target_exes = ctx->num_people / 16;
    for (i32 e = 0; e < target_exes; e++) {
        i32 p1 = rand_int(ctx->num_people);
        i32 p2 = rand_int(ctx->num_people);
        if (p1 == p2) continue;
        if (ctx->people[p1].age < 25 || ctx->people[p2].age < 25) continue;
        if (has_spouse[p1] && rand_int(3) != 0) continue;  // Married people less likely

        //TODO: Some odds to be REL_PARTNER instead
        add_relationship(ctx, p1, p2, REL_EX_PARTNER);
    }

    //TODO: Find REL_EMPLOYER/REL_EMPLOYEE relations, need to validate working at same location id

    printf("[WORLD] Generated %d relationships\n", ctx->num_relationships);
}

static void generate_objects(world_ctx *ctx) {
    i32 num_phones = ctx->num_people;
    i32 num_vehicles = 0;
    for (i32 i = 0; i < ctx->num_people; i++) {
        if (ctx->people[i].income > 5) num_vehicles++;
    }

    ctx->num_objects = num_phones + num_vehicles;
    ctx->objects = (sim_object *)mem_arena_alloc(&ctx->_scratch,
        sizeof(sim_object) * ctx->num_objects, sizeof(void *)).p;

    i32 obj_id = 0;

    for (i32 i = 0; i < ctx->num_people; i++) {
        sim_object *obj = &ctx->objects[obj_id];
        obj->id = obj_id++;
        obj->name = alloc_string(&ctx->_scratch, "%s's Phone", ctx->people[i].name);
        obj->type = OBJ_PERSONAL_ITEM;
        obj->serial_number = nullptr;
        obj->size = SIZE_SMALL;
        obj->color = (object_color)rand_int(COLOR_COUNT);
        obj->material = MAT_METAL;
        obj->condition = (rand_int(3) == 0) ? COND_WORN : COND_GOOD;
        obj->owner_id = i;
        obj->location_id = -1;  // With owner

        db_insert_object(ctx->db, obj);
    }

    static const char *vehicle_types[] = { "Sedan", "SUV", "Truck", "Coupe", "Hatchback" };
    for (i32 i = 0; i < ctx->num_people; i++) {
        if (ctx->people[i].income <= 5) continue;

        sim_object *obj = &ctx->objects[obj_id];
        obj->id = obj_id++;
        obj->name = alloc_string(&ctx->_scratch, "%s's %s",
            ctx->people[i].name,
            vehicle_types[rand_int(5)]);
        obj->type = OBJ_VEHICLE;
        obj->serial_number = nullptr;
        obj->size = SIZE_LARGE;
        obj->color = (object_color)rand_int(COLOR_COUNT);
        obj->material = MAT_METAL;
        obj->condition = (ctx->people[i].income > 7) ? COND_NEW : COND_GOOD;
        obj->owner_id = i;
        obj->location_id = ctx->people[i].home_location_id;

        db_insert_object(ctx->db, obj);
    }

    //TODO: Maybe have odds to generate weapons for some people? gun carry?

    printf("[WORLD] Generated %d objects (%d phones, %d vehicles)\n",
        ctx->num_objects, num_phones, num_vehicles);
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void world_init(world_ctx *ctx, i32 seed) {
    memset(ctx, 0, sizeof(world_ctx));
    mem_arena_init(&ctx->_scratch, WORLD_SCRATCH_SIZE);

    // Open in-memory SQLite database
    int res = sqlite3_open(":memory:", &ctx->db);
    assert(res == SQLITE_OK && "Could not open in-memory database");

    rand_seed(seed);
    db_init_tables(ctx->db);

    // Load name CSVs for actor names
    name_cycle_init(&ctx->male_names, ASSETS_DIR "m_names.csv");
    name_cycle_init(&ctx->female_names, ASSETS_DIR "f_names.csv");

    // Train Markov chain on city names for district/street generation
    name_gen_train(&ctx->district_names, ASSETS_DIR "city_names.csv");
    name_gen_train(&ctx->street_names, ASSETS_DIR "city_names.csv");

    printf("[WORLD] Initialized (loaded %d male, %d female names) w/ seed=%d\n",
        ctx->male_names.num_names, ctx->female_names.num_names, seed);
}

void world_clear(world_ctx *ctx, const char *db_save_path) {
    // Save database to file if path provided
    if (db_save_path && ctx->db) {
        db_save_to_file(ctx->db, db_save_path);
    }

    name_cycle_clear(&ctx->male_names);
    name_cycle_clear(&ctx->female_names);
    name_gen_clear(&ctx->street_names);
    name_gen_clear(&ctx->district_names);

    if (ctx->db) {
        sqlite3_close(ctx->db);
        ctx->db = nullptr;
    }

    mem_arena_clear(&ctx->_scratch);
}

void world_generate(world_ctx *ctx, city_size size) {
    ctx->size = size;
    printf("[WORLD] Generating world (size=%d)\n", size);

    generate_streets(ctx);
    generate_districts(ctx);
    generate_locations(ctx);
    generate_people(ctx);
    generate_relationships(ctx);
    generate_objects(ctx);

    printf("[WORLD] Generation complete!\n");
}

void world_print_summary(world_ctx *ctx) {
    printf("\n=== WORLD SUMMARY ===\n");
    printf("City size: %d\n", ctx->size);
    printf("Districts: %d\n", ctx->num_districts);
    printf("Locations: %d\n", ctx->num_locations);
    printf("People: %d\n", ctx->num_people);
    printf("Relationships: %d\n", ctx->num_relationships);
    printf("Objects: %d\n", ctx->num_objects);

    printf("\n--- Sample People ---\n");
    i32 sample = std::min(5, ctx->num_people);
    for (i32 i = 0; i < sample; i++) {
        person *p = &ctx->people[i];
        if (p->alias) {
            printf("  [%lld] %s aka \"%s\" (%c, %d) - %s, Income: %d, Archetype: %d\n",
                p->id, p->name, p->alias, p->sex, p->age, p->occupation, p->income, p->archetype);
        } else {
            printf("  [%lld] %s (%c, %d) - %s, Income: %d, Archetype: %d\n",
                p->id, p->name, p->sex, p->age, p->occupation, p->income, p->archetype);
        }
    }

    printf("\n--- Sample Relationships ---\n");
    i32 rel_sample = std::min(5, ctx->num_relationships);
    static const char *rel_names[] = {
        "spouse", "ex_spouse", "partner", "ex_partner", "parent", "child", "sibling",
        "friend", "colleague", "employer", "employee", "rival", "neighbor", "debtor", "creditor"
    };
    for (i32 i = 0; i < rel_sample; i++) {
        relationship *r = &ctx->relationships[i];
        printf("  %s -> %s (%s, str=%d)\n",
            ctx->people[r->person1_id].name,
            ctx->people[r->person2_id].name,
            rel_names[r->type],
            r->strength);
    }
    printf("=====================\n\n");
}
