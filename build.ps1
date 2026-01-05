# Build and run simplified_uninit checker
# For Windows PowerShell

param(
    [switch]$Clean = $false,
    [string]$TestFile = ""
)

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectDir "build"

Write-Host "Simplified Uninitialized Variable Checker" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build directory..."
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }
}

# Create build directory
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure with CMake (no external dependencies)
Write-Host "`nConfiguring with CMake..."
Push-Location $BuildDir
cmake -DCMAKE_BUILD_TYPE=Release ..
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host "`nBuilding..."
cmake --build . --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}
Pop-Location

$ExePath = Join-Path $BuildDir "Release\simplified_uninit.exe"
if (!(Test-Path $ExePath)) {
    $ExePath = Join-Path $BuildDir "simplified_uninit.exe"
}

Write-Host "`nBuild successful!" -ForegroundColor Green
Write-Host "Executable: $ExePath"

# Run tests if no specific file given
if ([string]::IsNullOrEmpty($TestFile)) {
    Write-Host "`n=========================================" -ForegroundColor Cyan
    Write-Host "Test 1: Checking BAD code (should report warnings)" -ForegroundColor Cyan
    Write-Host "=========================================" -ForegroundColor Cyan
    & $ExePath (Join-Path $ProjectDir "test_uninit_bad.c")
    
    Write-Host "`n=========================================" -ForegroundColor Cyan
    Write-Host "Test 2: Checking GOOD code (should report nothing)" -ForegroundColor Cyan
    Write-Host "=========================================" -ForegroundColor Cyan
    & $ExePath (Join-Path $ProjectDir "test_uninit_good.c")
} else {
    Write-Host "`n=========================================" -ForegroundColor Cyan
    Write-Host "Checking: $TestFile" -ForegroundColor Cyan
    Write-Host "=========================================" -ForegroundColor Cyan
    & $ExePath $TestFile
}
