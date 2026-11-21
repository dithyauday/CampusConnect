-- CampusConnect — PostgreSQL schema
-- Run once: psql -U postgres -d alumnidb -f schema.sql

CREATE TABLE IF NOT EXISTS alumni (
    id               SERIAL PRIMARY KEY,
    uid              TEXT NOT NULL UNIQUE,        -- external_id or JSON id field
    name             TEXT NOT NULL,
    department       TEXT DEFAULT '',             -- branch.name from JSON
    course           TEXT DEFAULT '',             -- e.g. BE, MCA
    graduation_year  INT  DEFAULT 0,              -- to_year
    from_year        INT  DEFAULT 0,              -- from_year
    batch            TEXT DEFAULT '',             -- e.g. "2023"
    current_company  TEXT DEFAULT '',             -- currently_at.name
    "current_role"   TEXT DEFAULT '',             -- Escaped with quotes so Postgres allows it as a column name
    city             TEXT DEFAULT '',             -- current_city.name
    linkedin_url     TEXT DEFAULT '',
    photo_url        TEXT DEFAULT '',
    created_at       TIMESTAMPTZ DEFAULT NOW()
);

-- Indexes for the common filter/search params
CREATE INDEX IF NOT EXISTS idx_alumni_batch      ON alumni(batch);
CREATE INDEX IF NOT EXISTS idx_alumni_dept       ON alumni(LOWER(department));
CREATE INDEX IF NOT EXISTS idx_alumni_company    ON alumni(LOWER(current_company));
CREATE INDEX IF NOT EXISTS idx_alumni_grad_year  ON alumni(graduation_year);