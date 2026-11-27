# Changelog

All notable user-facing changes to PrizmGBC are documented here. New development changes should be recorded under **Unreleased** as they are merged. When a version is published, move the relevant entries into a dated version section and keep the GitHub release notes consistent.

## Unreleased

- Community contribution workflow: structured bug, feature request and game compatibility issue forms.
- Added contribution guidance, a community compatibility tracker and development-build testing instructions.
- Added this changelog and linked community resources from the README and project website.
- Hardened F5 game-notes loading and rendering: bounds-check note offsets, allocate a terminated read buffer, and prevent line rendering from reading beyond the loaded text window.

## v1.3.1 - 2026-11-26

- Added the **F1 MENU** and **F5 NOTES** hints to the game-library footer.
- F5 opens the selected game's matching notes file from the library; its in-game Game Boy Select mapping remains unchanged.
- fx-CG50 stability patch: avoid Ptune2 calls on CG50 devices and harden shutdown cleanup. Physical calculator testing is still required to confirm whether this resolves the reported disappearing-add-ins symptom.

## v1.3.0 - 2026-11-26

- Redesigned the game library with persistent grid/list layouts and improved navigation.
- Added game information pages with ROM title, file size, cartridge type and memory information.
- Added favourites, recently played, playtime tracking and additional sorting options.
- Added per-game settings profiles.
- Added a three-slot save-state manager, with F3 save and F4 load actions.
- Added function-key hints for library actions.
- Reorganised settings into Performance, Display, Controls, Audio and System categories.

## v1.3.0-beta

- Prerelease testing build for the v1.3.0 feature set. This beta is archived and is not the current stable release.

---

For earlier project history, see the Git history and [published releases](https://github.com/Jzx12348/PrizmGBC/releases).
