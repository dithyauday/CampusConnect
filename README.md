Since you are using VS Code, you can do all of this without leaving your editor. Here is how to run the project using VS Code on both platforms.

---

## 🐧 1. Using VS Code on Linux (Fedora)

VS Code on Linux integrates perfectly with your native terminal.

### Step 1: Open the Project

1. Open VS Code.
2. Go to **File > Open Folder** and select your `CampusConnect` folder.

### Step 2: Open the Integrated Terminal

Press `Ctrl + ~` (or go to **Terminal > New Terminal** at the top menu).

### Step 3: Run the Commands

Paste these commands directly into the VS Code terminal panel:

```bash
# 1. Install prerequisites (if you haven't already)
sudo dnf install cmake gcc-c++ libpqxx-devel postgresql-devel boost-devel

# 2. Run your database script
psql -U postgres -d alumnidb -f backend/init.sql

# 3. Build and Run the C++ server
cd backend
mkdir build && cd build
cmake ..
make
./server

```

---

## 🪟 2. Using VS Code on Windows

On Windows, the easiest way to make VS Code compile C++ with PostgreSQL seamlessly is by leveraging **MSYS2 (UCRT64)** inside VS Code's terminal. It works exactly like a Linux terminal but runs natively on Windows.

### Step 1: Install MSYS2 & Dependencies (Outside VS Code)

1. Download and run the installer from [msys2.org](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal from your Windows Start Menu and install your compiler and C++ database libraries:
```bash
pacman -Syu --noconfirm
pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-postgresql mingw-w64-ucrt-x86_64-libpqxx mingw-w64-ucrt-x86_64-boost

```



### Step 2: Configure VS Code Terminal

1. Open VS Code on your `CampusConnect` folder.
2. Open VS Code Settings (`Ctrl + ,`), search for `Automation Profile: Windows`, and set it to use the **UCRT64 Bash** path (`C:\msys64\ucrt64.exe` or `C:\msys64\msys2_shell.cmd -ucrt64`).
3. *Alternative (Easier vibe):* Just open a fresh VS Code terminal (`Ctrl + ~`), click the dropdown arrow next to the `+` sign on the terminal panel, select **Command Prompt** (cmd), and manually initialize your compiler path:
```cmd
set PATH=C:\msys64\ucrt64\bin;%PATH%

```



### Step 3: Build and Run

Now, run these commands inside your VS Code terminal panel to compile your Windows binary target:

```bash
cd backend
mkdir build && cd build

# Configure CMake using the MinGW generator
cmake -G "MinGW Makefiles" ..

# Compile the executable 
mingw32-make

# Launch the server machine!
./server.exe

```

---

### 📡 Testing the Result

Once the server is alive on port `8080` in your VS Code terminal view, split your terminal panel or open your browser to poke the API channel:

```text
http://localhost:8080/api/alumni

```