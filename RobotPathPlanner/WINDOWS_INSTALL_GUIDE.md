# RobotPathPlanner GUI - Windows Installation Guide

Complete step-by-step instructions for installing Qt and building the PathPlanner GUI on Windows.

---

## Prerequisites

- **Windows 10 or 11** (64-bit)
- **At least 10 GB free disk space** (Qt + tools)
- **Administrator access** for installation

---

## Step 1: Install Qt

### Option A: Qt Online Installer (Recommended)

1. **Download Qt Online Installer**
   - Go to: https://www.qt.io/download-qt-installer
   - Click **"Download the Qt Online Installer"**
   - Download: `qt-unified-windows-x64-online.exe`

2. **Run the Installer**
   - Double-click the downloaded `.exe` file
   - Click **Yes** when User Account Control prompts

3. **Create Qt Account (or Sign In)**
   - You need a free Qt account to proceed
   - Click **"Sign up"** if you don't have one
   - Fill in email and create password
   - **Important:** Select **"I am an individual"** (free option)

4. **Installation Settings**
   - **Installation Folder:** Use default `C:\Qt` (recommended)
   - Click **Next**

5. **Select Components** (CRITICAL STEP)

   ✅ **Expand "Qt" → "Qt 6.6.0" (or latest Qt 6.x)**

   Select these components:
   - ☑ **MSVC 2019 64-bit** (or MSVC 2022 64-bit)
   - ☑ **MinGW 11.2.0 64-bit** (alternative compiler, recommended)
   - ☑ **Qt 5 Compatibility Module**
   - ☑ **Additional Libraries** (optional but helpful)

   ✅ **Expand "Developer and Designer Tools"**

   Select:
   - ☑ **CMake 3.24.2** (or latest)
   - ☑ **Ninja 1.10.2**
   - ☑ **MinGW 11.2.0 64-bit** (toolchain)

   **Minimum Size:** ~5 GB
   **Recommended Size:** ~8 GB

6. **Install**
   - Click **Next** through license agreements
   - Click **Install**
   - Wait 15-30 minutes (depends on internet speed)

7. **Complete Installation**
   - Click **Finish**
   - You can close Qt Creator if it auto-launches

---

### Option B: Qt Offline Installer (For Slow Connections)

1. Go to: https://www.qt.io/offline-installers
2. Download: **Qt 6.6.x for Windows**
3. Follow similar steps as Option A

---

## Step 2: Install CMake (If Not Included)

If you didn't install CMake via Qt installer:

1. **Download CMake**
   - Go to: https://cmake.org/download/
   - Download: **Windows x64 Installer** (cmake-3.x.x-windows-x86_64.msi)

2. **Install CMake**
   - Run the `.msi` installer
   - ✅ Check **"Add CMake to system PATH for all users"**
   - Complete installation

3. **Verify Installation**
   ```cmd
   cmake --version
   ```
   Should show: `cmake version 3.x.x`

---

## Step 3: Verify Qt Installation

1. **Open Command Prompt**
   - Press `Win + R`
   - Type `cmd` and press Enter

2. **Check qmake**
   ```cmd
   C:\Qt\6.6.0\mingw_64\bin\qmake.exe --version
   ```

   Should show:
   ```
   QMake version 3.1
   Using Qt version 6.6.0 in C:/Qt/6.6.0/mingw_64/lib
   ```

3. **Note Your Qt Path**

   Common paths:
   - **MinGW:** `C:\Qt\6.6.0\mingw_64`
   - **MSVC:** `C:\Qt\6.6.0\msvc2019_64`

   You'll need this later!

---

## Step 4: Build RobotPathPlanner GUI

### Method 1: Using CMake GUI (Easiest for Beginners)

1. **Open CMake GUI**
   - Search for "CMake" in Start Menu
   - Or run: `C:\Program Files\CMake\bin\cmake-gui.exe`

2. **Configure Paths**
   - **Where is the source code:**
     ```
     C:\Users\YourName\Desktop\WS Asia\differential robot\RobotPathPlanner
     ```
   - **Where to build the binaries:**
     ```
     C:\Users\YourName\Desktop\WS Asia\differential robot\RobotPathPlanner\build
     ```

   Click **Configure**

3. **Select Generator**
   - Generator: **MinGW Makefiles** (or **Ninja**)
   - Click **Finish**

4. **Set Qt Path** (if error appears)
   - Click **Add Entry** button
   - Name: `CMAKE_PREFIX_PATH`
   - Type: `PATH`
   - Value: `C:\Qt\6.6.0\mingw_64` (use YOUR Qt path)
   - Click **OK**
   - Click **Configure** again

5. **Generate**
   - Click **Generate**
   - Wait for "Generating done"

6. **Build**
   - Open Command Prompt in build folder
   - Run:
     ```cmd
     cd "C:\Users\YourName\Desktop\WS Asia\differential robot\RobotPathPlanner\build"
     cmake --build .
     ```

---

### Method 2: Using Command Line (Advanced)

1. **Open Command Prompt**
   ```cmd
   cd "C:\Users\YourName\Desktop\WS Asia\differential robot\RobotPathPlanner"
   ```

2. **Create Build Directory**
   ```cmd
   mkdir build
   cd build
   ```

3. **Run CMake** (MinGW)
   ```cmd
   cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\mingw_64 ..
   ```

   **OR** (Ninja - faster)
   ```cmd
   cmake -G "Ninja" -DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\mingw_64 ..
   ```

   **OR** (Visual Studio)
   ```cmd
   cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\msvc2019_64 ..
   ```

4. **Build**
   ```cmd
   cmake --build . --config Release
   ```

5. **Wait for Build**
   - Should take 2-5 minutes
   - Look for: **Build succeeded** or **[100%]**

