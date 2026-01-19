#include "sim_db.h"
#include <cstdio>
#include <cstring>

//------------------------------------------------------------------------------
// Helper Macros
//------------------------------------------------------------------------------

#define SQL_EXEC(db, sql) \
    do { \
        char* err = nullptr; \
        if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) { \
            printf("[DB ERROR] %s: %s\n", #sql, err); \
            sqlite3_free(err); \
            return false; \
        } \
    } while(0)

#define SQL_PREPARE(db, sql, stmt) \
    do { \
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) { \
            printf("[DB ERROR] prepare: %s\n", sqlite3_errmsg(db)); \
            return -1; \
        } \
    } while(0)

//------------------------------------------------------------------------------
// Database Lifecycle
//------------------------------------------------------------------------------

static const char* SCHEMA = R"(
    CREATE TABLE IF NOT EXISTS districts (
        id INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        type INTEGER NOT NULL,
        wealth_level INTEGER NOT NULL,
        crime_rate INTEGER NOT NULL
    );

    CREATE TABLE IF NOT EXISTS locations (
        id INTEGER PRIMARY KEY,
        district_id INTEGER NOT NULL,
        name TEXT NOT NULL,
        type INTEGER NOT NULL,
        is_public INTEGER NOT NULL,
        open_hour INTEGER,
        close_hour INTEGER,
        security INTEGER NOT NULL DEFAULT 0,
        capacity INTEGER NOT NULL DEFAULT 10,
        FOREIGN KEY (district_id) REFERENCES districts(id)
    );

    CREATE TABLE IF NOT EXISTS actors (
        id INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        age INTEGER NOT NULL,
        sex TEXT NOT NULL,
        occupation TEXT,
        income INTEGER NOT NULL DEFAULT 5,
        home_id INTEGER,
        work_id INTEGER,
        impulsivity INTEGER NOT NULL DEFAULT 50,
        morality INTEGER NOT NULL DEFAULT 50,
        greed INTEGER NOT NULL DEFAULT 50,
        aggression INTEGER NOT NULL DEFAULT 50,
        loyalty INTEGER NOT NULL DEFAULT 50,
        need_money INTEGER NOT NULL DEFAULT 50,
        need_belonging INTEGER NOT NULL DEFAULT 50,
        need_status INTEGER NOT NULL DEFAULT 50,
        need_security INTEGER NOT NULL DEFAULT 50,
        FOREIGN KEY (home_id) REFERENCES locations(id),
        FOREIGN KEY (work_id) REFERENCES locations(id)
    );

    CREATE TABLE IF NOT EXISTS relationships (
        id INTEGER PRIMARY KEY,
        actor1_id INTEGER NOT NULL,
        actor2_id INTEGER NOT NULL,
        type INTEGER NOT NULL,
        subtype INTEGER NOT NULL DEFAULT 0,
        strength INTEGER NOT NULL DEFAULT 50,
        sentiment INTEGER NOT NULL DEFAULT 0,
        FOREIGN KEY (actor1_id) REFERENCES actors(id),
        FOREIGN KEY (actor2_id) REFERENCES actors(id)
    );

    CREATE TABLE IF NOT EXISTS events (
        id INTEGER PRIMARY KEY,
        timestamp INTEGER NOT NULL,
        type INTEGER NOT NULL,
        actor_id INTEGER NOT NULL,
        target_id INTEGER,
        location_id INTEGER,
        details TEXT,
        FOREIGN KEY (actor_id) REFERENCES actors(id),
        FOREIGN KEY (target_id) REFERENCES actors(id),
        FOREIGN KEY (location_id) REFERENCES locations(id)
    );

    CREATE TABLE IF NOT EXISTS grievances (
        id INTEGER PRIMARY KEY,
        victim_id INTEGER NOT NULL,
        offender_id INTEGER NOT NULL,
        cause_event_id INTEGER,
        severity INTEGER NOT NULL DEFAULT 10,
        timestamp INTEGER NOT NULL,
        resolved INTEGER NOT NULL DEFAULT 0,
        FOREIGN KEY (victim_id) REFERENCES actors(id),
        FOREIGN KEY (offender_id) REFERENCES actors(id),
        FOREIGN KEY (cause_event_id) REFERENCES events(id)
    );

    CREATE TABLE IF NOT EXISTS crimes (
        id INTEGER PRIMARY KEY,
        type INTEGER NOT NULL,
        perpetrator_id INTEGER NOT NULL,
        victim_id INTEGER NOT NULL,
        location_id INTEGER NOT NULL,
        timestamp INTEGER NOT NULL,
        motive INTEGER NOT NULL DEFAULT 0,
        interest_score REAL NOT NULL DEFAULT 0.0,
        FOREIGN KEY (perpetrator_id) REFERENCES actors(id),
        FOREIGN KEY (victim_id) REFERENCES actors(id),
        FOREIGN KEY (location_id) REFERENCES locations(id)
    );

    CREATE TABLE IF NOT EXISTS evidence (
        id INTEGER PRIMARY KEY,
        crime_id INTEGER NOT NULL,
        type INTEGER NOT NULL,
        location_id INTEGER,
        points_to_id INTEGER,
        clarity INTEGER NOT NULL DEFAULT 50,
        description TEXT,
        FOREIGN KEY (crime_id) REFERENCES crimes(id),
        FOREIGN KEY (location_id) REFERENCES locations(id),
        FOREIGN KEY (points_to_id) REFERENCES actors(id)
    );

    CREATE TABLE IF NOT EXISTS witnesses (
        id INTEGER PRIMARY KEY,
        crime_id INTEGER NOT NULL,
        actor_id INTEGER NOT NULL,
        saw_what TEXT,
        reliability INTEGER NOT NULL DEFAULT 50,
        willingness INTEGER NOT NULL DEFAULT 50,
        FOREIGN KEY (crime_id) REFERENCES crimes(id),
        FOREIGN KEY (actor_id) REFERENCES actors(id)
    );

    CREATE TABLE IF NOT EXISTS actor_states (
        id INTEGER PRIMARY KEY,
        timestamp INTEGER NOT NULL,
        actor_id INTEGER NOT NULL,
        location_id INTEGER NOT NULL,
        activity TEXT,
        FOREIGN KEY (actor_id) REFERENCES actors(id),
        FOREIGN KEY (location_id) REFERENCES locations(id)
    );

    CREATE INDEX IF NOT EXISTS idx_events_timestamp ON events(timestamp);
    CREATE INDEX IF NOT EXISTS idx_events_actor ON events(actor_id);
    CREATE INDEX IF NOT EXISTS idx_events_location ON events(location_id);
    CREATE INDEX IF NOT EXISTS idx_grievances_victim ON grievances(victim_id);
    CREATE INDEX IF NOT EXISTS idx_actor_states_actor ON actor_states(actor_id);
    CREATE INDEX IF NOT EXISTS idx_actor_states_time ON actor_states(timestamp);
)";

