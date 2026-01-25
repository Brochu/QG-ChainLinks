#pragma once
#include "world_setup.h"
#include <sqlite3.h>

// Initialize database tables
void db_init_tables(sqlite3 *db);

// Insert functions
void db_insert_district(sqlite3 *db, const district *d);
void db_insert_location(sqlite3 *db, const location *loc);
void db_insert_person(sqlite3 *db, const person *p);
void db_insert_relationship(sqlite3 *db, const relationship *r);
void db_insert_object(sqlite3 *db, const sim_object *obj);

// Update functions
void db_update_location_owner(sqlite3 *db, i64 location_id, i64 owner_id);

// Add person to location residents
void db_add_resident(sqlite3 *db, i64 location_id, i64 person_id);

// Save in-memory DB to file for debugging
void db_save_to_file(sqlite3 *db, const char *filename);
