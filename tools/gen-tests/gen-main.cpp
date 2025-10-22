#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include <map>
#include <string>
#include <vector>

// Generation / Simulation passes definition
// 1. Fondation pass - City Districts; Landmarks and Locations; Transits between Landmarks

enum class city_e : int8_t {
    CITY_SMALL,
    CITY_MEDIUM,
    CITY_LARGE,
    CITY_METRO,
};

struct district_t {
    int64_t id;
};

enum class location_e : int32_t {
    LOCATION_NONE,
    LOCATION_RESTAURANT,
    LOCATION_BANK,
    LOCATION_COUNT,
};

struct location_t {
    int64_t id;
    const char *name;
    location_e type;

    int64_t district;
};

struct citygen_t {
};

void citygen_next(citygen_t ctx, city_e size) {
}

// =============================================================

struct namegen_t {
    std::vector<std::string> starts;
    std::map<std::string, std::vector<char>> chains;
};
void namegen_train(namegen_t *gen);
void namegen_next(namegen_t *gen, size_t num, std::vector<std::string> *out);

int main(int argc, char **argv) {
    srand(time(0));
    printf("[PROC-GEN] Testing history/events generation & simulation\n");

    namegen_t gen;
    namegen_train(&gen);

    std::vector<std::string> names;
    namegen_next(&gen, 25, &names);
    for (auto s : names) {
        printf(" - Generated name: %s\n", s.c_str());
    }

    return 0;
}

// NAME GENERATOR ----------------------------------------------------------------
void namegen_train(namegen_t *gen) {
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

void namegen_next(namegen_t *gen, size_t num, std::vector<std::string> *out) {
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

// SQLITE ----------------------------------------------------------------
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
