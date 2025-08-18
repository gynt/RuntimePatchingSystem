param ($Cleanup = $true)
$ErrorActionPreference = 'Stop'

$installRPS = $null -eq (Get-Item "RPS.dll" -ErrorAction SilentlyContinue)

$installLua = $false
$command = Get-Command "lua.exe" -ErrorAction SilentlyContinue
if (($null -eq $command) -or ($command.Version.ToString() -ne "5.4.6.0")) {
  $installLua = $true
}

if ($installLua) {
  Invoke-WebRequest https://www.nuget.org/api/v2/package/lua/5.4.6 -OutFile lua-5.4.6.zip
  Expand-Archive -Path .\lua-5.4.6.zip -DestinationPath lua
  Remove-Item -Path lua-5.4.6.zip
  Move-Item -Path .\lua\build\native\bin\Win32\v143\Release\* -Destination .
  Remove-Item -Recurse -Path lua\

  $command = Get-Command ".\lua.exe"
}


if ($installRPS) {
  Copy-Item -Path Release\*.dll -Destination .
}

Remove-Item .\tests\run.lua -ErrorAction SilentlyContinue

@"
local lunatest = require('tests.lunatest.lunatest')
local rps = require("RPS")

"@ | Add-Content -Path "tests\run.lua"
Get-Item -Path "tests\test-*.lua" | Resolve-Path -Relative | ForEach-Object {$_.Substring(2)} | ForEach-Object {$_.Substring(0, $_.Length - 4)} | ForEach-Object {"lunatest.suite('$_')"} | ForEach-Object {$_.Replace('\', '.')} |  Add-Content -Path "tests\run.lua"
"lunatest.run()" | Add-Content -Path "tests\run.lua"

& "$($command.Path)" tests\run.lua -v

if ($Cleanup) {
  Remove-Item .\tests\run.lua

  if ($installRPS) {
    Remove-Item RPS.dll
    Remove-Item lua.dll
  }
  if ($installLua) {
    Remove-Item lua.exe
  }
}

