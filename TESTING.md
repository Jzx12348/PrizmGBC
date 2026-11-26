# Development build testing programme

The testing programme helps identify regressions and compatibility problems before changes become stable releases.

Development builds are experimental. They may contain unfinished features, bugs or save-state incompatibilities. Use them only if you are comfortable testing software that may not work as expected.

## Where to find development builds

1. Open the repository's [Actions](https://github.com/Jzx12348/PrizmGBC/actions) page.
2. Select a recent successful build workflow run.
3. Check its Artifacts section for a downloadable build, if an artifact was produced.
4. Read the workflow run and commit details so you know exactly which source revision you are testing.

Not every workflow run necessarily publishes a downloadable artifact. If no artifact is attached, there is no development build available from that run.

Do not treat an Actions build as a stable release. The official stable add-in is published on the [Releases page](https://github.com/Jzx12348/PrizmGBC/releases).

## Before testing

- Keep a copy of important calculator data and save files.
- Record the PrizmGBC version or commit SHA.
- Record your calculator model and OS version.
- Use game files you are legally entitled to use.
- Do not send ROMs, BIOS files or copyrighted game data to the project.
- Avoid testing experimental builds with the only copy of important save data.

## Suggested test checklist

- [ ] Add-in appears in the calculator menu and launches.
- [ ] Game library opens and navigation works.
- [ ] A legally obtained Game Boy ROM loads.
- [ ] A legally obtained Game Boy Color ROM loads.
- [ ] Controls respond correctly.
- [ ] Display and scaling behave as expected.
- [ ] Audio behaves as expected, where enabled.
- [ ] Save-state save and load work.
- [ ] Returning to the library and exiting the add-in work.
- [ ] No unexpected calculator instability or add-in menu problems occur.

Not every test applies to every build. Focus on the feature or fix described in the workflow or pull request.

## Submit feedback

- [Report a bug](https://github.com/Jzx12348/PrizmGBC/issues/new?template=bug_report.yml) for crashes, regressions and unexpected behaviour.
- [Submit a compatibility report](https://github.com/Jzx12348/PrizmGBC/issues/new?template=compatibility_report.yml) for game-specific results.
- [Request a feature](https://github.com/Jzx12348/PrizmGBC/issues/new?template=feature_request.yml) for ideas and improvements.

Include the build version or commit, calculator model, OS version, reproduction steps and what happened. Screenshots are useful when they do not reveal personal information.

## Testing programme guidelines

- Testing is voluntary. You do not need to provide personal contact information.
- Be respectful and constructive when reporting results.
- Clearly distinguish confirmed results from guesses.
- Do not redistribute development builds as stable releases.
- A successful build or test on one calculator does not establish compatibility across all devices or games.

Thank you for helping make PrizmGBC more reliable.
