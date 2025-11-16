CREATE TABLE districts(
    district_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    type INTEGER NOT NULL,
    wealth INTEGER NOT NULL,
    rough INTEGER NOT NULL,
    response_time INTEGER NOT NULL
);

CREATE TABLE landmarks(
    landmark_id INTEGER PRIMARY KEY,
    district_id INTEGER NOT NULL REFERENCES districts(district_id),
    name TEXT NOT NULL,
    type INTEGER NOT NULL,
    size INTEGER NOT NULL,
    opening_hour INTEGER,
    closing_hour INTEGER,
    peak_hour INTEGER,
    num_staff INTEGER NOT NULL,
    is_public INTEGER NOT NULL, -- Boolean; 0: false, 1: true
    crime_factor INTEGER NOT NULL -- [0-100]
);

CREATE TABLE transit (
    from_landmark_id INTEGER NOT NULL REFERENCES landmark(landmark_id),
    to_landmark_id   INTEGER NOT NULL REFERENCES landmark(landmark_id),
    mode             INTEGER NOT NULL,
    day_phase        INTEGER NOT NULL,
    minutes          INTEGER NOT NULL,
    PRIMARY KEY (from_landmark_id, to_landmark_id, mode, day_phase)
) WITHOUT ROWID;

CREATE INDEX idx_landmark_district   ON landmark(district_id);
CREATE INDEX idx_landmark_type       ON landmark(type);
CREATE INDEX idx_transit_from        ON transit(from_landmark_id);
CREATE INDEX idx_transit_to          ON transit(to_landmark_id);
