// 1. Core Windows flags to reduce header bloat
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define DO_NOT_DEFINE_DELETE

// 2. Load the core Boost networking headers first
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>

// 3. Build the compatibility bridge for Boost 1.90.0 vs Crow
namespace boost { 
    namespace asio { 
        using io_service = io_context; 
        
        // Create a custom type that patches the missing method name
        class legacy_deadline_timer : public steady_timer {
        public:
            template<typename... Args>
            legacy_deadline_timer(Args&&... args) : steady_timer(std::forward<Args>(args)...) {}
            
            // Catch the old method call and instantly convert it to the modern equivalent
            template<typename Duration>
            void expires_from_now(const Duration&) {
                this->expires_after(std::chrono::seconds(1));
            }
        };

        using deadline_timer = legacy_deadline_timer;
    } 
}

// 4. WIPE OUT WINDOWS MACRO HIJACKS
#ifdef DELETE
#undef DELETE
#endif

#ifdef ERROR
#undef ERROR
#endif

#ifdef INFO
#undef INFO
#endif

#ifdef DEBUG
#undef DEBUG
#endif

// 5. NOW INCLUDE CROW AND UTILITIES
#include "crow_all.h" // like pythons fastapi 
#include <pqxx/pqxx>  // this is for postgreSQL connection and queries
#include <iostream>   // for terminal output
#include <fstream>    // reads external json files 
#include <string>     // for text strings 
#include <sstream>

// ─── DB Config ───────────────────────────────────────────────────────────────
const std::string DB_CONFIG =
    "dbname=alumnidb user=postgres password=123 host=127.0.0.1 port=5432";

// ─── Helpers ─────────────────────────────────────────────────────────────────

// Strips out bad Unicode surrogate pairs that cause PostgreSQL to throw a fit
std::string sanitizeUtf8(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = input[i];
        // Detect standard broken high surrogate patterns (like 0xED 0xA0 0xBD)
        if (c == 0xED && (i + 2 < input.size())) {
            unsigned char next1 = input[i+1];
            unsigned char next2 = input[i+2];
            if (next1 >= 0xA0 && next1 <= 0xBF) {
                i += 2; // Skip the malformed surrogate sequence entirely
                continue;
            }
        }
        output.push_back(c);
    }
    return output;
}

// Safely extract a string from a crow::json value, returns fallback on missing/null
std::string safeStr(const crow::json::rvalue& node, const std::string& key,
                    const std::string& fallback = "") {
    if (!node.has(key)) return fallback;
    const auto& v = node[key];
    if (v.t() == crow::json::type::Null || v.t() != crow::json::type::String)
        return fallback;
    return sanitizeUtf8(std::string(v.s()));
}

// Build a valid PostgreSQL TEXT[] literal from a comma-separated string
std::string toPostgresArray(const std::string& csv) {
    if (csv.empty()) return "{}";
    std::string result = "{";
    std::istringstream ss(csv);
    std::string token;
    bool first = true;
    while (std::getline(ss, token, ',')) {
        size_t s = token.find_first_not_of(" \t");
        size_t e = token.find_last_not_of(" \t");
        if (s == std::string::npos) continue;
        token = token.substr(s, e - s + 1);
        std::string escaped;
        for (char c : token) {
            if (c == '"') escaped += "\\\"";
            else escaped += c;
        }
        if (!first) result += ",";
        result += "\"" + escaped + "\"";
        first = false;
    }
    return result + "}";
}

// Add CORS headers to every response
void addCors(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin",  "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
}

