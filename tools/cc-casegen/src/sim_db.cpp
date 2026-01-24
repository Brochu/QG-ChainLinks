#include "sim_db.h"
#include <cassert>
#include <cstdio>
#include <cstring>

// ============================================================================
// SQL STATEMENTS
// ============================================================================

static const char *sql_create_tables = R"(
CREATE TABLE IF NOT EXISTS districts (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    type INTEGER NOT NULL,
    wealth INTEGER,
    roughness INTEGER,
    response_time INTEGER
);

CREATE TABLE IF NOT EXISTS locations (
    id INTEGER PRIMARY KEY,
    district_id INTEGER,
    name TEXT NOT NULL,
    address TEXT,
    type INTEGER NOT NULL,
    owner_id INTEGER,
    access_control INTEGER,
    operation_hours INTEGER,
    crime_factor INTEGER,
    FOREIGN KEY (district_id) REFERENCES districts(id)
);

CREATE TABLE IF NOT EXISTS location_residents (
    location_id INTEGER,
    person_id INTEGER,
    PRIMARY KEY (location_id, person_id)
);

CREATE TABLE IF NOT EXISTS people (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    alias TEXT,
    sex TEXT NOT NULL,
    age INTEGER,
    height INTEGER,
    build INTEGER,
    hair_color INTEGER,
    home_location INTEGER,
    work_location INTEGER,
    occupation TEXT,
    income INTEGER,
    phone_number TEXT,
    org_affiliation TEXT,
    impulsivity INTEGER,
    morality INTEGER,
    greed INTEGER,
    aggression INTEGER,
    loyalty INTEGER,
    need_money INTEGER,
    need_belonging INTEGER,
    need_status INTEGER,
    need_security INTEGER,
    archetype INTEGER,
    FOREIGN KEY (home_location) REFERENCES locations(id),
    FOREIGN KEY (work_location) REFERENCES locations(id)
);

CREATE TABLE IF NOT EXISTS relationships (
    person1_id INTEGER,
    person2_id INTEGER,
    type INTEGER NOT NULL,
    strength INTEGER DEFAULT 50,
    PRIMARY KEY (person1_id, person2_id),
    FOREIGN KEY (person1_id) REFERENCES people(id),
    FOREIGN KEY (person2_id) REFERENCES people(id)
);

CREATE TABLE IF NOT EXISTS objects (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    type INTEGER NOT NULL,
    serial_number TEXT,
    size INTEGER,
    color INTEGER,
    material INTEGER,
    condition INTEGER,
    owner_id INTEGER,
    location_id INTEGER,
    FOREIGN KEY (owner_id) REFERENCES people(id),
    FOREIGN KEY (location_id) REFERENCES locations(id)
);
)";

static const char *sql_insert_district =
    "INSERT INTO districts (id, name, type, wealth, roughness, response_time) "
    "VALUES (?, ?, ?, ?, ?, ?);";

static const char *sql_insert_location =
    "INSERT INTO locations (id, district_id, name, address, type, owner_id, access_control, operation_hours, crime_factor) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

static const char *sql_insert_person =
    "INSERT INTO people (id, name, alias, sex, age, height, build, hair_color, "
    "home_location, work_location, occupation, income, phone_number, org_affiliation, "
    "impulsivity, morality, greed, aggression, loyalty, "
    "need_money, need_belonging, need_status, need_security, archetype) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

static const char *sql_insert_relationship =
    "INSERT INTO relationships (person1_id, person2_id, type, strength) VALUES (?, ?, ?, ?);";

static const char *sql_insert_object =
    "INSERT INTO objects (id, name, type, serial_number, size, color, material, condition, owner_id, location_id) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

static const char *sql_add_resident =
    "INSERT OR IGNORE INTO location_residents (location_id, person_id) VALUES (?, ?);";

// ============================================================================
// IMPLEMENTATION
// ============================================================================

static int dummy_callback(void *data, int argc, char **argv, char **col) {
    return 0;
}

void db_init_tables(sqlite3 *db) {
    char *err = nullptr;
    int res = sqlite3_exec(db, sql_create_tables, dummy_callback, nullptr, &err);
    if (res != SQLITE_OK) {
        printf("[DB] Error creating tables: %s\n", err);
        sqlite3_free(err);
    }
}

