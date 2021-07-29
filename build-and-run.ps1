./build.ps1

if ($LastExitCode -ne 0) {
  Write-Error "Build Failed!"
} else {
  ./run.ps1
}