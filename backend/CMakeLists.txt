cmake_minimum_required(VERSION 3.15)
project(CampusConnect)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ──────────────────────────────────────────────────────────────

# Crow requires Boost.System and Boost.Filesystem (header-only parts are enough)
find_package(Boost 1.71 REQUIRED COMPONENTS system)

# OpenSSL (Crow uses it for optional TLS; linker needs it regardless)
find_package(OpenSSL REQUIRED)

# Threads (Crow's multithreaded() needs pthreads on Linux)
find_package(Threads REQUIRED)

# libpqxx via pkg-config (most reliable on Ubuntu/Debian)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(server main.cpp)

# crow_all.h lives next to main.cpp; add current source dir to includes
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}   # picks up crow_all.h
    ${PQXX_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(server PRIVATE
    ${PQXX_LIBRARIES}
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# ── Cross-Platform Windows Networking Bridge ──────────────────────────────────
if(WIN32)
    target_link_libraries(server PRIVATE
        ws2_32              # Core Windows Networking API
        mswsock             # Advanced Windows Sockets
    )
endif()

# ── Install hint (optional) ───────────────────────────────────────────────────
# sudo apt install libpqxx-dev libboost-system-dev libssl-dev