param(
    [string]$Engine53Root = "R:\UE\UE_5.3",
    [string]$Engine57Root = "R:\UE\UE_5.7",
    [switch]$SkipAutomation,
    [switch]$SkipPackage,
    [switch]$Skip57Smoke,
    [string]$PackageName = "ChronicleEngine-0.12.0-UE5.3"
)

$ErrorActionPreference = "Stop"

$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Resolve-Path (Join-Path $ScriptRoot "..")
$ProjectPath = Join-Path $RepoRoot "ChronicleHost.uproject"
$Build53 = Join-Path $Engine53Root "Engine\Build\BatchFiles\Build.bat"
$EditorCmd53 = Join-Path $Engine53Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$Build57 = Join-Path $Engine57Root "Engine\Build\BatchFiles\Build.bat"

function Invoke-ChronicleCommand {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )

    if (-not (Test-Path $FilePath)) {
        throw "Required executable not found: $FilePath"
    }

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $FilePath"
    }
}

Write-Host "== UE 5.3 editor build =="
Invoke-ChronicleCommand $Build53 @(
    "ChronicleHostEditor",
    "Win64",
    "Development",
    "-Project=$ProjectPath",
    "-WaitMutex"
)

if (-not $SkipAutomation) {
    Write-Host "== UE 5.3 Chronicle automation =="
    Invoke-ChronicleCommand $EditorCmd53 @(
        $ProjectPath,
        "-Unattended",
        "-NullRHI",
        "-NoSplash",
        "-NoSound",
        "-ExecCmds=Automation RunTests Chronicle; Quit"
    )
}

if (-not $SkipPackage) {
    Write-Host "== UE 5.3 BuildPlugin package =="
    & (Join-Path $ScriptRoot "PackagePlugin.ps1") -EngineRoot $Engine53Root -PackageName $PackageName
    if ($LASTEXITCODE -ne 0) {
        throw "PackagePlugin.ps1 failed with exit code $LASTEXITCODE"
    }
}

if (-not $Skip57Smoke) {
    Write-Host "== UE 5.7 editor build smoke =="
    Invoke-ChronicleCommand $Build57 @(
        "ChronicleHostEditor",
        "Win64",
        "Development",
        "-Project=$ProjectPath",
        "-WaitMutex"
    )

    Write-Host "== Restore UE 5.3 editor build after smoke =="
    Invoke-ChronicleCommand $Build53 @(
        "ChronicleHostEditor",
        "Win64",
        "Development",
        "-Project=$ProjectPath",
        "-WaitMutex"
    )
}

Write-Host "Chronicle validation completed successfully."