bool db_init(sim_context* ctx) {
    int rc = sqlite3_open(":memory:", &ctx->db);
    if (rc != SQLITE_OK) {
        printf("[DB ERROR] Failed to open database: %s\n", sqlite3_errmsg(ctx->db));
        return false;
    }

    // Enable foreign keys
    SQL_EXEC(ctx->db, "PRAGMA foreign_keys = ON;");

    // Create schema
    char* err = nullptr;
    rc = sqlite3_exec(ctx->db, SCHEMA, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        printf("[DB ERROR] Schema creation failed: %s\n", err);
        sqlite3_free(err);
        return false;
    }

    printf("[DB] Initialized in-memory database\n");
    return true;
}

bool db_export(sim_context* ctx, const char* filepath) {
    sqlite3* file_db;
    int rc = sqlite3_open(filepath, &file_db);
    if (rc != SQLITE_OK) {
        printf("[DB ERROR] Failed to create export file: %s\n", sqlite3_errmsg(file_db));
        return false;
    }

    sqlite3_backup* backup = sqlite3_backup_init(file_db, "main", ctx->db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }

    rc = sqlite3_errcode(file_db);
    sqlite3_close(file_db);

    if (rc == SQLITE_OK) {
        printf("[DB] Exported to: %s\n", filepath);
        return true;
    }

    printf("[DB ERROR] Export failed\n");
    return false;
}

void db_close(sim_context* ctx) {
    if (ctx->db) {
        sqlite3_close(ctx->db);
        ctx->db = nullptr;
    }
}

//------------------------------------------------------------------------------
// District Operations
//------------------------------------------------------------------------------

i64 db_insert_district(sim_context* ctx, const char* name, district_type type,
                       i32 wealth_level, i32 crime_rate) {
    const char* sql = "INSERT INTO districts (name, type, wealth_level, crime_rate) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, (int)type);
    sqlite3_bind_int(stmt, 3, wealth_level);
    sqlite3_bind_int(stmt, 4, crime_rate);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;

    ctx->num_districts++;
    return sqlite3_last_insert_rowid(ctx->db);
}

