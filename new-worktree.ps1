<#
.SYNOPSIS
  Create a short-path git worktree that is immediately buildable.

.DESCRIPTION
  `git worktree add` gives you a checkout of the last COMMIT -- it never carries the untracked
  files this checkout needs:
    - CMakeUserPresets.json (gitignored; no `local-debug` preset without it)
    - CLAUDE.md / AGENTS.md (in .git/info/exclude, so untracked -- a fresh worktree has no
      agent instructions at all)
  This script creates the worktree, copies those across, and prints the path.

  It also gives the worktree its OWN plugin deploy target (a slim cadwork userprofil under
  -ProfileRoot, recorded in the worktree's .cw-userprofile marker). Without that, two worktrees
  building the same example post-build-copy over the same <userprofil>\3d\API.x64\<target> folder,
  the e2e harness tests whichever built last, and a build fails outright whenever a running cadwork
  holds the DLL open. Distinct example target names (cw_api3d_hello vs cw_api3d_charts) already
  coexist in one profile; worktrees still isolate two checkouts of the same example.

  Worktrees are rooted at a short path on purpose -- a long nested path blows MAX_PATH during the
  build and surfaces as a misleading C1083 deep in a header, not as a path-length error.

.PARAMETER Key
  Short identifier for the worktree (e.g. a ticket key or feature name). Used for the worktree
  directory name, the branch name, and the profile-name suffix.

.PARAMETER Base
  Git ref to branch the worktree from. Default: HEAD.

.PARAMETER Root
  Parent directory for worktrees. Default: D:\wt\cw-api3d.

.PARAMETER ProfileRoot
  Parent directory holding the cadwork profiles the worktree's slim profile is created in.
  Default: D:\cadwork.

.PARAMETER NoLocalProfile
  Skip the slim profile + marker; the worktree then deploys into the machine-wide userprofil like
  an unconfigured checkout.

.EXAMPLE
  PS> .\new-worktree.ps1 -Key feature-x
  Creates D:\wt\cw-api3d\feature-x on branch wt-feature-x, copies the untracked build/agent config,
  and gives it its own plugin deploy target (D:\cadwork\userprofil_2026_feature-x).
#>

param(
  [Parameter(Mandatory = $true)]
  [string] $Key,
  [string] $Base = 'HEAD',
  [string] $Root = 'D:\wt\cw-api3d',
  [string] $ProfileRoot = 'D:\cadwork',
  [switch] $NoLocalProfile
)

$ErrorActionPreference = 'Stop'

# Untracked files git will never carry into a worktree, but every checkout needs.
$UntrackedFilesToCopy = @('CMakeUserPresets.json', 'CLAUDE.md', 'AGENTS.md')

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $repoRoot

$worktreePath = Join-Path $Root $Key
if (Test-Path $worktreePath) {
  Write-Host "ERROR: $worktreePath already exists -- pick a different -Key, or remove it first (git worktree remove)." -ForegroundColor Red
  exit 1
}

$branch = "wt-$Key"
Write-Host "Creating worktree at $worktreePath on branch $branch (base: $Base)..." -ForegroundColor Cyan
git worktree add -b $branch $worktreePath $Base
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

foreach ($fileName in $UntrackedFilesToCopy) {
  $source = Join-Path $repoRoot $fileName
  if (Test-Path -LiteralPath $source) {
    Copy-Item -LiteralPath $source -Destination (Join-Path $worktreePath $fileName) -Force
    Write-Host "Copied $fileName." -ForegroundColor DarkGray
  }
  else {
    Write-Host "Skipped $fileName (absent here)." -ForegroundColor DarkGray
  }
}

# --- This worktree's own plugin deploy target (slim cadwork profile + marker) ---
# Deliberately last: a failure here (no template profile on this machine) must not look like the
# worktree itself failed -- the worktree above is already usable.
if (-not $NoLocalProfile) {
  # This checkout's copy, not the worktree's: the worktree holds the last COMMIT, so a script that
  # is new or locally edited here would be missing (or stale) over there.
  $localProfileScript = Join-Path $repoRoot 'build-scripts\new-local-profile.ps1'
  if (Test-Path -LiteralPath $localProfileScript) {
    Write-Host ""
    try {
      & $localProfileScript -Name $Key -RepoRoot $worktreePath -Root $ProfileRoot
    }
    catch {
      # $ErrorActionPreference is 'Stop' here, so without this catch a missing template profile
      # would abort the whole script and read as "the worktree failed" -- it did not.
      Write-Host "WARN: could not create the local cadwork profile ($($_.Exception.Message))." -ForegroundColor Yellow
      Write-Host "      This worktree will deploy into the machine-wide userprofil." -ForegroundColor Yellow
    }
  }
  else {
    Write-Host "WARN: $localProfileScript not found -- no per-worktree plugin deploy." -ForegroundColor Yellow
  }
}

Write-Host ""
Write-Host "Worktree ready: $worktreePath" -ForegroundColor Green
Write-Host "Build it with:" -ForegroundColor Green
Write-Host "  cd `"$worktreePath`"; cmake --preset local-debug; cmake --build out/build/local-debug"
