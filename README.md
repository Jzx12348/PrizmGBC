# PrizmGBC

<p align="center"><img src="PrizmGBC-logo.svg" alt="PrizmGBC logo" width="420"></p>
<p align="center"><strong>A Game Boy and Game Boy Color emulator for the Casio fx-CG50.</strong><br>Built for the calculator and distributed as a native <code>.g3a</code> add-in.</p>
<p align="center"><a href="https://github.com/Jzx12348/PrizmGBC/releases/latest"><strong>Latest stable release: v1.3.1</strong></a> · <a href="https://jzx12348.github.io/PrizmGBC/">Project website</a> · <a href="https://github.com/Jzx12348/PrizmGBC/releases">All releases</a></p>

---

## Overview

PrizmGBC is a community-developed fork of [Prizoop](https://github.com/tswilliamson/prizoop), bringing classic handheld gaming to Casio Prizm graphing calculators.

The project is primarily developed for the **Casio fx-CG50**. Compatibility and performance can vary between games, calculator models and OS versions.

## v1.3.0 stable features

- **Game Boy and Game Boy Color support** for .gb and .gbc ROMs, plus inherited .gbz support.
- **Redesigned game library** with persistent grid/list layouts and improved navigation.
- **Favourites and recently played** with sorting by name, recent activity, favourites and playtime.
- **Game information pages** showing ROM title, file size, cartridge type and memory capacities.
- **Per-game settings profiles** for performance, display, audio and controls.
- **Playtime tracking** and playtime-based sorting.
- **Three-slot save-state manager** for the currently loaded game.
- **F3 saves and F4 loads** the selected save-state slot, in the manager and during gameplay.
- **Function-key hints** positioned above all six function keys: F1 Menu, F2 Settings, F3 Favourite, F4 List/Grid, F5 Notes and F6 Play.
- **Categorised settings** for Performance, Display, Controls, Audio and System, with improved tab spacing.
- **Configurable key mappings** and display modes including native 1×, 1.5× Fit and 2× Crop.
- **Native calculator installation** through a .g3a add-in.

## v1.3.1 patch

- Added **F1 MENU** and **F5 NOTES** hints to the game-library footer, alongside the existing F2, F3, F4 and F6 actions.
- F5 opens the selected game's matching notes file from the menu; its in-game Select mapping is unchanged.
- **fx-CG50 stability patch:** Removed Ptune2 calls on CG50 devices and hardened shutdown cleanup. This addresses an unsupported SDK interaction that could contribute to instability; the disappearing-add-ins symptom still requires confirmation on a physical calculator.

## Download and install

1. Open the [latest stable release](https://github.com/Jzx12348/PrizmGBC/releases/latest).
2. Download **PrizmGBC.g3a**.
3. Connect the fx-CG50 to your computer and open its storage.
4. Copy the add-in to the calculator's root directory.
5. Copy your legally obtained .gb, .gbc or supported .gbz ROM files to the calculator.
6. Safely eject the calculator and launch **PrizmGBC** from the add-in menu.

Use short, simple ROM filenames. The original Prizoop documentation recommends names shorter than 12 characters; whether that limitation can be relaxed has not been verified for this fork.

## Controls

The default mappings are listed below. They can be changed in the in-emulator Controls settings.

| Game Boy control | Default calculator key |
| --- | --- |
| Directional pad | Calculator directional pad |
| A | SHIFT |
| B | OPTN |
| Select | F5 |
| Start | F6 |
| Save state | F3 |
| Load state | F4 |
| Increase volume | + |
| Decrease volume | - |

In menus, **F1** returns to the library and **F5** opens the selected game's matching `.txt` notes file. During gameplay, **F5** retains its Game Boy Select mapping.

The **MENU** key exits the current game to the emulator menu. Press MENU again to return to the calculator's main system.

## Building from source

PrizmGBC uses the **PrizmSDK / FXCGSDK toolchain**. It is not a standard desktop C++ project and cannot be built with a regular host compiler.

### Requirements

- A compatible PrizmSDK installation.
- The SDK compiler, libraries, make rules and mkg3a utility.
- A Windows or Linux environment configured for the SDK toolchain.

### SDK directory layout

The default Makefile lookup expects the repository at PrizmSDK/projects/PrizmGBC:

    PrizmSDK/
    ├── bin/
    ├── toolchain/
    └── projects/
        └── PrizmGBC/
            ├── Makefile
            ├── make-DeviceRelease.bat
            └── src/

If the SDK is installed elsewhere, set the FXCGSDK environment variable to the absolute path of the SDK root.

### Windows

From a command prompt in the repository root, run:

    make-DeviceRelease.bat

A successful device-release build should produce PrizmGBC.g3a.

### Linux

With a Linux-compatible SDK installation, run:

    export FXCGSDK="/absolute/path/to/PrizmSDK"
    make clean
    make DEFINES="-DTARGET_PRIZM=1 -DDEBUG=0" CONFIG=DeviceRelease

The exact toolchain and SDK components must match the Makefile's expectations. A successful compile does not by itself prove that the add-in works on a physical calculator.

### Visual Studio project

The optional Visual Studio solution is located at <code>proj/PrizmGBC.sln</code>. It contains the calculator project and the separate <code>comp-gb</code> ROM compression utility. The legacy <code>comp-gb</code> utility expects the external <code>prizm-zx7/zx7.inl</code> dependency in a sibling directory; that dependency is not bundled, and the utility is not required to build or run the emulator. Existing <code>.gbz</code> ROM support remains part of the emulator. The primary supported release build remains the PrizmSDK Makefile workflow used by GitHub Actions.

### If the build fails

Check that:

- The repository is in PrizmSDK/projects/PrizmGBC, or FXCGSDK points to the SDK root.
- PrizmSDK/bin/make.exe exists on Windows.
- PrizmSDK/toolchain/prizm_rules exists.
- The SDK compiler and libraries are installed.
- The build is being run from the repository root.

To clean generated output before rebuilding, run rebuild.bat followed by make-DeviceRelease.bat on Windows. Do not remove source files or SDK directories.

## Project structure

| Path | Purpose |
| --- | --- |
| <code>src/</code> | Emulator core, calculator interface, graphics, audio and menus |
| <code>src/asm/</code> | SH-family assembly routines used by the calculator build |
| <code>src/bg_compressed.cpp</code> | Embedded compressed display backgrounds |
| <code>proj/PrizmGBC.sln</code> | Optional Visual Studio solution |
| <code>comp-gb/</code> | Source for the optional .gbz ROM compression utility |
| <code>assets/upstream-backgrounds/</code> | Archived source bitmaps; not required by the current build |
| <code>docs/legacy-performance-notes.txt</code> | Archived historical development notes |
| <code>Makefile</code> | PrizmSDK build rules and .g3a packaging |
| <code>make-DeviceRelease.bat</code> | Windows device-release build entry point |

Generated executables and build outputs are intentionally excluded from source control. Build them locally with the relevant toolchain.

## Roadmap

The roadmap describes the current direction of the project. Planned features are targets, not guarantees; implementation may change based on fx-CG50 hardware, memory and SDK limitations.

### v1.3.0 · Library and usability

Released. Includes game information pages, persistent grid/list layout, refined grid navigation, favourites and recently played, playtime display and sorting, manually saved and applied per-game settings profiles, and a dedicated three-slot save-state manager. Settings migration updates legacy default save/load bindings individually while preserving custom mappings.

### v1.4.0 · Personalisation

- Custom themes and accent colours.
- Configurable game controls.
- Quick Resume.
- Save-state previews, if technically practical.
- More sorting and filtering options.
- Expanded performance statistics.

### v1.5.0 · Game Library Evolution

- Improved game organisation.
- Custom game categories and collections.
- More game metadata.
- Enhanced search.

### v1.6.0 · Save System

- Expanded save-state management.
- Save-state organisation.
- Backup and restore options.
- Improved save reliability.

### v1.7.0 · Emulator Performance

- CPU optimisation.
- Graphics rendering improvements.
- Memory optimisation.
- Better frame pacing.

### v1.8.0 · Audio & Display

- Improved audio options.
- Display configuration.
- More scaling options.
- Visual rendering improvements.

### v1.9.0 · Polish & Preparation

- Interface refinements.
- Compatibility improvements.
- Stability fixes.
- Performance tuning.
- Preparation for v2.0.0.

### v2.0.0 · Major overhaul

A substantial evolution of the emulator and the overall user experience.

- **Emulator engine:** performance optimisation, improved Game Boy and Game Boy Color compatibility, more accurate CPU, graphics, memory and timing behaviour, and audio improvements.
- **Interface:** a complete application UI overhaul, redesigned library, game information pages, updated settings and in-game menus, and transitions where performance permits.
- **Game management:** artwork and metadata, per-game configuration, expanded save-state management and previews where practical, and improved ROM organisation.
- **Configuration:** expanded display and audio options, advanced key mapping, performance monitoring and diagnostics.
- **Personalisation:** themes, accent colours and configurable interface options.
- **Reliability:** broader compatibility testing, better error handling, settings and save migration, and improved stability and memory management.

The v2.0.0 scope will be refined as earlier releases establish what is practical on the fx-CG50. The existing PrizmGBC logo and application icon are staying unchanged.

## Community and contributing

PrizmGBC is open to community bug reports, feature ideas, compatibility testing and code contributions.

- **[Report a bug](https://github.com/Jzx12348/PrizmGBC/issues/new?template=bug_report.yml):** use the structured form to provide reproducible details.
- **[Request a feature](https://github.com/Jzx12348/PrizmGBC/issues/new?template=feature_request.yml):** share ideas and explain the problem they solve.
- **[Submit a game compatibility report](https://github.com/Jzx12348/PrizmGBC/issues/new?template=compatibility_report.yml):** record how a game performs on a specific build and calculator.
- **[Contribution guide](CONTRIBUTING.md):** learn how to propose code, documentation and testing changes.
- **[Compatibility tracker](COMPATIBILITY.md):** review community game test results.
- **[Development testing programme](TESTING.md):** find available development build artifacts and submit test feedback.
- **[Changelog](CHANGELOG.md):** follow user-facing changes across releases.

Development builds are experimental and are not stable releases. A successful CI build does not guarantee correct operation on a physical calculator. Never upload or distribute copyrighted ROMs or private calculator data.

## ROM files

| File extension | Description |
| --- | --- |
| .gb | Game Boy ROM |
| .gbc | Game Boy Color ROM |
| .gbz | Optional compressed format inherited from Prizoop |
| .txt | Optional game FAQ / notes file |

A FAQ file should use the same base filename as its ROM. For example, Pokemon.gbc can have a matching Pokemon.txt. The inherited FAQ reader expects ANSI text; UTF-8 support has not been established.

Only use game files you are legally entitled to use. Commercial game ROMs are not distributed with PrizmGBC.

## Troubleshooting

### PrizmGBC does not appear in the add-in menu

- Confirm that PrizmGBC.g3a is in the calculator's root directory.
- Check that the file transfer completed and safely eject the calculator.
- Make sure you copied a device-release add-in, not a simulator executable.

### The add-in appears but will not start

Rebuild with a compatible PrizmSDK toolchain and the device-release configuration. Record the exact OS error or startup behaviour before changing emulator code.

### A game starts but does not display correctly

Try a known-good, unmodified .gb or .gbc file. Record whether the issue is a black screen, freeze, incorrect graphics, missing audio or a crash. These symptoms can have different causes.

### The build produces an unexpected filename

The Makefile fixes the target name to PrizmGBC. Build from the repository root and check the generated output before installing it.

## Credits and licensing

PrizmGBC is based on [Prizoop by tswilliamson](https://github.com/tswilliamson/prizoop), which acknowledges [Cinoop by CTurt](https://github.com/CTurt/Cinoop). The upstream project also credits BGB for its usefulness during emulator debugging and compatibility work.

Preserve the original license and copyright notices when modifying or redistributing the project. Check the upstream Prizoop repository for its applicable license terms before redistributing derivative work.

## Disclaimer

PrizmGBC is an independent community project. It is not affiliated with, endorsed by or sponsored by CASIO or Nintendo. Game Boy is a Nintendo trademark. Casio and fx-CG50 are trademarks of CASIO COMPUTER CO., LTD.

---

**Project links:** [Website](https://jzx12348.github.io/PrizmGBC/) · [Source code](https://github.com/Jzx12348/PrizmGBC) · [Releases](https://github.com/Jzx12348/PrizmGBC/releases) · [Report an issue](https://github.com/Jzx12348/PrizmGBC/issues)
