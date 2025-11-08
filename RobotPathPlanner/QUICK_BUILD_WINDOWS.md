# Quick Build Guide - Windows

**For experienced users who just need the commands.**

---

## Install Qt

1. Download: https://www.qt.io/download-qt-installer
2. Run installer → Sign up (free account)
3. Select:
   - ☑ Qt 6.6.x → MinGW 64-bit
   - ☑ Developer Tools → CMake
   - ☑ Developer Tools → MinGW 11.2.0
4. Install to: `C:\Qt`

---

## Build Commands

```cmd
cd "C:\Users\YourName\Desktop\WS Asia\differential robot\RobotPathPlanner"
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\mingw_64 ..
cmake --build . --config Release
C:\Qt\6.6.0\mingw_64\bin\windeployqt.exe Release\RobotPathPlanner.exe
Release\RobotPathPlanner.exe
```

**Replace `6.6.0` with your Qt version if different.**

---

## Common Paths

- **Qt MinGW:** `C:\Qt\6.6.0\mingw_64`
- **Qt MSVC:** `C:\Qt\6.6.0\msvc2019_64`
- **windeployqt:** `C:\Qt\6.6.0\mingw_64\bin\windeployqt.exe`

---

## Troubleshooting

| Error | Fix |
|-------|-----|
| DLL missing | Run `windeployqt.exe` on your .exe file |
| Qt6 not found | Add `-DCMAKE_PREFIX_PATH=C:\Qt\6.6.0\mingw_64` |
| No compiler | Install MinGW via Qt installer |
| Build fails | Clean build: `rmdir /s /q build` then retry |

---

## Full Guide

See [`WINDOWS_INSTALL_GUIDE.md`](WINDOWS_INSTALL_GUIDE.md) for detailed step-by-step instructions.
