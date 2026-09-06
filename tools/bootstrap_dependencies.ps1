$ErrorActionPreference = 'Stop'

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$externalRoot = Join-Path $projectRoot 'external'
$juceDirectory = Join-Path $externalRoot 'JUCE'
$juceUrl = 'https://github.com/juce-framework/JUCE.git'
$juceCommit = 'e18f7f506c0b96f2c738a0bcd7fe6467a5005ad8'
$patchPath = Join-Path $projectRoot 'tools\patches\JUCE-9.0.1-msvc-toolchain.patch'

New-Item -ItemType Directory -Force -Path $externalRoot | Out-Null

if (-not (Test-Path (Join-Path $juceDirectory 'CMakeLists.txt'))) {
    if (Test-Path $juceDirectory) {
        throw "JUCE directory exists but is incomplete: $juceDirectory"
    }

    & git clone --branch 9.0.1 --depth 1 $juceUrl $juceDirectory
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to clone JUCE 9.0.1 from $juceUrl"
    }
}

$safeDirectory = "safe.directory=$juceDirectory"
$actualCommit = (& git -c $safeDirectory -C $juceDirectory rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or $actualCommit -ne $juceCommit) {
    throw "JUCE revision mismatch. Expected $juceCommit, found $actualCommit"
}

if (-not (Test-Path $patchPath)) {
    throw "Missing JUCE compatibility patch: $patchPath"
}

$juceUtils = Join-Path $juceDirectory 'extras\Build\CMake\JUCEUtils.cmake'
$juceaide = Join-Path $juceDirectory 'extras\Build\juceaide\CMakeLists.txt'
$juceUtilsPatched = Select-String -Path $juceUtils -Pattern 'list(APPEND PASSTHROUGH_ARGS "-DCMAKE_MT=${CMAKE_MT}")' -SimpleMatch -Quiet
$juceaidePatched = (Select-String -Path $juceaide -Pattern 'list(APPEND PASSTHROUGH_ARGS "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")' -SimpleMatch -Quiet) -and
                  (Select-String -Path $juceaide -Pattern 'list(APPEND PASSTHROUGH_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")' -SimpleMatch -Quiet) -and
                  (Select-String -Path $juceaide -Pattern 'list(APPEND PASSTHROUGH_ARGS "-DCMAKE_RC_COMPILER=${CMAKE_RC_COMPILER}")' -SimpleMatch -Quiet) -and
                  (Select-String -Path $juceaide -Pattern 'list(APPEND PASSTHROUGH_ARGS "-DCMAKE_MT=${CMAKE_MT}")' -SimpleMatch -Quiet)

if ($juceUtilsPatched -and $juceaidePatched) {
    Write-Host 'JUCE compatibility patch already applied'
} else {
    & git -c $safeDirectory -C $juceDirectory apply --check $patchPath 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "JUCE compatibility patch is not applicable to the pinned JUCE checkout"
    }

    & git -c $safeDirectory -C $juceDirectory apply $patchPath
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to apply JUCE compatibility patch"
    }
}

Write-Host "JUCE 9.0.1 ready at $juceDirectory ($juceCommit)"
