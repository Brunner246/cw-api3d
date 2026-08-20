@echo off
REM Thin wrapper around new-worktree.ps1 -- see that file for parameters/behavior.
REM Usage:
REM   new-worktree.cmd -Key feature-x
REM   new-worktree.cmd -Key feature-x -Base origin/main -Root D:\wt\cw-api3d

setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0new-worktree.ps1" %*
exit /b %ERRORLEVEL%
