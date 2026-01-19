#include "sim_db.h"
#include <cstdio>
#include <cstring>

//------------------------------------------------------------------------------
// JSON Output Helper
//------------------------------------------------------------------------------

static void write_json_string(FILE* f, const char* str) {
    fprintf(f, "\"");
    while (*str) {
        switch (*str) {
            case '"': fprintf(f, "\\\""); break;
            case '\\': fprintf(f, "\\\\"); break;
            case '\n': fprintf(f, "\\n"); break;
            case '\r': fprintf(f, "\\r"); break;
            case '\t': fprintf(f, "\\t"); break;
            default: fputc(*str, f);
        }
        str++;
    }
    fprintf(f, "\"");
}

//------------------------------------------------------------------------------
// Case File JSON Output
//------------------------------------------------------------------------------

bool output_case_json(sim_context* ctx, const case_file& cf, const char* filepath) {
    FILE* f = fopen(filepath, "w");
    if (!f) {
        printf("[ERROR] Failed to open %s for writing\n", filepath);
        return false;
    }

    actor perp, victim;
    db_get_actor(ctx, cf.selected_crime.perpetrator_id, &perp);
    db_get_actor(ctx, cf.selected_crime.victim_id, &victim);

    location crime_loc;
    db_get_location(ctx, cf.selected_crime.location_id, &crime_loc);

    fprintf(f, "{\n");

    // Crime details
    fprintf(f, "  \"crime\": {\n");
    fprintf(f, "    \"type\": \"%s\",\n", crime_type_str(cf.selected_crime.type));
    fprintf(f, "    \"perpetrator\": {\n");
    fprintf(f, "      \"id\": %lld,\n", cf.selected_crime.perpetrator_id);
    fprintf(f, "      \"name\": "); write_json_string(f, perp.name); fprintf(f, ",\n");
    fprintf(f, "      \"age\": %d,\n", perp.age);
    fprintf(f, "      \"occupation\": "); write_json_string(f, perp.occupation); fprintf(f, "\n");
    fprintf(f, "    },\n");
    fprintf(f, "    \"victim\": {\n");
    fprintf(f, "      \"id\": %lld,\n", cf.selected_crime.victim_id);
    fprintf(f, "      \"name\": "); write_json_string(f, victim.name); fprintf(f, ",\n");
    fprintf(f, "      \"age\": %d,\n", victim.age);
    fprintf(f, "      \"occupation\": "); write_json_string(f, victim.occupation); fprintf(f, "\n");
    fprintf(f, "    },\n");
    fprintf(f, "    \"location\": {\n");
    fprintf(f, "      \"id\": %lld,\n", cf.selected_crime.location_id);
    fprintf(f, "      \"name\": "); write_json_string(f, crime_loc.name); fprintf(f, "\n");
    fprintf(f, "    },\n");
    fprintf(f, "    \"day\": %d,\n", timestamp_to_day(cf.selected_crime.timestamp));
    fprintf(f, "    \"hour\": %d,\n", timestamp_to_hour(cf.selected_crime.timestamp));
    fprintf(f, "    \"motive\": \"%s\"\n", motive_type_str(cf.selected_crime.motive));
    fprintf(f, "  },\n");

    // Causality chain
    fprintf(f, "  \"causality_chain\": [\n");
    for (i32 i = 0; i < cf.chain_length; i++) {
        fprintf(f, "    {\n");
        fprintf(f, "      \"event_id\": %lld,\n", cf.chain[i].event_id);
        fprintf(f, "      \"day\": %d,\n", cf.chain[i].day);
        fprintf(f, "      \"summary\": "); write_json_string(f, cf.chain[i].summary); fprintf(f, "\n");
        fprintf(f, "    }%s\n", (i < cf.chain_length - 1) ? "," : "");
    }
    fprintf(f, "  ],\n");

    // Suspects
    fprintf(f, "  \"suspects\": [\n");
    for (i32 i = 0; i < cf.num_suspects; i++) {
        actor s;
        if (db_get_actor(ctx, cf.suspect_ids[i], &s)) {
            fprintf(f, "    {\n");
            fprintf(f, "      \"id\": %lld,\n", cf.suspect_ids[i]);
            fprintf(f, "      \"name\": "); write_json_string(f, s.name); fprintf(f, ",\n");
            fprintf(f, "      \"age\": %d,\n", s.age);
            fprintf(f, "      \"occupation\": "); write_json_string(f, s.occupation); fprintf(f, ",\n");
            fprintf(f, "      \"is_perpetrator\": %s\n",
                   (cf.suspect_ids[i] == cf.selected_crime.perpetrator_id) ? "true" : "false");
            fprintf(f, "    }%s\n", (i < cf.num_suspects - 1) ? "," : "");
        }
    }
    fprintf(f, "  ],\n");

    // Evidence
    fprintf(f, "  \"evidence\": [\n");
    for (i32 i = 0; i < cf.num_evidence; i++) {
        fprintf(f, "    {\n");
        fprintf(f, "      \"id\": %lld,\n", cf.evidence_list[i].id);
        fprintf(f, "      \"type\": \"%s\",\n", evidence_type_str(cf.evidence_list[i].type));
        fprintf(f, "      \"description\": "); write_json_string(f, cf.evidence_list[i].description); fprintf(f, ",\n");
        fprintf(f, "      \"clarity\": %d,\n", cf.evidence_list[i].clarity);
        fprintf(f, "      \"points_to_id\": %lld\n", cf.evidence_list[i].points_to_id);
        fprintf(f, "    }%s\n", (i < cf.num_evidence - 1) ? "," : "");
    }
    fprintf(f, "  ],\n");

    // Witnesses
    fprintf(f, "  \"witnesses\": [\n");
    for (i32 i = 0; i < cf.num_witnesses; i++) {
        actor w;
        if (db_get_actor(ctx, cf.witnesses[i].actor_id, &w)) {
            fprintf(f, "    {\n");
            fprintf(f, "      \"id\": %lld,\n", cf.witnesses[i].id);
            fprintf(f, "      \"name\": "); write_json_string(f, w.name); fprintf(f, ",\n");
            fprintf(f, "      \"saw_what\": "); write_json_string(f, cf.witnesses[i].saw_what); fprintf(f, ",\n");
            fprintf(f, "      \"reliability\": %d,\n", cf.witnesses[i].reliability);
            fprintf(f, "      \"willingness\": %d\n", cf.witnesses[i].willingness);
            fprintf(f, "    }%s\n", (i < cf.num_witnesses - 1) ? "," : "");
        }
    }
    fprintf(f, "  ],\n");

    // Blanks (player challenge)
    const char* type_names[] = { "WHO", "WHERE", "WHEN", "WHAT" };

    fprintf(f, "  \"blanks\": [\n");
    for (i32 i = 0; i < cf.num_blanks; i++) {
        fprintf(f, "    {\n");
        fprintf(f, "      \"type\": \"%s\",\n", type_names[(i32)cf.blanks[i].type]);
        fprintf(f, "      \"chain_index\": %d,\n", cf.blanks[i].chain_index);
        fprintf(f, "      \"correct_answer\": "); write_json_string(f, cf.blanks[i].correct_answer); fprintf(f, ",\n");
        fprintf(f, "      \"evidence_ids\": [");
        for (i32 j = 0; j < cf.blanks[i].num_evidence; j++) {
            fprintf(f, "%lld%s", cf.blanks[i].evidence_ids[j],
                   (j < cf.blanks[i].num_evidence - 1) ? ", " : "");
        }
        fprintf(f, "]\n");
        fprintf(f, "    }%s\n", (i < cf.num_blanks - 1) ? "," : "");
    }
    fprintf(f, "  ]\n");

    fprintf(f, "}\n");

    fclose(f);
    printf("[OUTPUT] Case file written to: %s\n", filepath);
    return true;
}

