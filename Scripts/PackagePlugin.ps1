param(
    [string]$EngineRoot = "R:\UE\UE_5.3",
    [string]$PackageName = "ChronicleEngine-0.12.0-UE5.3"
)

$ErrorActionPreference = "Stop"

$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Resolve-Path (Join-Path $ScriptRoot "..")
$PluginPath = Join-Path $RepoRoot "Plugins\ChronicleEngine\ChronicleEngine.uplugin"
$PackageRoot = Join-Path $RepoRoot "Artifacts"
$PackagePath = Join-Path $PackageRoot $PackageName
$RunUatPath = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"

if (-not (Test-Path $RunUatPath)) {
    throw "RunUAT not found at $RunUatPath"
}

New-Item -ItemType Directory -Force -Path $PackageRoot | Out-Null

if (Test-Path $PackagePath) {
    $ResolvedPackageRoot = (Resolve-Path $PackageRoot).Path
    $ResolvedPackagePath = (Resolve-Path $PackagePath).Path
    if (-not $ResolvedPackagePath.StartsWith($ResolvedPackageRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove package path outside artifact root: $ResolvedPackagePath"
    }

    Remove-Item -LiteralPath $ResolvedPackagePath -Recurse -Force
}

& $RunUatPath BuildPlugin `
    -Plugin="$PluginPath" `
    -Package="$PackagePath" `
    -Rocket

if ($LASTEXITCODE -ne 0) {
    throw "BuildPlugin failed with exit code $LASTEXITCODE"
}

Write-Host "Packaged Chronicle Engine to $PackagePath"