bool db_get_district(sim_context* ctx, i64 id, district* out) {
    const char* sql = "SELECT id, name, type, wealth_level, crime_rate FROM districts WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id = sqlite3_column_int64(stmt, 0);
        strncpy(out->name, (const char*)sqlite3_column_text(stmt, 1), sizeof(out->name) - 1);
        out->type = (district_type)sqlite3_column_int(stmt, 2);
        out->wealth_level = sqlite3_column_int(stmt, 3);
        out->crime_rate = sqlite3_column_int(stmt, 4);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

i32 db_get_all_districts(sim_context* ctx, district* out, i32 max_count) {
    const char* sql = "SELECT id, name, type, wealth_level, crime_rate FROM districts;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        strncpy(out[count].name, (const char*)sqlite3_column_text(stmt, 1), sizeof(out[count].name) - 1);
        out[count].type = (district_type)sqlite3_column_int(stmt, 2);
        out[count].wealth_level = sqlite3_column_int(stmt, 3);
        out[count].crime_rate = sqlite3_column_int(stmt, 4);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

//------------------------------------------------------------------------------
// Location Operations
//------------------------------------------------------------------------------

i64 db_insert_location(sim_context* ctx, i64 district_id, const char* name,
                       location_type type, bool is_public, i32 open_hour,
                       i32 close_hour, security_level security, i32 capacity) {
    const char* sql = "INSERT INTO locations (district_id, name, type, is_public, open_hour, close_hour, security, capacity) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_int64(stmt, 1, district_id);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, (int)type);
    sqlite3_bind_int(stmt, 4, is_public ? 1 : 0);
    sqlite3_bind_int(stmt, 5, open_hour);
    sqlite3_bind_int(stmt, 6, close_hour);
    sqlite3_bind_int(stmt, 7, (int)security);
    sqlite3_bind_int(stmt, 8, capacity);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;

    ctx->num_locations++;
    return sqlite3_last_insert_rowid(ctx->db);
}

bool db_get_location(sim_context* ctx, i64 id, location* out) {
    const char* sql = "SELECT id, district_id, name, type, is_public, open_hour, close_hour, security, capacity FROM locations WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id = sqlite3_column_int64(stmt, 0);
        out->district_id = sqlite3_column_int64(stmt, 1);
        strncpy(out->name, (const char*)sqlite3_column_text(stmt, 2), sizeof(out->name) - 1);
        out->type = (location_type)sqlite3_column_int(stmt, 3);
        out->is_public = sqlite3_column_int(stmt, 4) != 0;
        out->open_hour = sqlite3_column_int(stmt, 5);
        out->close_hour = sqlite3_column_int(stmt, 6);
        out->security = (security_level)sqlite3_column_int(stmt, 7);
        out->capacity = sqlite3_column_int(stmt, 8);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

i32 db_get_locations_by_district(sim_context* ctx, i64 district_id,
                                  location* out, i32 max_count) {
    const char* sql = "SELECT id, district_id, name, type, is_public, open_hour, close_hour, security, capacity FROM locations WHERE district_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, district_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].district_id = sqlite3_column_int64(stmt, 1);
        strncpy(out[count].name, (const char*)sqlite3_column_text(stmt, 2), sizeof(out[count].name) - 1);
        out[count].type = (location_type)sqlite3_column_int(stmt, 3);
        out[count].is_public = sqlite3_column_int(stmt, 4) != 0;
        out[count].open_hour = sqlite3_column_int(stmt, 5);
        out[count].close_hour = sqlite3_column_int(stmt, 6);
        out[count].security = (security_level)sqlite3_column_int(stmt, 7);
        out[count].capacity = sqlite3_column_int(stmt, 8);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_get_locations_by_type(sim_context* ctx, location_type type,
                              location* out, i32 max_count) {
    const char* sql = "SELECT id, district_id, name, type, is_public, open_hour, close_hour, security, capacity FROM locations WHERE type = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int(stmt, 1, (int)type);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].district_id = sqlite3_column_int64(stmt, 1);
        strncpy(out[count].name, (const char*)sqlite3_column_text(stmt, 2), sizeof(out[count].name) - 1);
        out[count].type = (location_type)sqlite3_column_int(stmt, 3);
        out[count].is_public = sqlite3_column_int(stmt, 4) != 0;
        out[count].open_hour = sqlite3_column_int(stmt, 5);
        out[count].close_hour = sqlite3_column_int(stmt, 6);
        out[count].security = (security_level)sqlite3_column_int(stmt, 7);
        out[count].capacity = sqlite3_column_int(stmt, 8);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_count_locations(sim_context* ctx) {
    const char* sql = "SELECT COUNT(*) FROM locations;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    i32 count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

//------------------------------------------------------------------------------
// Actor Operations
//------------------------------------------------------------------------------

i64 db_insert_actor(sim_context* ctx, const actor* a) {
    const char* sql = "INSERT INTO actors (name, age, sex, occupation, income, home_id, work_id, impulsivity, morality, greed, aggression, loyalty, need_money, need_belonging, need_status, need_security) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_text(stmt, 1, a->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, a->age);
    char sex_str[2] = { a->sex, 0 };
    sqlite3_bind_text(stmt, 3, sex_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, a->occupation, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, a->income);
    sqlite3_bind_int64(stmt, 6, a->home_id);
    sqlite3_bind_int64(stmt, 7, a->work_id);
    sqlite3_bind_int(stmt, 8, a->impulsivity);
    sqlite3_bind_int(stmt, 9, a->morality);
    sqlite3_bind_int(stmt, 10, a->greed);
    sqlite3_bind_int(stmt, 11, a->aggression);
    sqlite3_bind_int(stmt, 12, a->loyalty);
    sqlite3_bind_int(stmt, 13, a->need_money);
    sqlite3_bind_int(stmt, 14, a->need_belonging);
    sqlite3_bind_int(stmt, 15, a->need_status);
    sqlite3_bind_int(stmt, 16, a->need_security);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;

    ctx->num_actors++;
    return sqlite3_last_insert_rowid(ctx->db);
}

bool db_update_actor_needs(sim_context* ctx, i64 id, i32 money, i32 belonging,
                           i32 status, i32 security) {
    const char* sql = "UPDATE actors SET need_money = ?, need_belonging = ?, need_status = ?, need_security = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, money);
    sqlite3_bind_int(stmt, 2, belonging);
    sqlite3_bind_int(stmt, 3, status);
    sqlite3_bind_int(stmt, 4, security);
    sqlite3_bind_int64(stmt, 5, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool db_get_actor(sim_context* ctx, i64 id, actor* out) {
    const char* sql = "SELECT id, name, age, sex, occupation, income, home_id, work_id, impulsivity, morality, greed, aggression, loyalty, need_money, need_belonging, need_status, need_security FROM actors WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id = sqlite3_column_int64(stmt, 0);
        strncpy(out->name, (const char*)sqlite3_column_text(stmt, 1), sizeof(out->name) - 1);
        out->age = sqlite3_column_int(stmt, 2);
        const char* sex_str = (const char*)sqlite3_column_text(stmt, 3);
        out->sex = sex_str ? sex_str[0] : 'M';
        const char* occ = (const char*)sqlite3_column_text(stmt, 4);
        if (occ) strncpy(out->occupation, occ, sizeof(out->occupation) - 1);
        out->income = sqlite3_column_int(stmt, 5);
        out->home_id = sqlite3_column_int64(stmt, 6);
        out->work_id = sqlite3_column_int64(stmt, 7);
        out->impulsivity = sqlite3_column_int(stmt, 8);
        out->morality = sqlite3_column_int(stmt, 9);
        out->greed = sqlite3_column_int(stmt, 10);
        out->aggression = sqlite3_column_int(stmt, 11);
        out->loyalty = sqlite3_column_int(stmt, 12);
        out->need_money = sqlite3_column_int(stmt, 13);
        out->need_belonging = sqlite3_column_int(stmt, 14);
        out->need_status = sqlite3_column_int(stmt, 15);
        out->need_security = sqlite3_column_int(stmt, 16);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

i32 db_get_all_actors(sim_context* ctx, actor* out, i32 max_count) {
    const char* sql = "SELECT id, name, age, sex, occupation, income, home_id, work_id, impulsivity, morality, greed, aggression, loyalty, need_money, need_belonging, need_status, need_security FROM actors;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        strncpy(out[count].name, (const char*)sqlite3_column_text(stmt, 1), sizeof(out[count].name) - 1);
        out[count].age = sqlite3_column_int(stmt, 2);
        const char* sex_str = (const char*)sqlite3_column_text(stmt, 3);
        out[count].sex = sex_str ? sex_str[0] : 'M';
        const char* occ = (const char*)sqlite3_column_text(stmt, 4);
        if (occ) strncpy(out[count].occupation, occ, sizeof(out[count].occupation) - 1);
        out[count].income = sqlite3_column_int(stmt, 5);
        out[count].home_id = sqlite3_column_int64(stmt, 6);
        out[count].work_id = sqlite3_column_int64(stmt, 7);
        out[count].impulsivity = sqlite3_column_int(stmt, 8);
        out[count].morality = sqlite3_column_int(stmt, 9);
        out[count].greed = sqlite3_column_int(stmt, 10);
        out[count].aggression = sqlite3_column_int(stmt, 11);
        out[count].loyalty = sqlite3_column_int(stmt, 12);
        out[count].need_money = sqlite3_column_int(stmt, 13);
        out[count].need_belonging = sqlite3_column_int(stmt, 14);
        out[count].need_status = sqlite3_column_int(stmt, 15);
        out[count].need_security = sqlite3_column_int(stmt, 16);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_get_actors_at_location(sim_context* ctx, i64 location_id, i32 timestamp,
                               i64* out_ids, i32 max_count) {
    // Get the most recent state for each actor at or before this timestamp
    const char* sql = R"(
        SELECT DISTINCT actor_id FROM actor_states
        WHERE location_id = ? AND timestamp <= ?
        AND timestamp = (
            SELECT MAX(timestamp) FROM actor_states AS sub
            WHERE sub.actor_id = actor_states.actor_id AND sub.timestamp <= ?
        );
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, location_id);
    sqlite3_bind_int(stmt, 2, timestamp);
    sqlite3_bind_int(stmt, 3, timestamp);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out_ids[count++] = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

//------------------------------------------------------------------------------
// Relationship Operations
//------------------------------------------------------------------------------

i64 db_insert_relationship(sim_context* ctx, i64 actor1, i64 actor2,
                           relationship_type type, relationship_subtype subtype,
                           i32 strength, i32 sentiment) {
    const char* sql = "INSERT INTO relationships (actor1_id, actor2_id, type, subtype, strength, sentiment) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_int64(stmt, 1, actor1);
    sqlite3_bind_int64(stmt, 2, actor2);
    sqlite3_bind_int(stmt, 3, (int)type);
    sqlite3_bind_int(stmt, 4, (int)subtype);
    sqlite3_bind_int(stmt, 5, strength);
    sqlite3_bind_int(stmt, 6, sentiment);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;
    return sqlite3_last_insert_rowid(ctx->db);
}

bool db_update_relationship_sentiment(sim_context* ctx, i64 id, i32 new_sentiment) {
    const char* sql = "UPDATE relationships SET sentiment = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, new_sentiment);
    sqlite3_bind_int64(stmt, 2, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

i32 db_get_relationships_for_actor(sim_context* ctx, i64 actor_id,
                                    relationship* out, i32 max_count) {
    const char* sql = "SELECT id, actor1_id, actor2_id, type, subtype, strength, sentiment FROM relationships WHERE actor1_id = ? OR actor2_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, actor_id);
    sqlite3_bind_int64(stmt, 2, actor_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].actor1_id = sqlite3_column_int64(stmt, 1);
        out[count].actor2_id = sqlite3_column_int64(stmt, 2);
        out[count].type = (relationship_type)sqlite3_column_int(stmt, 3);
        out[count].subtype = (relationship_subtype)sqlite3_column_int(stmt, 4);
        out[count].strength = sqlite3_column_int(stmt, 5);
        out[count].sentiment = sqlite3_column_int(stmt, 6);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

bool db_get_relationship_between(sim_context* ctx, i64 actor1, i64 actor2,
                                  relationship* out) {
    const char* sql = "SELECT id, actor1_id, actor2_id, type, subtype, strength, sentiment FROM relationships WHERE (actor1_id = ? AND actor2_id = ?) OR (actor1_id = ? AND actor2_id = ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, actor1);
    sqlite3_bind_int64(stmt, 2, actor2);
    sqlite3_bind_int64(stmt, 3, actor2);
    sqlite3_bind_int64(stmt, 4, actor1);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id = sqlite3_column_int64(stmt, 0);
        out->actor1_id = sqlite3_column_int64(stmt, 1);
        out->actor2_id = sqlite3_column_int64(stmt, 2);
        out->type = (relationship_type)sqlite3_column_int(stmt, 3);
        out->subtype = (relationship_subtype)sqlite3_column_int(stmt, 4);
        out->strength = sqlite3_column_int(stmt, 5);
        out->sentiment = sqlite3_column_int(stmt, 6);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

//------------------------------------------------------------------------------
// Event Operations
//------------------------------------------------------------------------------

i64 db_insert_event(sim_context* ctx, i32 timestamp, event_type type,
                    i64 actor_id, i64 target_id, i64 location_id,
                    const char* details) {
    const char* sql = "INSERT INTO events (timestamp, type, actor_id, target_id, location_id, details) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_int(stmt, 1, timestamp);
    sqlite3_bind_int(stmt, 2, (int)type);
    sqlite3_bind_int64(stmt, 3, actor_id);
    if (target_id > 0) sqlite3_bind_int64(stmt, 4, target_id);
    else sqlite3_bind_null(stmt, 4);
    if (location_id > 0) sqlite3_bind_int64(stmt, 5, location_id);
    else sqlite3_bind_null(stmt, 5);
    if (details) sqlite3_bind_text(stmt, 6, details, -1, SQLITE_TRANSIENT);
    else sqlite3_bind_null(stmt, 6);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;
    return sqlite3_last_insert_rowid(ctx->db);
}

i32 db_get_events_for_actor(sim_context* ctx, i64 actor_id,
                             event* out, i32 max_count) {
    const char* sql = "SELECT id, timestamp, type, actor_id, target_id, location_id, details FROM events WHERE actor_id = ? OR target_id = ? ORDER BY timestamp;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, actor_id);
    sqlite3_bind_int64(stmt, 2, actor_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].timestamp = sqlite3_column_int(stmt, 1);
        out[count].type = (event_type)sqlite3_column_int(stmt, 2);
        out[count].actor_id = sqlite3_column_int64(stmt, 3);
        out[count].target_id = sqlite3_column_type(stmt, 4) != SQLITE_NULL ? sqlite3_column_int64(stmt, 4) : 0;
        out[count].location_id = sqlite3_column_type(stmt, 5) != SQLITE_NULL ? sqlite3_column_int64(stmt, 5) : 0;
        const char* det = (const char*)sqlite3_column_text(stmt, 6);
        if (det) strncpy(out[count].details, det, sizeof(out[count].details) - 1);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_get_events_at_location(sim_context* ctx, i64 location_id, i32 time_start,
                               i32 time_end, event* out, i32 max_count) {
    const char* sql = "SELECT id, timestamp, type, actor_id, target_id, location_id, details FROM events WHERE location_id = ? AND timestamp >= ? AND timestamp <= ? ORDER BY timestamp;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, location_id);
    sqlite3_bind_int(stmt, 2, time_start);
    sqlite3_bind_int(stmt, 3, time_end);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].timestamp = sqlite3_column_int(stmt, 1);
        out[count].type = (event_type)sqlite3_column_int(stmt, 2);
        out[count].actor_id = sqlite3_column_int64(stmt, 3);
        out[count].target_id = sqlite3_column_type(stmt, 4) != SQLITE_NULL ? sqlite3_column_int64(stmt, 4) : 0;
        out[count].location_id = sqlite3_column_type(stmt, 5) != SQLITE_NULL ? sqlite3_column_int64(stmt, 5) : 0;
        const char* det = (const char*)sqlite3_column_text(stmt, 6);
        if (det) strncpy(out[count].details, det, sizeof(out[count].details) - 1);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_get_events_in_range(sim_context* ctx, i32 time_start, i32 time_end,
                            event* out, i32 max_count) {
    const char* sql = "SELECT id, timestamp, type, actor_id, target_id, location_id, details FROM events WHERE timestamp >= ? AND timestamp <= ? ORDER BY timestamp;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int(stmt, 1, time_start);
    sqlite3_bind_int(stmt, 2, time_end);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].timestamp = sqlite3_column_int(stmt, 1);
        out[count].type = (event_type)sqlite3_column_int(stmt, 2);
        out[count].actor_id = sqlite3_column_int64(stmt, 3);
        out[count].target_id = sqlite3_column_type(stmt, 4) != SQLITE_NULL ? sqlite3_column_int64(stmt, 4) : 0;
        out[count].location_id = sqlite3_column_type(stmt, 5) != SQLITE_NULL ? sqlite3_column_int64(stmt, 5) : 0;
        const char* det = (const char*)sqlite3_column_text(stmt, 6);
        if (det) strncpy(out[count].details, det, sizeof(out[count].details) - 1);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

bool db_get_event(sim_context* ctx, i64 id, event* out) {
    const char* sql = "SELECT id, timestamp, type, actor_id, target_id, location_id, details FROM events WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id = sqlite3_column_int64(stmt, 0);
        out->timestamp = sqlite3_column_int(stmt, 1);
        out->type = (event_type)sqlite3_column_int(stmt, 2);
        out->actor_id = sqlite3_column_int64(stmt, 3);
        out->target_id = sqlite3_column_type(stmt, 4) != SQLITE_NULL ? sqlite3_column_int64(stmt, 4) : 0;
        out->location_id = sqlite3_column_type(stmt, 5) != SQLITE_NULL ? sqlite3_column_int64(stmt, 5) : 0;
        const char* det = (const char*)sqlite3_column_text(stmt, 6);
        if (det) strncpy(out->details, det, sizeof(out->details) - 1);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

//------------------------------------------------------------------------------
// Grievance Operations
//------------------------------------------------------------------------------

i64 db_insert_grievance(sim_context* ctx, i64 victim_id, i64 offender_id,
                        i64 cause_event_id, i32 severity, i32 timestamp) {
    const char* sql = "INSERT INTO grievances (victim_id, offender_id, cause_event_id, severity, timestamp) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_int64(stmt, 1, victim_id);
    sqlite3_bind_int64(stmt, 2, offender_id);
    sqlite3_bind_int64(stmt, 3, cause_event_id);
    sqlite3_bind_int(stmt, 4, severity);
    sqlite3_bind_int(stmt, 5, timestamp);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;
    return sqlite3_last_insert_rowid(ctx->db);
}

bool db_resolve_grievance(sim_context* ctx, i64 id) {
    const char* sql = "UPDATE grievances SET resolved = 1 WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

i32 db_get_grievances_for_actor(sim_context* ctx, i64 victim_id,
                                 grievance* out, i32 max_count) {
    const char* sql = "SELECT id, victim_id, offender_id, cause_event_id, severity, timestamp, resolved FROM grievances WHERE victim_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, victim_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].victim_id = sqlite3_column_int64(stmt, 1);
        out[count].offender_id = sqlite3_column_int64(stmt, 2);
        out[count].cause_event_id = sqlite3_column_int64(stmt, 3);
        out[count].severity = sqlite3_column_int(stmt, 4);
        out[count].timestamp = sqlite3_column_int(stmt, 5);
        out[count].resolved = sqlite3_column_int(stmt, 6) != 0;
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_get_active_grievances(sim_context* ctx, i64 victim_id,
                              grievance* out, i32 max_count) {
    const char* sql = "SELECT id, victim_id, offender_id, cause_event_id, severity, timestamp, resolved FROM grievances WHERE victim_id = ? AND resolved = 0;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, victim_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].victim_id = sqlite3_column_int64(stmt, 1);
        out[count].offender_id = sqlite3_column_int64(stmt, 2);
        out[count].cause_event_id = sqlite3_column_int64(stmt, 3);
        out[count].severity = sqlite3_column_int(stmt, 4);
        out[count].timestamp = sqlite3_column_int(stmt, 5);
        out[count].resolved = sqlite3_column_int(stmt, 6) != 0;
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_get_total_grievance_severity(sim_context* ctx, i64 victim_id, i64 offender_id) {
    const char* sql = "SELECT SUM(severity) FROM grievances WHERE victim_id = ? AND offender_id = ? AND resolved = 0;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, victim_id);
    sqlite3_bind_int64(stmt, 2, offender_id);

    i32 total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return total;
}

//------------------------------------------------------------------------------
// Crime Operations
//------------------------------------------------------------------------------

i64 db_insert_crime(sim_context* ctx, crime_type type, i64 perpetrator_id,
                    i64 victim_id, i64 location_id, i32 timestamp,
                    motive_type motive) {
    const char* sql = "INSERT INTO crimes (type, perpetrator_id, victim_id, location_id, timestamp, motive) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_int(stmt, 1, (int)type);
    sqlite3_bind_int64(stmt, 2, perpetrator_id);
    sqlite3_bind_int64(stmt, 3, victim_id);
    sqlite3_bind_int64(stmt, 4, location_id);
    sqlite3_bind_int(stmt, 5, timestamp);
    sqlite3_bind_int(stmt, 6, (int)motive);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;

    ctx->num_crimes++;
    return sqlite3_last_insert_rowid(ctx->db);
}

bool db_update_crime_score(sim_context* ctx, i64 id, f32 score) {
    const char* sql = "UPDATE crimes SET interest_score = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_double(stmt, 1, score);
    sqlite3_bind_int64(stmt, 2, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

i32 db_get_all_crimes(sim_context* ctx, crime* out, i32 max_count) {
    const char* sql = "SELECT id, type, perpetrator_id, victim_id, location_id, timestamp, motive, interest_score FROM crimes ORDER BY interest_score DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].type = (crime_type)sqlite3_column_int(stmt, 1);
        out[count].perpetrator_id = sqlite3_column_int64(stmt, 2);
        out[count].victim_id = sqlite3_column_int64(stmt, 3);
        out[count].location_id = sqlite3_column_int64(stmt, 4);
        out[count].timestamp = sqlite3_column_int(stmt, 5);
        out[count].motive = (motive_type)sqlite3_column_int(stmt, 6);
        out[count].interest_score = (f32)sqlite3_column_double(stmt, 7);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

bool db_get_crime(sim_context* ctx, i64 id, crime* out) {
    const char* sql = "SELECT id, type, perpetrator_id, victim_id, location_id, timestamp, motive, interest_score FROM crimes WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id = sqlite3_column_int64(stmt, 0);
        out->type = (crime_type)sqlite3_column_int(stmt, 1);
        out->perpetrator_id = sqlite3_column_int64(stmt, 2);
        out->victim_id = sqlite3_column_int64(stmt, 3);
        out->location_id = sqlite3_column_int64(stmt, 4);
        out->timestamp = sqlite3_column_int(stmt, 5);
        out->motive = (motive_type)sqlite3_column_int(stmt, 6);
        out->interest_score = (f32)sqlite3_column_double(stmt, 7);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

//------------------------------------------------------------------------------
// Evidence Operations
//------------------------------------------------------------------------------

i64 db_insert_evidence(sim_context* ctx, i64 crime_id, evidence_type type,
                       i64 location_id, i64 points_to_id, i32 clarity,
                       const char* description) {
    const char* sql = "INSERT INTO evidence (crime_id, type, location_id, points_to_id, clarity, description) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_int64(stmt, 1, crime_id);
    sqlite3_bind_int(stmt, 2, (int)type);
    if (location_id > 0) sqlite3_bind_int64(stmt, 3, location_id);
    else sqlite3_bind_null(stmt, 3);
    if (points_to_id > 0) sqlite3_bind_int64(stmt, 4, points_to_id);
    else sqlite3_bind_null(stmt, 4);
    sqlite3_bind_int(stmt, 5, clarity);
    if (description) sqlite3_bind_text(stmt, 6, description, -1, SQLITE_TRANSIENT);
    else sqlite3_bind_null(stmt, 6);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;
    return sqlite3_last_insert_rowid(ctx->db);
}

i32 db_get_evidence_for_crime(sim_context* ctx, i64 crime_id,
                               evidence* out, i32 max_count) {
    const char* sql = "SELECT id, crime_id, type, location_id, points_to_id, clarity, description FROM evidence WHERE crime_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, crime_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].crime_id = sqlite3_column_int64(stmt, 1);
        out[count].type = (evidence_type)sqlite3_column_int(stmt, 2);
        out[count].location_id = sqlite3_column_type(stmt, 3) != SQLITE_NULL ? sqlite3_column_int64(stmt, 3) : 0;
        out[count].points_to_id = sqlite3_column_type(stmt, 4) != SQLITE_NULL ? sqlite3_column_int64(stmt, 4) : 0;
        out[count].clarity = sqlite3_column_int(stmt, 5);
        const char* desc = (const char*)sqlite3_column_text(stmt, 6);
        if (desc) strncpy(out[count].description, desc, sizeof(out[count].description) - 1);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

//------------------------------------------------------------------------------
// Witness Operations
//------------------------------------------------------------------------------

i64 db_insert_witness(sim_context* ctx, i64 crime_id, i64 actor_id,
                      const char* saw_what, i32 reliability, i32 willingness) {
    const char* sql = "INSERT INTO witnesses (crime_id, actor_id, saw_what, reliability, willingness) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQL_PREPARE(ctx->db, sql, stmt);

    sqlite3_bind_int64(stmt, 1, crime_id);
    sqlite3_bind_int64(stmt, 2, actor_id);
    if (saw_what) sqlite3_bind_text(stmt, 3, saw_what, -1, SQLITE_TRANSIENT);
    else sqlite3_bind_null(stmt, 3);
    sqlite3_bind_int(stmt, 4, reliability);
    sqlite3_bind_int(stmt, 5, willingness);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;
    return sqlite3_last_insert_rowid(ctx->db);
}

i32 db_get_witnesses_for_crime(sim_context* ctx, i64 crime_id,
                                witness* out, i32 max_count) {
    const char* sql = "SELECT id, crime_id, actor_id, saw_what, reliability, willingness FROM witnesses WHERE crime_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, crime_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out[count].id = sqlite3_column_int64(stmt, 0);
        out[count].crime_id = sqlite3_column_int64(stmt, 1);
        out[count].actor_id = sqlite3_column_int64(stmt, 2);
        const char* saw = (const char*)sqlite3_column_text(stmt, 3);
        if (saw) strncpy(out[count].saw_what, saw, sizeof(out[count].saw_what) - 1);
        out[count].reliability = sqlite3_column_int(stmt, 4);
        out[count].willingness = sqlite3_column_int(stmt, 5);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

//------------------------------------------------------------------------------
// Actor State Tracking
//------------------------------------------------------------------------------

bool db_update_actor_location(sim_context* ctx, i64 actor_id, i64 location_id,
                               i32 timestamp, const char* activity) {
    const char* sql = "INSERT INTO actor_states (timestamp, actor_id, location_id, activity) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, timestamp);
    sqlite3_bind_int64(stmt, 2, actor_id);
    sqlite3_bind_int64(stmt, 3, location_id);
    if (activity) sqlite3_bind_text(stmt, 4, activity, -1, SQLITE_TRANSIENT);
    else sqlite3_bind_null(stmt, 4);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

i64 db_get_actor_location_at(sim_context* ctx, i64 actor_id, i32 timestamp) {
    const char* sql = "SELECT location_id FROM actor_states WHERE actor_id = ? AND timestamp <= ? ORDER BY timestamp DESC LIMIT 1;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, actor_id);
    sqlite3_bind_int(stmt, 2, timestamp);

    i64 loc_id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        loc_id = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return loc_id;
}

//------------------------------------------------------------------------------
// Query Helpers
//------------------------------------------------------------------------------

i32 db_get_actors_with_grudge_against(sim_context* ctx, i64 target_id,
                                       i64* out_ids, i32 max_count) {
    const char* sql = "SELECT DISTINCT victim_id FROM grievances WHERE offender_id = ? AND resolved = 0;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, target_id);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out_ids[count++] = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_get_wealthy_actors(sim_context* ctx, i32 min_income,
                           i64* out_ids, i32 max_count) {
    const char* sql = "SELECT id FROM actors WHERE income >= ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int(stmt, 1, min_income);

    i32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
        out_ids[count++] = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

i32 db_count_events_of_type(sim_context* ctx, i64 actor_id, event_type type) {
    const char* sql = "SELECT COUNT(*) FROM events WHERE actor_id = ? AND type = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int64(stmt, 1, actor_id);
    sqlite3_bind_int(stmt, 2, (int)type);

    i32 count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

bool db_has_recent_crime(sim_context* ctx, i64 actor_id, i32 current_timestamp, i32 cooldown_hours) {
    const char* sql = "SELECT COUNT(*) FROM crimes WHERE perpetrator_id = ? AND timestamp > ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    i32 cutoff = current_timestamp - cooldown_hours;
    sqlite3_bind_int64(stmt, 1, actor_id);
    sqlite3_bind_int(stmt, 2, cutoff);

    bool has_recent = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        has_recent = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return has_recent;
}

bool db_resolve_grievances_against(sim_context* ctx, i64 victim_id, i64 offender_id) {
    const char* sql = "UPDATE grievances SET resolved = 1 WHERE victim_id = ? AND offender_id = ? AND resolved = 0;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, victim_id);
    sqlite3_bind_int64(stmt, 2, offender_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}