// ─── Seeder ──────────────────────────────────────────────────────────────────
void seedFromJson(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "⚠️  Seed file not found: " << filename << "\n";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    auto json = crow::json::load(content);
    if (!json) {
        std::cerr << "❌ Invalid JSON in: " << filename << "\n";
        return;
    }

    try {
        pqxx::connection conn(DB_CONFIG);
        pqxx::work txn(conn);
        int count = 0;

        for (const auto& item : json) {
            std::string uid = safeStr(item, "external_id");
            if (uid.empty()) uid = safeStr(item, "id");
            if (uid.empty()) continue;

            std::string name       = safeStr(item, "name", "Unknown");
            std::string department = "General Engineering";
            std::string course     = "";
            std::string city       = "";
            std::string company    = "";
            std::string role       = "";
            std::string linkedin   = safeStr(item, "linkedin_profile_url");
            std::string photo_url  = "";
            int    grad_year       = 0;
            int    from_year       = 0;
            std::string batch      = safeStr(item, "batch");

            if (item.has("branch") && item["branch"].has("name"))
                department = sanitizeUtf8(std::string(item["branch"]["name"].s()));

            if (item.has("course") && item["course"].has("name"))
                course = sanitizeUtf8(std::string(item["course"]["name"].s()));

            if (item.has("to_year") && item["to_year"].t() == crow::json::type::Number)
                grad_year = static_cast<int>(item["to_year"].i());

            if (item.has("from_year") && item["from_year"].t() == crow::json::type::Number)
                from_year = static_cast<int>(item["from_year"].i());

            if (item.has("currently_at") && item["currently_at"].has("name"))
                company = sanitizeUtf8(std::string(item["currently_at"]["name"].s()));

            if (item.has("profile_title") && item["profile_title"].t() == crow::json::type::String)
                role = sanitizeUtf8(std::string(item["profile_title"].s()));

            if (item.has("current_city") && item["current_city"].has("name"))
                city = sanitizeUtf8(std::string(item["current_city"]["name"].s()));

            if (item.has("image") && item["image"].has("url"))
                photo_url = sanitizeUtf8(std::string(item["image"]["url"].s()));

            txn.exec(
                "INSERT INTO alumni "
                "(uid, name, department, course, graduation_year, from_year, batch, "
                " current_company, \"current_role\", city, linkedin_url, photo_url) "
                "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12) "
                "ON CONFLICT (uid) DO NOTHING;",
                pqxx::params{
                    uid, name, department, course,
                    grad_year, from_year, batch,
                    company, role, city, linkedin, photo_url
                }
            );
            ++count;
        }

        txn.commit();
        std::cout << "✅ Seeded " << count << " records from " << filename << "\n";

    } catch (const std::exception& e) {
        std::cerr << "❌ Seed error (" << filename << "): " << e.what() << "\n";
    }
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "⏳ Seeding database...\n";
    seedFromJson("../2023_data.json");
    seedFromJson("../2025_data.json");

    crow::SimpleApp app;

    // ── GET & OPTIONS /api/alumni ────────────────────────────────────────────
    CROW_ROUTE(app, "/api/alumni").methods(crow::HTTPMethod::GET, crow::HTTPMethod::OPTIONS)(
    [](const crow::request& req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
            crow::response res(204);
            addCors(res);
            return res;
        }

        std::string search     = req.url_params.get("search")     ? req.url_params.get("search")     : "";
        std::string batch      = req.url_params.get("batch")      ? req.url_params.get("batch")      : "";
        std::string department = req.url_params.get("department") ? req.url_params.get("department") : "";
        std::string company    = req.url_params.get("company")    ? req.url_params.get("company")    : "";

        std::string sql =
            "SELECT uid, name, department, course, graduation_year, from_year, batch, "
            "current_company, \"current_role\", city, linkedin_url, photo_url "
            "FROM alumni WHERE 1=1";

        if (!search.empty())
            sql += " AND (LOWER(name) LIKE LOWER('%" + search + "%') "
                   "OR LOWER(department) LIKE LOWER('%" + search + "%'))";
        if (!batch.empty())
            sql += " AND batch = '" + batch + "'";
        if (!department.empty())
            sql += " AND LOWER(department) LIKE LOWER('%" + department + "%')";
        if (!company.empty())
            sql += " AND LOWER(current_company) LIKE LOWER('%" + company + "%')";

        sql += " ORDER BY graduation_year DESC, name ASC LIMIT 200;";

        try {
            pqxx::connection conn(DB_CONFIG);
            pqxx::work txn(conn);
            pqxx::result rows = txn.exec(sql);

            std::ostringstream json_stream;
            json_stream << "[";
            bool first_item = true;

            for (const auto& row : rows) {
                if (!first_item) {
                    json_stream << ",";
                }
                first_item = false;

                crow::json::wvalue item;
                item["uid"]              = row["uid"].c_str();
                item["name"]             = row["name"].c_str();
                item["department"]       = row["department"].c_str();
                item["course"]           = row["course"].c_str();
                item["graduation_year"]  = row["graduation_year"].as<int>(0);
                item["from_year"]        = row["from_year"].as<int>(0);
                item["batch"]            = row["batch"].c_str();
                item["current_company"]  = row["current_company"].c_str();
                item["current_role"]     = row["current_role"].c_str();
                item["city"]             = row["city"].c_str();
                item["linkedin_url"]     = row["linkedin_url"].c_str();
                item["photo_url"]        = row["photo_url"].c_str();

                json_stream << crow::json::dump(item);
            }
            json_stream << "]";
            txn.commit();

            crow::response res;
            res.body = json_stream.str();
            res.add_header("Content-Type", "application/json");
            addCors(res);
            return res;

        } catch (const std::exception& e) {
            crow::response res(500, std::string("DB error: ") + e.what());
            addCors(res);
            return res;
        }
    });

    // ── POST & OPTIONS /api/alumni/add ───────────────────────────────────────
    CROW_ROUTE(app, "/api/alumni/add").methods(crow::HTTPMethod::POST, crow::HTTPMethod::OPTIONS)(
    [](const crow::request& req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
            crow::response res(204);
            addCors(res);
            return res;
        }

        auto body = crow::json::load(req.body);
        if (!body) {
            crow::response res(400, "Invalid JSON");
            addCors(res);
            return res;
        }

        auto require = [&](const std::string& key) -> std::string {
            if (!body.has(key)) throw std::invalid_argument("Missing field: " + key);
            return sanitizeUtf8(std::string(body[key].s()));
        };

        try {
            std::string uid        = require("uid");
            std::string name       = require("name");
            std::string department = safeStr(body, "department");
            std::string course     = safeStr(body, "course");
            std::string batch      = safeStr(body, "batch");
            std::string company    = safeStr(body, "current_company");
            std::string role       = safeStr(body, "current_role");
            std::string city       = safeStr(body, "city");
            std::string linkedin   = safeStr(body, "linkedin_url");
            std::string photo      = safeStr(body, "photo_url");

            int grad_year  = body.has("graduation_year") ? static_cast<int>(body["graduation_year"].i()) : 0;
            int from_year  = body.has("from_year")       ? static_cast<int>(body["from_year"].i())       : 0;

            pqxx::connection conn(DB_CONFIG);
            pqxx::work txn(conn);

            txn.exec(
                "INSERT INTO alumni "
                "(uid, name, department, course, graduation_year, from_year, batch, "
                " current_company, \"current_role\", city, linkedin_url, photo_url) "
                "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12) "
                "ON CONFLICT (uid) DO UPDATE SET "
                "  name=EXCLUDED.name, department=EXCLUDED.department, "
                "  course=EXCLUDED.course, current_company=EXCLUDED.current_company, "
                "  \"current_role\"=EXCLUDED.\"current_role\", city=EXCLUDED.city, "
                "  linkedin_url=EXCLUDED.linkedin_url, photo_url=EXCLUDED.photo_url;",
                pqxx::params{
                    uid, name, department, course,
                    grad_year, from_year, batch,
                    company, role, city, linkedin, photo
                }
            );
            txn.commit();

            crow::response res(201, "{\"message\":\"Alumni saved successfully\"}");
            res.add_header("Content-Type", "application/json");
            addCors(res);
            return res;

        } catch (const std::invalid_argument& e) {
            crow::response res(400, std::string("Validation error: ") + e.what());
            addCors(res);
            return res;
        } catch (const std::exception& e) {
            crow::response res(500, std::string("DB error: ") + e.what());
            addCors(res);
            return res;
        }
    });

    std::cout << "🚀 CampusConnect backend on http://localhost:8081\n";
    app.port(8081).multithreaded().run();
}