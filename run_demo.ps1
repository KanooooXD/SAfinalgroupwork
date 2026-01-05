# Simplified Uninit Checker - Complete Run Environment
# PowerShell script to demonstrate the checker on test files

Write-Host "========================================" -ForegroundColor Green
Write-Host "Simplified Uninitialized Variable Checker" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BadTestFile = Join-Path $ProjectDir "test_uninit_bad.c"
$GoodTestFile = Join-Path $ProjectDir "test_uninit_good.c"
$CheckerScript = Join-Path $ProjectDir "uninit_checker.py"

# Check if Python is available
try {
    $pythonVersion = python --version 2>&1
    Write-Host "Using: $pythonVersion" -ForegroundColor Cyan
} catch {
    Write-Host "ERROR: Python is not installed or not in PATH" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Test 1: Checking BAD code (with errors)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "File: test_uninit_bad.c" -ForegroundColor Yellow
Write-Host "Expected: Multiple warnings about uninitialized variables" -ForegroundColor Yellow
Write-Host ""

python $CheckerScript $BadTestFile

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Test 2: Checking GOOD code (no errors)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "File: test_uninit_good.c" -ForegroundColor Yellow
Write-Host "Expected: No warnings (or minimal false positives due to control flow)" -ForegroundColor Yellow
Write-Host ""

python $CheckerScript $GoodTestFile

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Demo Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Project Structure:" -ForegroundColor Cyan
Get-ChildItem $ProjectDir -Filter *.py, *.c, *.cpp, *.txt, *.md, *.ps1, *.bat -ErrorAction SilentlyContinue | 
    Select-Object Name | Format-Table -HideTableHeaders
