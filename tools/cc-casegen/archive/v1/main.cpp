#include "sim_db.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

//------------------------------------------------------------------------------
// Phase Forward Declarations
//------------------------------------------------------------------------------

bool phase_foundation(sim_context* ctx);
bool phase_population(sim_context* ctx);
bool phase_simulation(sim_context* ctx);
bool phase_selection(sim_context* ctx, crime* selected_crime);
bool phase_evidence(sim_context* ctx, const crime& selected_crime);
bool phase_extraction(sim_context* ctx, const crime& selected_crime, case_file* output);
bool phase_blanks(sim_context* ctx, case_file* cf);
bool output_case_json(sim_context* ctx, const case_file& cf, const char* filepath);
void output_case_summary(sim_context* ctx, const case_file& cf);

//------------------------------------------------------------------------------
// Usage
//------------------------------------------------------------------------------

static void print_usage(const char* prog) {
    printf("Usage: %s [options]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -size <small|medium|large|metro>  City size (default: medium)\n");
    printf("  -seed <number>                    Random seed (default: time-based)\n");
    printf("  -out <filename>                   Output JSON file (default: case.json)\n");
    printf("  -db <filename>                    Export database to file\n");
    printf("  -h, --help                        Show this help\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s -size large -seed 12345 -out my_case.json -db simulation.db\n", prog);
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    printf("=== Case Generation System ===\n");
    printf("Dwarf Fortress-inspired procedural crime simulation\n\n");

    // Parse arguments
    city_size size = city_size::MEDIUM;
    i32 seed = (i32)time(nullptr);
    const char* output_file = "case.json";
    const char* db_export_path = nullptr;

    for (i32 i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-size") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "small") == 0) size = city_size::SMALL;
            else if (strcmp(argv[i], "medium") == 0) size = city_size::MEDIUM;
            else if (strcmp(argv[i], "large") == 0) size = city_size::LARGE;
            else if (strcmp(argv[i], "metro") == 0) size = city_size::METRO;
            else {
                printf("Unknown city size: %s\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-out") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-db") == 0 && i + 1 < argc) {
            db_export_path = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    const char* size_names[] = { "small", "medium", "large", "metro" };
    printf("Configuration:\n");
    printf("  City Size: %s\n", size_names[(i32)size]);
    printf("  Random Seed: %d\n", seed);
    printf("  Output: %s\n", output_file);
    if (db_export_path) printf("  DB Export: %s\n", db_export_path);
    printf("\n");

    // Initialize random number generator
    srand(seed);

    // Initialize simulation context
    sim_context ctx = {};
    ctx.size = size;
    ctx.seed = seed;

    // Initialize database
    if (!db_init(&ctx)) {
        printf("[ERROR] Failed to initialize database\n");
        return 1;
    }

    // Run phases
    printf("\n--- GENERATION PHASES ---\n\n");

    // Phase 1: Foundation
    if (!phase_foundation(&ctx)) {
        printf("[ERROR] Foundation phase failed\n");
        db_close(&ctx);
        return 1;
    }

    // Phase 2: Population
    if (!phase_population(&ctx)) {
        printf("[ERROR] Population phase failed\n");
        db_close(&ctx);
        return 1;
    }

    // Phase 3: Simulation
    if (!phase_simulation(&ctx)) {
        printf("[ERROR] Simulation phase failed\n");
        db_close(&ctx);
        return 1;
    }

    // Phase 4: (Crime emergence happens during simulation)
    printf("[PHASE 4] Crime Emergence - Integrated into simulation\n");

    // Phase 5: Selection
    crime selected_crime;
    if (!phase_selection(&ctx, &selected_crime)) {
        printf("[ERROR] Selection phase failed - no crimes to select\n");

        // If no crimes, generate a forced one for testing
        printf("\n[FALLBACK] Attempting to generate a case anyway...\n");

        // Export database for debugging
        if (db_export_path) {
            db_export(&ctx, db_export_path);
        }

        db_close(&ctx);
        return 1;
    }

    // Phase 6: Evidence
    if (!phase_evidence(&ctx, selected_crime)) {
        printf("[WARNING] Evidence phase generated no evidence\n");
    }

    // Phase 7: Extraction
    case_file cf;
    if (!phase_extraction(&ctx, selected_crime, &cf)) {
        printf("[ERROR] Extraction phase failed\n");
        db_close(&ctx);
        return 1;
    }

    // Phase 8: Blanks
    if (!phase_blanks(&ctx, &cf)) {
        printf("[WARNING] Blanks phase generated no blanks\n");
    }

    // Output results
    printf("\n--- OUTPUT ---\n\n");

    output_case_summary(&ctx, cf);
    output_case_json(&ctx, cf, output_file);

    // Export database if requested
    if (db_export_path) {
        db_export(&ctx, db_export_path);
    }

    // Cleanup
    delete[] cf.chain;
    delete[] cf.suspect_ids;
    delete[] cf.evidence_list;
    delete[] cf.witnesses;
    for (i32 i = 0; i < cf.num_blanks; i++) {
        delete[] cf.blanks[i].evidence_ids;
    }
    delete[] cf.blanks;

    db_close(&ctx);

    printf("\n[DONE] Case generation complete!\n");
    return 0;
}
