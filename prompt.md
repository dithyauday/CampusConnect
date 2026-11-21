# CampusConnect — Project Prompt

## What this is
Alumni Information System for a college. Ingests dirty JSON dumps from an
institutional alumni portal, seeds them into PostgreSQL, and serves the data
via a C++ REST API to a Next.js dashboard where users can search, filter, and
manually add alumni records.

## Stack
| Layer      | Technology                                      |
|------------|-------------------------------------------------|
| Database   | PostgreSQL (`alumnidb`, user `postgres`, pw `password`, port 5432) |
| Backend    | C++ 17, Crow framework (`crow_all.h`), libpqxx  |
| Frontend   | Next.js 15, React, Tailwind CSS                 |
| Build      | CMake 3.15+                                     |

## Folder layout
```
CampusConnect/
├── backend/
│   ├── main.cpp          ← Crow API server + seeder
│   ├── crow_all.h        ← single-header Crow (download separately)
│   ├── CMakeLists.txt
│   ├── schema.sql        ← run once before first build
│   └── build/            ← cmake build dir
├── frontend/             ← Next.js 15 app
│   └── ...
├── 2023_data.json        ← raw JSON dump (array of alumni objects)
└── 2025_data.json
```

## DB schema (table: alumni)
```
uid TEXT UNIQUE         ← external_id or id from JSON
name TEXT
department TEXT         ← branch.name
course TEXT             ← course.name  (e.g. BE, MCA)
graduation_year INT     ← to_year
from_year INT
batch TEXT              ← e.g. "2023"
current_company TEXT    ← currently_at.name
current_role TEXT       ← profile_title
city TEXT               ← current_city.name
linkedin_url TEXT
photo_url TEXT          ← image.url (CDN URL)
created_at TIMESTAMPTZ
```

## API endpoints (base: http://localhost:8080)
| Method | Path                | Purpose                              |
|--------|---------------------|--------------------------------------|
| GET    | /api/alumni         | Fetch alumni, supports query params  |
| POST   | /api/alumni/add     | Insert or update one record          |
| OPTIONS| both routes         | CORS preflight                       |

### GET /api/alumni query params
- `search=<text>` — fuzzy match on name or department
- `batch=2023`
- `department=CSE`
- `company=Google`

### POST /api/alumni/add body (JSON)
```json
{
  "uid": "required-unique-id",
  "name": "Jane Doe",
  "department": "Computer Science",
  "course": "BE",
  "graduation_year": 2024,
  "from_year": 2020,
  "batch": "2024",
  "current_company": "Google",
  "current_role": "SDE",
  "city": "Bangalore",
  "linkedin_url": "https://linkedin.com/in/janedoe",
  "photo_url": "https://cdn.example.com/photo.jpg"
}
```
Upserts on conflict — safe to call for edits too.

## Input JSON shape (from institutional dump)
Key fields Claude maps from raw JSON → DB column:
- `id` / `external_id`  → `uid`
- `name`                → `name`
- `branch.name`         → `department`
- `course.name`         → `course`
- `to_year`             → `graduation_year`
- `from_year`           → `from_year`
- `batch`               → `batch`
- `currently_at.name`   → `current_company`
- `profile_title`       → `current_role`
- `current_city.name`   → `city`
- `linkedin_profile_url`→ `linkedin_url`
- `image.url`           → `photo_url`

## Build instructions
```bash
# 1. Install deps (Ubuntu/Debian)
sudo apt install libpqxx-dev libboost-system-dev libssl-dev cmake build-essential

# 2. Create DB + schema
psql -U postgres -c "CREATE DATABASE alumnidb;"
psql -U postgres -d alumnidb -f schema.sql

# 3. Build
cd backend && mkdir build && cd build
cmake .. && make -j$(nproc)

# 4. Run (JSON files expected at ../2023_data.json and ../2025_data.json)
./server
```


## Next steps (not yet built)
- [ ] Next.js frontend — alumni grid with search/filter UI
- [ ] Alumni detail modal/page
- [ ] Batch import endpoint (POST array)
- [ ] Admin auth (JWT)
- [ ] Connection pooling (e.g. PgBouncer or manual pool)