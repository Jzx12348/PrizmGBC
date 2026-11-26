# PrizmGBC

**A Game Boy and Game Boy Color emulator for Casio Prizm graphing calculators.**

PrizmGBC is a community fork of [Prizoop](https://github.com/tswilliamson/prizoop), originally written for the Casio fx-CG10, fx-CG20 and fx-CG50. The project targets the calculator itself and produces a Casio add-in (`.g3a`).

> **Development status:** experimental. The inherited emulator has not yet been revalidated on current calculator hardware after the fork and subsequent changes. Treat compatibility, performance and save behaviour as unverified until a release has passed the device test checklist below.

## Features inherited from Prizoop

- Game Boy (`.gb`) and Game Boy Color (`.gbc`) emulation.
- ROM selection and an on-calculator settings menu.
- Configurable calculator-key mappings.
- Save states.
- Optional compressed `.gbz` ROM support.
- A text FAQ viewer for matching `.txt` files.
- Sound output using the calculator's supported audio connection.

These are inherited project capabilities, not a claim that every feature currently works in this fork or on every calculator OS version.

## Supported hardware

The upstream Prizoop project lists these calculator families:

- Casio fx-CG10
- Casio fx-CG20
- Casio fx-CG50

PrizmGBC is primarily being brought back to a known-good state on the **fx-CG50**. Other models should be considered unverified until tested.

## Installation

1. Download the latest `PrizmGBC.g3a` from the [GitHub Releases page](https://github.com/Jzx12348/PrizmGBC/releases), or build it from source using the instructions below.
2. Connect the calculator to your computer over USB and open its storage.
3. Copy **`PrizmGBC.g3a`** to the calculator's root directory.
4. Copy your legally obtained `.gb` or `.gbc` game files to the same root directory.
5. Safely eject the calculator and launch **PrizmGBC** from the add-in menu.

Keep ROM filenames short and simple. The original Prizoop documentation recommends names shorter than 12 characters; this fork has not yet verified whether that restriction can be relaxed.

**Release policy:** compiled `.g3a` files are distributed through GitHub Releases, not committed to the source tree. The repository is intended to contain source code, documentation, and project assets. The Makefile generates `PrizmGBC.g3a` when the project is built from a directory named `PrizmGBC`.

## Building from source

PrizmGBC uses the **PrizmSDK / FXCGSDK toolchain**. It is not a normal desktop C++ project and cannot be built by compiling the source files with a regular host compiler.

### 1. Set up the SDK directory

The expected directory layout is:

```text
PrizmSDK/
├── bin/
├── toolchain/
└── projects/
    └── PrizmGBC/
        ├── Makefile
        ├── make-DeviceRelease.bat
        ├── src/
        └── ...
```

Clone or move this repository into `PrizmSDK/projects/PrizmGBC`. The Makefile's default SDK lookup assumes that exact two-level relationship. If your SDK is somewhere else, set the `FXCGSDK` environment variable to the absolute path of the SDK root before building.

### 2. Build a calculator release on Windows

Open a command prompt in the repository directory and run:

```bat
make-DeviceRelease.bat
```

The script invokes the SDK's make executable and enables the calculator target with `TARGET_PRIZM=1` and `DEBUG=0`.

A successful build should create:

```text
PrizmGBC.g3a
```

If the build fails, check that:

- The repository is inside `PrizmSDK/projects/PrizmGBC`.
- `PrizmSDK/bin/make.exe` exists.
- `PrizmSDK/toolchain/prizm_rules` exists.
- The SDK compiler and libraries are installed and match the SDK revision.
- The build is being run from the repository root, not from `src` or `proj`.

### 3. Clean rebuild

To remove generated build output and rebuild:

```bat
rebuild.bat
make-DeviceRelease.bat
```

Do not delete source files or the SDK directories when cleaning. The Makefile's `clean` target is intended to remove generated project output only.

### 4. Linux

The project Makefile uses the PrizmSDK make rules and toolchain. Use a Linux-compatible installation of the SDK toolchain and GNU Make, then build from the project directory:

```sh
export FXCGSDK="/absolute/path/to/PrizmSDK"
make clean
make DEFINES="-DTARGET_PRIZM=1 -DDEBUG=0" CONFIG=DeviceRelease
```

The SDK must provide the compiler, linker, `prizm_rules`, Casio libraries and `mkg3a` utility expected by the Makefile. A successful host-side compile alone does not prove that the resulting add-in is valid on a calculator.

## Controls

The original Prizoop defaults are listed below. Key mappings may be changed in the in-emulator settings menu.

| Game Boy control | Default calculator key |
| --- | --- |
| Directional pad | Calculator directional pad |
| A | SHIFT |
| B | OPTN |
| Select | F5 |
| Start | F6 |
| Save state | X (multiply) |
| Load state | Right arrow / STORE |
| Increase volume | + |
| Decrease volume | - |

The MENU key exits the game to the emulator menu. Press MENU again to return to the calculator's main system.

Some games may feel better with custom key mappings. Use the settings menu rather than assuming the defaults are ideal for every title.

## ROMs and FAQ files

- Raw Game Boy ROM: `.gb`
- Raw Game Boy Color ROM: `.gbc`
- Optional compressed ROM format inherited from Prizoop: `.gbz`
- Optional FAQ text file: `.txt`

A FAQ file should share the ROM's base filename. For example, `Pokemon.gbc` can have a matching `Pokemon.txt`. The original FAQ reader expects ANSI text; UTF-8 support is not established.

Only use game files that you are legally entitled to use. Do not distribute commercial game ROMs with PrizmGBC.

## Device validation checklist

A build is not considered calculator-working merely because it compiles or launches in the Windows simulator. Before calling a release stable, test it on a physical fx-CG50 and record the calculator OS version and exact build.

- [ ] The add-in appears in the calculator's add-in menu.
- [ ] The add-in launches without an OS error or immediate return to the menu.
- [ ] The ROM browser lists files from calculator storage.
- [ ] A known-good `.gb` ROM boots and displays correctly.
- [ ] A known-good `.gbc` ROM boots and displays correctly.
- [ ] Directional input, A, B, Start and Select work.
- [ ] Gameplay remains stable for at least 10 minutes.
- [ ] Sound can be enabled and disabled without a crash.
- [ ] Save state and load state work after returning to the game.
- [ ] Exiting the game and returning to the calculator OS works.
- [ ] The same checks pass after a cold restart of the calculator.

Record failures with the game title, ROM type, calculator model, OS version, PrizmGBC build/commit, observed behaviour and any diagnostic output. Do not publish compatibility percentages until they are backed by a repeatable test list.

## Troubleshooting

### PrizmGBC does not appear in the add-in menu

- Confirm that the freshly built `PrizmGBC.g3a` is in the calculator's root directory.
- Check that the file transfer completed and the calculator was safely ejected.
- Rebuild with the PrizmSDK device-release configuration. A Windows simulator executable is not a calculator add-in.

### The build creates `Prizoop.g3a` or another unexpected filename

The Makefile sets `TARGET` from the current project directory name. Build from a directory named `PrizmGBC`, then install the resulting `PrizmGBC.g3a`. Do not rename an unrelated or stale binary and assume it is a valid build.

### The add-in appears but will not start

A visible add-in entry does not guarantee that its binary is compatible with the calculator. Rebuild with the matching PrizmSDK toolchain and the device-release defines. Then test the generated file on the physical calculator. Capture the exact OS error or startup behaviour before changing emulator code.

### The emulator starts but a game does not

Try a known-good, unmodified `.gb` or `.gbc` file. Confirm the extension and filename, then record whether the issue is a black screen, freeze, incorrect graphics, missing audio or a crash. Different symptoms point to different parts of the emulator.

## Project structure

- `src/`: emulator core, calculator interface, graphics, audio and menu code.
- `src/asm/`: SH-family assembly routines used by the calculator build.
- `src/ptune2_simple/`: inherited calculator clock/performance support.
- `proj/`: Visual Studio project and simulator configuration.
- `Makefile`: PrizmSDK build rules and `.g3a` packaging.
- `make-DeviceRelease.bat`: Windows device-release build entry point.

## Credits and upstream

PrizmGBC is based on [Prizoop by tswilliamson](https://github.com/tswilliamson/prizoop), which in turn acknowledges [Cinoop by CTurt](https://github.com/CTurt/Cinoop). The upstream project also credits BGB for its usefulness during emulator debugging and compatibility work.

This fork should preserve the original license and copyright notices. See the repository's license information before redistributing modified binaries or source.

## Disclaimer

PrizmGBC is an independent community project and is not affiliated with, endorsed by or sponsored by CASIO or Nintendo. Game Boy is a Nintendo trademark. Casio and fx-CG50 are trademarks of CASIO COMPUTER CO., LTD.


## Website and downloads

The project website is published with GitHub Pages from `index.html`. The website links to the source repository and the [GitHub Releases page](https://github.com/Jzx12348/PrizmGBC/releases), where packaged `.g3a` builds belong. Enable **Settings → Pages → Build and deployment → Source: GitHub Actions** in the repository if Pages has not already been activated.

### Release policy

- Keep compiled `.g3a` files out of the Git source tree.
- Attach the calculator add-in as `PrizmGBC.g3a` to a versioned GitHub Release.
- Include the source commit, build configuration, calculator OS tested, and known limitations in each release's notes.
- Mark builds as pre-releases until the fx-CG50 device validation checklist has passed.

## Current hardware diagnostic build

The initial fx-CG50 display investigation now defaults to **unscaled rendering**. This deliberately tests the simpler 1:1 display path before re-enabling the 150% and 200% scaling modes. The settings version was bumped, so the first launch of this build resets saved emulator settings to defaults. This is a diagnostic change, not confirmation that the renderer is fixed. Please test a small, known-good `.gb` ROM and a `.gbc` ROM separately and report whether either displays.
