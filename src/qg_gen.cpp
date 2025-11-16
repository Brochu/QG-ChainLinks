#include "generator.hpp"

int8_t size_to_districts[city_size::SIZE_COUNT] = { 1, 3, 5, 9 };
int8_t district_weights[district_type::DISTRICT_COUNT] = { 2, 3, 4, 1, 1 };

int8_t size_to_landmarks[city_size::SIZE_COUNT] = { 5, 15, 40, 80 };

void case_gen_fondation(case_gen *ctx, city_size s, int32_t seed) {
    srand(seed);
    ctx->size = s;
    ctx->num_landmarks = 0;

    ctx->num_districts = size_to_districts[s];
    for (int i = 0; i < ctx->num_districts; i++) {
        // Districts
        printf("[PROC-GEN] Generate a new district\n");
        district *d = &ctx->districts[i];
        d->id = i;
        d->type = district_type::RESIDENTIAL;

        for (int j = 0; j < size_to_landmarks[s]; j++) {
            // landmarks
            printf("[PROC-GEN] Generate a new landmark\n");
            landmark *loc = &ctx->landmarks[ctx->num_landmarks++];
            loc->id = ctx->num_landmarks - 1;
            loc->district = i;
        }
    }

    // Transit Links
    for (int i = 0; i < ctx->num_landmarks; i++) {
        for (int j = 0; j < ctx->num_landmarks; j++) {
        }
    }
    // Cut some links

    /*
    sqlite3 *db;
    int res = sqlite3_open("./detective.db", &db);
    printf("[PROC-GEN] Opening Case DB; res = %i\n", res);

    char *err = nullptr;
    res = sqlite3_exec(db, "SELECT * FROM users;", debug_callback, NULL, &err);
    printf("[PROC-GEN] EXEC res = %i; err = %s\n", res, err);

    res = sqlite3_close_v2(db);
    printf("[PROC-GEN] Closing Case DB; res = %i\n", res);
    printf("[PROC-GEN] Just testing, SQLITE_OK value = %i\n", SQLITE_OK);
    */
}

void case_gen_population(case_gen *ctx) { }
void case_gen_motive(case_gen *ctx) { }
void case_gen_crime(case_gen *ctx) { }
void case_gen_planning(case_gen *ctx) { }
void case_gen_exec(case_gen *ctx) { }
void case_gen_hook(case_gen *ctx) { }
void case_gen_polish(case_gen *ctx) { }

// ====================

void name_generator_train(name_generator *gen) {
    std::string data = "New York,Los Angeles,Chicago,Houston,Phoenix,Philadelphia,San Antonio,San Diego,Dallas,Jacksonville,Fort Worth,San Jose,Austin,Charlotte,Columbus,Indianapolis,San Francisco,Seattle,Denver,Oklahoma City,Nashville,Washington,El Paso,Las Vegas,Boston,Detroit,Louisville,Portland,Memphis,Baltimore,Milwaukee,Albuquerque,Tucson,Fresno,Sacramento,Atlanta,Mesa,Kansas City,Raleigh,Colorado Springs,Omaha,Miami,Virginia Beach,Long Beach,Oakland,Minneapolis,Bakersfield,Tulsa,Tampa,Arlington,Aurora,Wichita,Cleveland,New Orleans,Henderson,Honolulu,Anaheim,Orlando,Lexington,Stockton,Riverside,Irvine,Corpus Christi,Newark,Santa Ana,Cincinnati,Pittsburgh,Saint Paul,Greensboro,Jersey City,Durham,Lincoln,North Las Vegas,Plano,Anchorage,Gilbert,Madison,Reno,Chandler,St. Louis,Chula Vista,Buffalo,Fort Wayne,Lubbock,St. Petersburg,Toledo,Laredo,Port St. Lucie,Glendale,Irving,Winston-Salem,Chesapeake,Garland,Scottsdale,Boise,Hialeah,Frisco,Richmond,Cape Coral,Norfolk,Spokane,Huntsville,Santa Clarita,Tacoma,Fremont,McKinney,San Bernardino,Baton Rouge,Modesto,Fontana,Salt Lake City,Moreno Valley,Des Moines,Worcester,Yonkers,Fayetteville,Sioux Falls,Grand Prairie,Rochester,Tallahassee,Little Rock,Amarillo,Overland Park,Columbus,Augusta,Mobile,Oxnard,Grand Rapids,Peoria,Vancouver,Knoxville,Birmingham,Montgomery,Providence,Huntington Beach,Brownsville,Chattanooga,Fort Lauderdale,Tempe,Akron,Glendale,Clarksville,Ontario,Newport News,Elk Grove,Cary,Aurora,Salem,Pembroke Pines,Eugene,Santa Rosa,Rancho Cucamonga,Shreveport,Garden Grove,Oceanside,Fort Collins,Springfield,Murfreesboro,Surprise,Lancaster,Denton,Roseville,Palmdale,Corona,Salinas,Killeen,Paterson,Alexandria,Hollywood,Hayward,Charleston,Macon,Lakewood,Sunnyvale,Kansas City,Springfield,Bellevue,Naperville,Joliet,Bridgeport,Mesquite,Pasadena,Olathe,Escondido,Savannah,McAllen,Gainesville,Pomona,Rockford,Thornton,Waco,Visalia,Syracuse,Columbia,Midland,Miramar,Palm Bay,Lakewood,Jackson,Coral Springs,Victorville,Elizabeth,Fullerton,Meridian,Torrance,Stamford,West Valley City,Orange,Cedar Rapids,Warren,Hampton,New Haven,Pasadena,Kent,Dayton,Fargo,Lewisville,Carrollton,Round Rock,Sterling Heights,Santa Clara,Norman,Columbia,Abilene,Pearland,Athens,College Station,Clovis,West Palm Beach,Allentown,North Charleston,Simi Valley,Topeka,Wilmington,Lakeland,Thousand Oaks,Concord,Rochester,Vallejo,Ann Arbor,Broken Arrow,Fairfield,Lafayette,Hartford,Arvada,Berkeley,Independence,Billings,Cambridge,Lowell,Odessa,High Point,League City,Antioch,Richardson";
    size_t s = 0;
    size_t e = data.find(',', s);

    while (e != std::string::npos) {
        std::string name = "^^" + data.substr(s, e-s) + "$$";

        s = e + 1;
        e = data.find(',', s);

        gen->starts.push_back(name.substr(0, 3));

        for (int i = 0; i < name.size() - 3; i++) {
            std::string prefix = name.substr(i, 3);
            char next = name[i+3];
            gen->chains[prefix].emplace_back(next);
        }
    }
}

void name_generator_next(name_generator *gen, size_t num, std::vector<std::string> *out) {
    rand();
    for (int i = 0; i < num; i++) {
        size_t start_idx = (size_t)((rand()/(float)RAND_MAX) * gen->starts.size());
        std::string name = gen->starts[start_idx];

        char next = '\0';
        while (next != '$') {
            std::string prefix = name.substr(name.size()-3, 3);
            size_t next_idx = (size_t)((rand()/(float)RAND_MAX) * gen->chains[prefix].size());
            next = gen->chains[prefix][next_idx];
            name += next;
        }
        out->push_back(name);
    }
}

// ====================

int rand_weighted_index(int8_t *weights, int num_items) {
    rand();

    int sum = 0;
    for (int i = 0; i < num_items; i++) {
        sum += weights[i];
    }

    int target = (int)(((float)rand() / RAND_MAX) * sum) + 1;

    int current = 0;
    while (target > weights[current]) {
        target -= weights[current];
        current++;
    }
    return current;
}