---

## Step 5: Run RobotPathPlanner

1. **Navigate to Build Folder**
   ```cmd
   cd build
   ```

2. **Run the Application**

   **MinGW:**
   ```cmd
   Release\RobotPathPlanner.exe
   ```

   **OR**
   ```cmd
   RobotPathPlanner.exe
   ```

3. **If "DLL Missing" Error Appears:**

   You need to copy Qt DLLs. Run this from the build folder:
   ```cmd
   C:\Qt\6.6.0\mingw_64\bin\windeployqt.exe Release\RobotPathPlanner.exe
   ```

   This automatically copies all required DLLs.

4. **Application Should Launch!**
   - You should see the PathPlanner GUI window
   - Main window with menu bar
   - Panels for Paths and Robot

---

## Troubleshooting

### Error: "Qt6Core.dll is missing"

**Solution:**
```cmd
cd "C:\Users\YourName\Desktop\WS Asia\differential robot\RobotPathPlanner\build"
C:\Qt\6.6.0\mingw_64\bin\windeployqt.exe Release\RobotPathPlanner.exe
```

---

### Error: "Could NOT find Qt6 (missing: Qt6_DIR)"

**Solution:** Specify Qt path explicitly:
```cmd
cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\mingw_64 ..
```

Make sure the path matches YOUR Qt installation!

---

### Error: "No CMAKE_CXX_COMPILER could be found"

**Solution:** Install MinGW via Qt installer, or use Visual Studio:

1. Install **Visual Studio 2022 Community** (free)
   - Download: https://visualstudio.microsoft.com/downloads/
   - Select **"Desktop development with C++"** workload

2. Use MSVC generator:
   ```cmd
   cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\msvc2019_64 ..
   ```

---

### Error: Build fails with compiler errors

**Solution:** Make sure you selected the correct Qt modules:
- Qt6::Core
- Qt6::Gui
- Qt6::Widgets
- Qt6::Network

Reinstall Qt with all required components.

---

### Application crashes on startup

**Solution:**
1. Make sure you ran `windeployqt`
2. Check antivirus isn't blocking it
3. Try running from Qt Creator:
   - Open `CMakeLists.txt` in Qt Creator
   - Click **Run** button

---

## Creating a Standalone Executable

To create a portable version you can copy to other computers:

1. **Build in Release mode:**
   ```cmd
   cmake --build . --config Release
   ```

2. **Deploy Qt DLLs:**
   ```cmd
   C:\Qt\6.6.0\mingw_64\bin\windeployqt.exe --release Release\RobotPathPlanner.exe
   ```

3. **Copy entire Release folder** to other Windows PCs
   - The folder will contain:
     - `RobotPathPlanner.exe`
     - All required DLLs
     - Qt plugins folder

4. **Requirements on target PC:**
   - Windows 10/11
   - **Visual C++ Redistributable 2015-2022**
     - Download: https://aka.ms/vs/17/release/vc_redist.x64.exe

---

## Quick Reference: Command Summary

```cmd
# 1. Navigate to project
cd "C:\Users\YourName\Desktop\WS Asia\differential robot\RobotPathPlanner"

# 2. Create build directory
mkdir build
cd build

# 3. Configure with CMake (MinGW)
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\mingw_64 ..

# 4. Build
cmake --build . --config Release

# 5. Deploy Qt DLLs
C:\Qt\6.6.0\mingw_64\bin\windeployqt.exe Release\RobotPathPlanner.exe

# 6. Run
Release\RobotPathPlanner.exe
```

---

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **OS** | Windows 10 64-bit | Windows 11 64-bit |
| **RAM** | 4 GB | 8 GB |
| **Disk Space** | 10 GB | 15 GB |
| **CPU** | Dual-core | Quad-core |
| **Graphics** | Any | Dedicated GPU |

---

## Alternative: Using Qt Creator IDE

If you prefer a graphical IDE:

1. **Launch Qt Creator**
   - Start Menu → Qt Creator

2. **Open Project**
   - File → Open File or Project
   - Select: `RobotPathPlanner\CMakeLists.txt`

3. **Configure Kit**
   - Select: **Desktop Qt 6.6.0 MinGW 64-bit**
   - Click **Configure Project**

4. **Build**
   - Click the **hammer icon** (Build)
   - Or press `Ctrl + B`

5. **Run**
   - Click the **play icon** (Run)
   - Or press `Ctrl + R`

---

## Next Steps

After successfully building:

1. ✅ Read [`QUICKSTART.md`](QUICKSTART.md) for usage guide
2. ✅ Read [`README.md`](README.md) for features overview
3. ✅ Connect to your robot and test communication
4. ✅ See [`../PATHPLANNER_INTEGRATION.md`](../PATHPLANNER_INTEGRATION.md) for robot integration

---

## Getting Help

### Qt Installation Issues
- Qt Forum: https://forum.qt.io/
- Qt Documentation: https://doc.qt.io/

### Build Issues
- Check `CMakeLists.txt` in project root
- Verify Qt version compatibility
- Try clean rebuild:
  ```cmd
  rmdir /s /q build
  mkdir build
  cd build
  cmake ..
  ```

### Application Issues
- Check console output for errors
- Verify network connectivity (for robot connection)
- Test with example paths first

---

## Video Tutorial (Recommended for First-Time Users)

If you prefer video instructions, search YouTube for:
- "How to install Qt on Windows"
- "Qt6 CMake setup Windows"
- "Building Qt application with CMake"

---

**Good luck! If you encounter any issues, the most common fix is ensuring CMAKE_PREFIX_PATH points to your Qt installation.**

📧 **Support:** See main README.md for contact information

🎯 **Goal:** Get the GUI running → Connect to robot → Plan paths!
