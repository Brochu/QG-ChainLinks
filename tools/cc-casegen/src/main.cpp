#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "world_setup.h"
#include "sim_db.h"

int main(int argc, char** argv) {
    printf("[CC-CASEGEN] Case Generator v2\n");
    printf("==============================\n\n");

    // Parse command line args
    city_size size = CITY_SMALL;
    i32 seed = (i32)time(nullptr);
    const char *db_save_path = nullptr;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            int s = atoi(argv[++i]);
            if (s >= 0 && s < CITY_SIZE_COUNT) {
                size = (city_size)s;
            }
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--db") == 0 && i + 1 < argc) {
            db_save_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: casegen [--size 0-3] [--seed N] [--db FILE]\n");
            printf("  --size: 0=SMALL(50), 1=MEDIUM(80), 2=LARGE(120), 3=METRO(150)\n");
            printf("  --seed: Random seed for reproducibility\n");
            printf("  --db:   Save database to file (optional)\n");
            return 0;
        }
    }

    // Initialize and generate world
    world_ctx world;
    world_init(&world, seed);
    world_generate(&world, size);
    world_print_summary(&world);

    // Cleanup (saves DB to file if --db was specified)
    world_clear(&world, db_save_path);

    printf("[CC-CASEGEN] Done!\n");
    return 0;
}
