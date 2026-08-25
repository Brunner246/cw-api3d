<#
.SYNOPSIS
  Create a slim, checkout-local cadwork userprofil and point this checkout's plugin deploy at it.

.DESCRIPTION
  Every build post-build-copies cw_api3d_charts.dll into ONE shared directory --
  <userprofil>\3d\API.x64\cw_api3d_charts (see cmake/cadwork_deploy.cmake). With several git
  worktrees that means they overwrite each other's DLL, the e2e harness tests whichever worktree
  built last, and a build fails outright whenever a running cadwork holds that DLL open.

  This script gives a checkout its own profile, hence its own plugin directory:

      <Root>\userprofil_<Year>_<Name>\3d\API.x64\cw_api3d_charts

  and writes the profile path into `<RepoRoot>\.cw-userprofile`, the gitignored marker that
  cmake/cadwork_deploy.cmake and tests/e2e/cadwork_paths.py both read. From then on this checkout
  builds and e2e-tests against its own binaries with no env var to remember.

  The profile is a whitelist copy of the real profile (<Root>\userprofil_<Year>) -- only the handful
  of files cadwork needs to start, no project data. The name deliberately keeps the
  `userprofil_<year>` prefix: cadwork rewrites any profile path that does not contain it.

  Idempotent -- re-running only fills in what is missing (and rewrites the marker).

  NOTE: nothing here repoints cadwork's machine-wide active profile (CADWORK_USP lives in the
  registry). The e2e harness passes this profile to the cadwork it launches via the environment.

.PARAMETER Name
  Short identifier for this checkout (e.g. the worktree key). Becomes the profile-name suffix.

.PARAMETER RepoRoot
  The checkout that gets the marker file. Default: the repository this script lives in.

.PARAMETER Root
  Parent directory holding the cadwork profiles. Default: D:\cadwork.

.PARAMETER Year
  cadwork profile year. Default: derived from the registry's CADWORK_USP, else 2026.

.EXAMPLE
  PS> .\build-scripts\new-local-profile.ps1 -Name main
.EXAMPLE
  PS> .\build-scripts\new-local-profile.ps1 -Name feature-x -RepoRoot D:\wt\cw-api3d\feature-x
#>

param(
  [Parameter(Mandatory = $true)] [string] $Name,
  [string] $RepoRoot,
  [string] $Root = 'D:\cadwork',
  [string] $Year
)

$ErrorActionPreference = 'Stop'

# Keep in sync with cmake/cadwork_deploy.cmake and tests/e2e/cadwork_paths.py.
$MarkerName = '.cw-userprofile'
$PluginName = 'cw_api3d'

# The slim set of profile entries cadwork needs to start. Everything else it recreates on demand.
# Paths are relative to the profile root; a trailing '*' means "glob, may match nothing".
$SlimProfileFiles = @(
  '3d\DBE\Materials.db3',
  '3d\DBE\MultiLayerSet.db3',
  '3d\DBE\stdElement.db3',
  '3d\DBE\USERP.SDB',
  '3d\Hiddenface.opt',
  '3d\no_intro_*',
  'AppData\AppNotification\AppNotificationSettings.ini'
)

# Directories cadwork expects to exist even when empty.
$SlimProfileDirs = @(
  '3d\DBE',
  '3d\Machine\Panel Prefabrication',
  'AppData\AppNotification'
)

if (-not $RepoRoot) {
  $RepoRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
}

if ($Name -match '[\\/:*?"<>|]') {
  throw "-Name '$Name' contains a character that is not valid in a directory name."
}

if (-not $Year) {
  $registryUsp = (Get-ItemProperty 'HKCU:\Software\cadwork Informatik\ENV' -Name 'CADWORK_USP' -ErrorAction SilentlyContinue).CADWORK_USP
  if ($registryUsp -and (Split-Path -Leaf $registryUsp) -match '(\d{4})') { $Year = $Matches[1] }
  if (-not $Year) { $Year = '2026' }
}

$templateProfile = Join-Path $Root "userprofil_$Year"
$targetProfile = Join-Path $Root "userprofil_${Year}_$Name"
$pluginDir = Join-Path $targetProfile "3d\API.x64\$PluginName"

Write-Host "Local cadwork profile for '$Name'" -ForegroundColor Cyan
Write-Host "  profile:  $targetProfile"
Write-Host "  plugin:   $pluginDir"
Write-Host "  template: $templateProfile"

if (-not (Test-Path -LiteralPath $templateProfile)) {
  throw ("Template profile not found: $templateProfile. Pass -Root/-Year to point at the real " +
    "cadwork userprofil this machine uses.")
}

# --- Slim profile ----------------------------------------------------------------------------

foreach ($relative in $SlimProfileDirs + @("3d\API.x64\$PluginName")) {
  New-Item -ItemType Directory -Path (Join-Path $targetProfile $relative) -Force | Out-Null
}

foreach ($relative in $SlimProfileFiles) {
  $sources = @(Get-ChildItem -Path (Join-Path $templateProfile $relative) -File -ErrorAction SilentlyContinue)
  if ($sources.Count -eq 0) {
    Write-Host "  skipped (absent in template): $relative" -ForegroundColor DarkGray
    continue
  }
  $destinationDir = Join-Path $targetProfile (Split-Path -Parent $relative)
  foreach ($source in $sources) {
    $destination = Join-Path $destinationDir $source.Name
    if (Test-Path -LiteralPath $destination) {
      Write-Host "  kept: $relative" -ForegroundColor DarkGray
      continue
    }
    Copy-Item -LiteralPath $source.FullName -Destination $destination -Force
    Write-Host "  copied: $(Join-Path (Split-Path -Parent $relative) $source.Name)" -ForegroundColor DarkGray
  }
}

# --- Marker ----------------------------------------------------------------------------------

$markerPath = Join-Path $RepoRoot $MarkerName
$markerLines = @(
  '# Cadwork userprofil this checkout deploys and e2e-tests against (gitignored).',
  '# Read by cmake/cadwork_deploy.cmake and tests/e2e/cadwork_paths.py; CADWORK_USP overrides it.',
  $targetProfile
)
# WriteAllLines rather than Set-Content -Encoding UTF8: under Windows PowerShell 5.1 (which
# new-worktree.cmd uses) that switch prepends a BOM, and a BOM would ride along into the value the
# readers parse. This writes UTF-8 without one on both 5.1 and 7.
[System.IO.File]::WriteAllLines($markerPath, $markerLines)

Write-Host ""
Write-Host "Wrote $markerPath" -ForegroundColor Green
Write-Host "This checkout now builds and e2e-tests against $pluginDir" -ForegroundColor Green
Write-Host "  build: re-run 'cmake --preset local-debug' once, then the POST_BUILD deploy follows the marker"
