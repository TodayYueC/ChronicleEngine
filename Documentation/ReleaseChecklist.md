# Chronicle Engine Release Checklist

## Required Checks

- Build `ChronicleHostEditor` with UE 5.3.
- Run `Automation RunTests Chronicle` with UE 5.3.
- Build `ChronicleHostEditor` with UE 5.7 as a compatibility smoke test.
- Package the plugin with `Scripts/PackagePlugin.ps1`.
- Confirm no PRD or internal requirements documents are tracked by git.

## Latest Verification

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation suite: 35 tests passed.
- UE 5.3 100-node condition traversal: `0.0844ms` against a `0.25ms` budget and the PRD target of `<0.1ms`.
- UE 5.7 editor build smoke: passed.
- UE 5.3 plugin package: passed.

## Release Artifacts

- `Artifacts/ChronicleEngine-0.12.0-UE5.3`
- Optional `Artifacts/ChronicleEngine-0.12.0-UE5.3.zip` can be created from the packaged plugin folder for GitHub Releases.

## Current Known Limits

- Default UMG is provided as a C++/Blueprint base widget rather than committed binary widget assets.
- Demo content is source-generated through `AChronicleDialogueDemoActor`; a showcase `.umap` remains optional.
- UE 5.5 is not part of the required matrix because the local install is incomplete.

## CI

- Local and self-hosted CI validation share `Scripts/RunChronicleValidation.ps1`.
- GitHub Actions workflow: `.github/workflows/chronicle-validation.yml`.
- The workflow expects a self-hosted Windows Unreal runner with UE 5.3 and UE 5.7 installed at the configured paths.
