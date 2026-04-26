# Chronicle Engine 0.5.0 Release Notes

Chronicle Engine 0.5.0 is the M5 hardening pass for the UE5 JRPG dialogue plugin.

## Highlights

- Runtime traversal now builds per-run node and outgoing-edge lookup tables instead of scanning arrays on every hop.
- Condition expressions compile to a cached AST after first use.
- Variable name to Gameplay Tag resolution is cached in `UVariableBank`.
- Repeated condition results are cached during a dialogue run and invalidated when variables, save state, or events can change runtime state.
- Rollback mementos are now stored at player-visible pause points instead of every internal node transition.
- The 100-node condition traversal automation budget is tightened to `0.25ms`; the latest UE 5.3 editor automation run recorded `0.1181ms`.
- Packaging is scripted with `Scripts/PackagePlugin.ps1`.

## Verification

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation suite: 21 tests passed.
- UE 5.7 editor build smoke: passed.
- UE 5.3 `BuildPlugin` package: passed for Win64 Editor, Development Game, and Shipping Game targets.

## Packaging

Use the default UE 5.3 install path:

```powershell
.\Scripts\PackagePlugin.ps1
```

Or pass a different engine root:

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.5.0-UE5.7"
```

Generated packages are written under `Artifacts/`, which is intentionally ignored by git.
