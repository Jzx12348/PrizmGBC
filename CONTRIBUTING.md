# Contributing to PrizmGBC

Thanks for helping improve PrizmGBC, a Game Boy and Game Boy Color emulator for the Casio fx-CG50.

Contributions can include bug reports, compatibility testing, documentation fixes, feature proposals, code improvements and testing development builds.

## Before contributing

1. Check the [open issues](https://github.com/Jzx12348/PrizmGBC/issues) to avoid duplicating work.
2. Read the [README](README.md), [roadmap](README.md#roadmap) and [testing guide](TESTING.md).
3. For larger changes, open an issue first to discuss the approach.
4. Keep changes focused. Explain what changed and why.

## Reporting bugs and requesting features

Use the GitHub issue forms:

- [Report a bug](https://github.com/Jzx12348/PrizmGBC/issues/new?template=bug_report.yml)
- [Request a feature](https://github.com/Jzx12348/PrizmGBC/issues/new?template=feature_request.yml)
- [Submit a compatibility report](https://github.com/Jzx12348/PrizmGBC/issues/new?template=compatibility_report.yml)

Include the PrizmGBC version, calculator model and OS version where possible. For game-specific issues, include the game title and region/revision, but never attach or distribute the ROM.

## Code contributions

1. Fork the repository.
2. Create a focused branch from the current `master`.
3. Make the smallest practical change that solves the issue.
4. Follow the existing C/C++ style and avoid unrelated formatting changes.
5. Build with the PrizmSDK / FXCGSDK toolchain using the instructions in the README.
6. Test on an fx-CG50 whenever possible. A successful desktop or CI build does not prove that the add-in works correctly on the calculator.
7. Update documentation and the changelog when behaviour, user-facing features or build instructions change.
8. Open a pull request against `master`. Explain the change, link related issues and list the tests performed.

Do not commit generated build output, private credentials, copyrighted ROMs or calculator data.

## Testing expectations

For emulator or calculator changes, report:

- Calculator model and OS version.
- PrizmGBC version or commit.
- Game title and region/revision, if relevant.
- Steps tested and observed results.
- Any regressions or limitations.

See [TESTING.md](TESTING.md) for the development-build testing programme and [COMPATIBILITY.md](COMPATIBILITY.md) for game reports.

## Project scope

PrizmGBC targets the Casio fx-CG50. The calculator has strict CPU, memory and SDK constraints, so a feature that works on a desktop may not be practical on the device. Compatibility and performance claims should be based on reproducible testing.

## Communication

Keep discussions respectful, specific and focused on the project. Do not post private contact details or personal information in issues or pull requests.
