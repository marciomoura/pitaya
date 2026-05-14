param (
    [Parameter(Mandatory=$true)]
    [string]$TestFilter,
    
    [string]$OutputDir = "reports",
    
    [switch]$Open
)

# 1. Create output directory
if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

# 2. Run GTest
Write-Host "Running tests with filter: $TestFilter..." -ForegroundColor Cyan
$TestExe = ".\build-host\bin\pitaya_tests.exe"
& $TestExe --gtest_filter=$TestFilter

if ($LASTEXITCODE -ne 0) {
    Write-Error "Tests failed!"
    exit $LASTEXITCODE
}

# 3. Find generated binary files
# The gtest_exporter saves files as Suite_Case.bin in the current directory or temp.
# We expect the test to be configured to save in the current dir or we can search.
$BinFiles = Get-ChildItem -Filter "*.bin"

if ($BinFiles.Count -eq 0) {
    Write-Warning "No binary data files found. Did you use pitaya::gtest_exporter?"
    exit 0
}

# 4. Generate Reports
foreach ($file in $BinFiles) {
    Write-Host "Generating report for $($file.Name)..." -ForegroundColor Cyan
    $HtmlFile = Join-Path $OutputDir ($file.BaseName + ".html")
    python pitaya/tools/plot_data.py $file.FullName -o $HtmlFile
    
    if ($Open) {
        Start-Process $HtmlFile
    }
    
    # Move bin file to output dir for archiving
    Move-Item $file.FullName (Join-Path $OutputDir $file.Name) -Force
}

Write-Host "Done! Reports saved in $OutputDir" -ForegroundColor Green
