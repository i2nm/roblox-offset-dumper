# Perc Ripper

An offset and FFlag dumper for Roblox.

This project was originally created for researching and reverse engineering Roblox internals and gaining more experience in using tools such as IDA. After sharing some of my research and findings, I started getting requests for offsets, which eventually led to the creation of Perc Ripper.

After working on the project for a while, I have decided to make the source code public. At this point, I don't have much motivation or interest in actively developing the project anymore, mainly due to the community surrounding it.

If you'd like to add something or improve the project, feel free to open a pull request. Just make sure to read the [contributing](#contributing) guidelines first. ;)

---

## Installation & Building

For a comprehensive guide on requirements, building with Visual Studio or CMake CLI, and troubleshooting, refer to [BUILD_AND_RUN.md](BUILD_AND_RUN.md).

### Prerequisites
- Windows 10/11 (64-bit)
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.16+ & Git

### Building from Source
1. Clone the repository:
   ```cmd
   git clone https://github.com/meowpurrpur/PercRipper.git
   cd PercRipper
   ```
2. Build using CMake in **Release** mode:
   ```cmd
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
   The compiled `PercRipper.exe` will be located under `build\PercRipper\Release\`.

3. [Setup Configuration](#configuration)
4. [Run](#how-to-run)

#### Configuration

Create a file named `config.json` in the same directory as the scripts/executable (if it dosen't exist):

```json
{
    "placeId": 113264082193197,
    "gameId": 8220026920,
    "creatorId": 8979812863,
    "localeId": "GB"
}
```
---

## How to Run

1. Open Roblox and join the [game](https://www.roblox.com/games/113264082193197/Dumper9000).
   - Make sure you don't move your camera around in game after joining, if you do, rejoin the game before dumping
2. Once in-game, run `run_ripper.bat` (or execute `PercRipper.exe`).
3. The dumped offsets and FFlags will be saved in a versioned directory (e.g. `version-xxxxxxxx/`).

---

## Contributing

### AI & LLM Policy
- **AI-assisted contributions are allowed, but work must be mostly human-written.**
- Fully or predominantly AI-generated pull requests, commits, or issue reports will be rejected.
- You must personally understand, review, and test every line of code you submit. Do not submit unvetted LLM output.

### Submitting Changes
1. Create an account on [git.imtheo.lol](https://git.imtheo.lol) if you do not already have one.
2. Fork the repository and create a feature branch for your changes (`git checkout -b feature/my-change`).
3. Ensure your code conforms to modern C++20 standards and matches the existing codebase conventions.

### Reporting Issues
- Provide detailed steps to reproduce the bug, including your client version, environment details, and relevant console/error logs.

---

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE) - Copyright (C) 2026 theo.