void db_insert_district(sqlite3 *db, const district *d) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql_insert_district, -1, &stmt, nullptr);

    sqlite3_bind_int64(stmt, 1, d->id);
    sqlite3_bind_text(stmt, 2, d->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, d->type);
    sqlite3_bind_int(stmt, 4, d->wealth);
    sqlite3_bind_int(stmt, 5, d->roughness);
    sqlite3_bind_int(stmt, 6, d->response_time);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_insert_location(sqlite3 *db, const location *loc) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql_insert_location, -1, &stmt, nullptr);

    sqlite3_bind_int64(stmt, 1, loc->id);
    sqlite3_bind_int64(stmt, 2, loc->district_id);
    sqlite3_bind_text(stmt, 3, loc->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, loc->address ? loc->address : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, loc->type);
    sqlite3_bind_int64(stmt, 6, loc->owner_id);
    sqlite3_bind_int(stmt, 7, loc->access);
    sqlite3_bind_int(stmt, 8, loc->hours);
    sqlite3_bind_int(stmt, 9, loc->crime_factor);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_insert_person(sqlite3 *db, const person *p) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql_insert_person, -1, &stmt, nullptr);

    char sex_str[2] = { p->sex, '\0' };

    sqlite3_bind_int64(stmt, 1, p->id);
    sqlite3_bind_text(stmt, 2, p->name, -1, SQLITE_TRANSIENT);
    if (p->alias) {
        sqlite3_bind_text(stmt, 3, p->alias, -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 3);
    }
    sqlite3_bind_text(stmt, 4, sex_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, p->age);
    sqlite3_bind_int(stmt, 6, p->height);
    sqlite3_bind_int(stmt, 7, p->build);
    sqlite3_bind_int(stmt, 8, p->hair);
    sqlite3_bind_int64(stmt, 9, p->home_location_id);
    sqlite3_bind_int64(stmt, 10, p->work_location_id);
    sqlite3_bind_text(stmt, 11, p->occupation ? p->occupation : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 12, p->income);
    sqlite3_bind_text(stmt, 13, p->phone_number ? p->phone_number : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 14, p->org_affiliation ? p->org_affiliation : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 15, p->impulsivity);
    sqlite3_bind_int(stmt, 16, p->morality);
    sqlite3_bind_int(stmt, 17, p->greed);
    sqlite3_bind_int(stmt, 18, p->aggression);
    sqlite3_bind_int(stmt, 19, p->loyalty);
    sqlite3_bind_int(stmt, 20, p->need_money);
    sqlite3_bind_int(stmt, 21, p->need_belonging);
    sqlite3_bind_int(stmt, 22, p->need_status);
    sqlite3_bind_int(stmt, 23, p->need_security);
    sqlite3_bind_int(stmt, 24, p->archetype);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_insert_relationship(sqlite3 *db, const relationship *r) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql_insert_relationship, -1, &stmt, nullptr);

    sqlite3_bind_int64(stmt, 1, r->person1_id);
    sqlite3_bind_int64(stmt, 2, r->person2_id);
    sqlite3_bind_int(stmt, 3, r->type);
    sqlite3_bind_int(stmt, 4, r->strength);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_insert_object(sqlite3 *db, const sim_object *obj) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql_insert_object, -1, &stmt, nullptr);

    sqlite3_bind_int64(stmt, 1, obj->id);
    sqlite3_bind_text(stmt, 2, obj->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, obj->type);
    sqlite3_bind_text(stmt, 4, obj->serial_number ? obj->serial_number : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, obj->size);
    sqlite3_bind_int(stmt, 6, obj->color);
    sqlite3_bind_int(stmt, 7, obj->material);
    sqlite3_bind_int(stmt, 8, obj->condition);
    sqlite3_bind_int64(stmt, 9, obj->owner_id);
    sqlite3_bind_int64(stmt, 10, obj->location_id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_add_resident(sqlite3 *db, i64 location_id, i64 person_id) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql_add_resident, -1, &stmt, nullptr);

    sqlite3_bind_int64(stmt, 1, location_id);
    sqlite3_bind_int64(stmt, 2, person_id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_save_to_file(sqlite3 *db, const char *filename) {
    sqlite3 *file_db;
    int res = sqlite3_open(filename, &file_db);
    if (res != SQLITE_OK) {
        printf("[DB] Could not open file %s for backup\n", filename);
        return;
    }

    sqlite3_backup *backup = sqlite3_backup_init(file_db, "main", db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }

    sqlite3_close(file_db);
    printf("[DB] Saved database to %s\n", filename);
}
