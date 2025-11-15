#pragma once

#include <cstdint>

#include <map>
#include <string>
#include <vector>

enum city_size : int8_t { SIZE_SMALL, SIZE_MEDIUM, SIZE_LARGE, SIZE_METRO, SIZE_COUNT };
enum district_type : int8_t { INDUSTRIAL, COMMERCIAL, RESIDENTIAL, NIGHT_LIFE, FINANCIAL, DISTRICT_COUNT };

struct district {
    int64_t id;
    district_type type;
};
extern int8_t size_to_districts[city_size::SIZE_COUNT];
extern int8_t district_weights[district_type::DISTRICT_COUNT];

enum class location_type : int32_t {
    //TODO: More variety
    // categorize by district type
    LOCATION_RESIDENTIAL,
    LOCATION_HOME = LOCATION_RESIDENTIAL,
    LOCATION_RESTAURANT,
    LOCATION_BANK,
    LOCATION_COUNT,
};

struct location {
    int64_t id;
    int64_t district;
};
extern int8_t size_to_locations[city_size::SIZE_COUNT];

struct city_gen {
    city_size size;
    int8_t num_districts;
    district districts[16];

    int8_t num_locations;
    location locations[128];
};

// --------------------------------------------

//TODO: Find a way to simplify this data structure; remove maps/vectors
struct name_generator {
    std::vector<std::string> starts;
    std::map<std::string, std::vector<char>> chains;
};

void generator_init();
void generator_stop();

void name_generator_train(name_generator *gen);
void name_generator_next(name_generator *gen, size_t num, std::vector<std::string> *out);

int rand_weighted_index(int8_t *weights, int num_items);
