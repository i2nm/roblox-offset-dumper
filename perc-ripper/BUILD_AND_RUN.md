# Perc Ripper - Build & Run Guide

A step-by-step guide on how to build **Perc Ripper** from source code into an executable (`PercRipper.exe`) and run it against Roblox.

---

## 1. Prerequisites

Before building, ensure you have the following installed on Windows:

1. **Visual Studio 2022** (Community, Professional, or Enterprise):
   - Download: [Visual Studio 2022](https://visualstudio.microsoft.com/vs/)
   - During installation, make sure to check **Desktop development with C++**.
   - Ensure the following individual components are selected (they are usually included by default):
     - MSVC v143 - VS 2022 C++ x64/x86 build tools
     - C++ CMake tools for Windows
     - Windows 10/11 SDK
2. **Git for Windows** (Required for CMake to fetch dependencies like Zydis automatically):
   - Download: [Git for Windows](https://git-scm.com/download/win)
3. **CMake** (v3.16+):
   - Comes included with Visual Studio if you checked "C++ CMake tools for Windows", or can be installed standalone from [cmake.org](https://cmake.org/download/).

---

## 2. Building PercRipper.exe

You can build the executable either via the **Command Line** or inside **Visual Studio**.

### Method A: Command Line (Quickest)

1. Open **Command Prompt** (`cmd`) or **PowerShell**.
2. Navigate to your `perc-ripper` folder:
   ```cmd
   cd path\to\perc-ripper
   ```
3. Generate the build files in Release mode:
   ```cmd
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```
   *(CMake will automatically download and link the Zydis disassembler library).*

4. Compile the executable:
   ```cmd
   cmake --build build --config Release
   ```
5. Once compilation finishes, your executable will be located in:
   ```
   build\PercRipper\Release\PercRipper.exe
   ```

---

### Method B: Visual Studio 2022 GUI

1. Open **Visual Studio 2022**.
2. Select **Open a local folder** and choose the `perc-ripper` directory.
3. Wait a few moments while CMake configures the project in the background.
4. At the top toolbar, set the target configuration to **x64-Release**.
5. Click **Build** in the top menu -> **Build All** (or press `Ctrl + Shift + B`).
6. The compiled `PercRipper.exe` will be located under the `out/build/x64-Release/` folder.

---

## 3. Configuration

Before running the tool, ensure `config.json` is located in the same directory as the executable (or in the root folder if using `run_ripper.bat`):

```json
{
    "placeId": 113264082193197,
    "gameId": 8220026920,
    "creatorId": 8979812863,
    "localeId": "GB"
}
```

- **`placeId`**: Target place ID (default: `113264082193197` for Dumper9000).
- **`gameId`**: Target universe/game ID.
- **`creatorId`**: Target creator ID.
- **`localeId`**: Game locale setting (`GB`, `US`, etc.).

---

## 4. How to Run

1. Open Roblox and join the required research place (e.g., [Dumper9000](https://www.roblox.com/games/113264082193197/Dumper9000)).
2. **Important**: Once inside the game, do **not** move your mouse or camera. If you moved the camera, rejoin before dumping.
3. Run the ripper:
   - Double-click `run_ripper.bat`, or
   - Run `PercRipper.exe` directly (Right-click -> **Run as Administrator** if elevated permissions are needed).
4. The console window will display scanning progress.
5. Once complete, all dumped offsets and FFlags will be exported into a versioned folder (e.g., `version-xxxxxxxx/`):
   - `Offsets.txt` — Plaintext offset list
   - `Offsets.hpp` — C++ header file
   - `Offsets.cs` — C# class definitions
   - `Offsets.json` — Standard JSON format
   - `OffsetsHex.json` — Hexadecimal JSON format
   - `Types.json` — Type definitions