//------------------------------------------------------------------------------
// Summary Output
//------------------------------------------------------------------------------

void output_case_summary(sim_context* ctx, const case_file& cf) {
    actor perp, victim;
    db_get_actor(ctx, cf.selected_crime.perpetrator_id, &perp);
    db_get_actor(ctx, cf.selected_crime.victim_id, &victim);

    location crime_loc;
    db_get_location(ctx, cf.selected_crime.location_id, &crime_loc);

    printf("\n");
    printf("============================================================\n");
    printf("                      CASE SUMMARY                          \n");
    printf("============================================================\n");
    printf("\n");
    printf("CRIME: %s\n", crime_type_str(cf.selected_crime.type));
    printf("DATE: Day %d\n", timestamp_to_day(cf.selected_crime.timestamp));
    printf("LOCATION: %s\n", crime_loc.name);
    printf("\n");
    printf("VICTIM:\n");
    printf("  Name: %s\n", victim.name);
    printf("  Age: %d\n", victim.age);
    printf("  Occupation: %s\n", victim.occupation);
    printf("\n");
    printf("MOTIVE: %s\n", motive_type_str(cf.selected_crime.motive));
    printf("\n");
    printf("CHAIN OF EVENTS:\n");
    for (i32 i = 0; i < cf.chain_length; i++) {
        printf("  Day %3d: %s\n", cf.chain[i].day, cf.chain[i].summary);
    }
    printf("\n");
    printf("SUSPECTS (%d):\n", cf.num_suspects);
    for (i32 i = 0; i < cf.num_suspects; i++) {
        actor s;
        if (db_get_actor(ctx, cf.suspect_ids[i], &s)) {
            printf("  - %s (%s, age %d)%s\n", s.name, s.occupation, s.age,
                   (cf.suspect_ids[i] == cf.selected_crime.perpetrator_id) ? " [PERPETRATOR]" : "");
        }
    }
    printf("\n");
    printf("EVIDENCE (%d pieces):\n", cf.num_evidence);
    for (i32 i = 0; i < cf.num_evidence && i < 8; i++) {
        printf("  - [%s] %s\n",
               evidence_type_str(cf.evidence_list[i].type),
               cf.evidence_list[i].description);
    }
    if (cf.num_evidence > 8) {
        printf("  ... and %d more\n", cf.num_evidence - 8);
    }
    printf("\n");
    printf("PLAYER CHALLENGE - Fill in the blanks:\n");
    const char* type_names[] = { "WHO", "WHERE", "WHEN", "WHAT" };
    for (i32 i = 0; i < cf.num_blanks; i++) {
        printf("  %d. [%s] _______ (supported by %d evidence)\n",
               i + 1, type_names[(i32)cf.blanks[i].type], cf.blanks[i].num_evidence);
    }
    printf("\n");
    printf("============================================================\n");
}
