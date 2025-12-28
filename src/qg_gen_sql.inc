static const char *sql_init =
"DROP TABLE IF EXISTS districts;\n"
"DROP TABLE IF EXISTS landmarks;\n"
"DROP TABLE IF EXISTS transits;\n"
"CREATE TABLE districts(\n"
"    district_id INTEGER PRIMARY KEY,\n"
"    name TEXT NOT NULL,\n"
"    type INTEGER NOT NULL,\n"
"    wealth INTEGER NOT NULL,\n"
"    rough INTEGER NOT NULL,\n"
"    response_time INTEGER NOT NULL\n"
");\n"
"CREATE TABLE landmarks(\n"
"    landmark_id INTEGER PRIMARY KEY,\n"
"    district_id INTEGER NOT NULL REFERENCES districts(district_id),\n"
"    name TEXT NOT NULL,\n"
"    type INTEGER NOT NULL,\n"
"    size INTEGER NOT NULL,\n"
"    opening_hour INTEGER,\n"
"    closing_hour INTEGER,\n"
"    peak_hour INTEGER,\n"
"    num_staff INTEGER NOT NULL,\n"
"    is_public INTEGER NOT NULL, -- Boolean; 0: false, 1: true\n"
"    crime_factor INTEGER NOT NULL -- [0-100]\n"
");\n"
"CREATE TABLE transits (\n"
"    from_landmark_id INTEGER NOT NULL REFERENCES landmarks(landmark_id),\n"
"    to_landmark_id   INTEGER NOT NULL REFERENCES landmarks(landmark_id),\n"
"    mode             INTEGER NOT NULL,\n"
"    day_phase        INTEGER NOT NULL,\n"
"    minutes          INTEGER NOT NULL,\n"
"    PRIMARY KEY (from_landmark_id, to_landmark_id, mode, day_phase)\n"
") WITHOUT ROWID;\n"
"CREATE INDEX idx_landmark_district   ON landmarks(district_id);\n"
"CREATE INDEX idx_landmark_type       ON landmarks(type);\n"
"CREATE INDEX idx_transit_from        ON transits(from_landmark_id);\n"
"CREATE INDEX idx_transit_to          ON transits(to_landmark_id);\n";

static const char *sql_district = "INSERT INTO districts VALUES (:id, :name, :type, :wealth, :rough, :response_time)";
static const char *sql_landmark = "INSERT INTO landmarks VALUES (:landmark_id, :district_id, :name, :type, :size, :open_hour, :close_hour, :peak_hour, :num_staff, :is_public, :crime_factor)";
static const char *sql_transit = "INSERT INTO transits VALUES (:from_landmark_id, :to_landmark_id, :mode, :day_phase, :minutes)";
static const char *sql_cull_transit = "INSERT INTO transits VALUES (:from_landmark_id, :to_landmark_id, :mode, :day_phase, :minutes)";
static const char *sql_prune_transit = "DELETE FROM transits WHERE (from_landmark_id * 37 + to_landmark_id * 41 + mode * 43 + day_phase * 47) % 10 = 0";
