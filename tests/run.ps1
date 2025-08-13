Invoke-WebRequest https://www.nuget.org/api/v2/package/lua/5.4.6 -OutFile lua-5.4.6.zip
Expand-Archive -Path .\lua-5.4.6.zip -DestinationPath lua
Remove-Item -Path lua-5.4.6.zip
Move-Item -Path .\lua\build\native\bin\Win32\v143\Release\* -Destination .
Remove-Item -Recurse -Path lua\

Copy-Item -Path Release\*.dll -Destination .

Remove-Item .\tests\run.lua -ErrorAction SilentlyContinue

@"
local lunatest = require('tests.lunatest.lunatest')
local rps = require("RPS")

"@ | Add-Content -Path "tests\run.lua"
Get-Item -Path "tests\test-*.lua" | Resolve-Path -Relative | ForEach-Object {$_.Substring(2)} | ForEach-Object {$_.Substring(0, $_.Length - 4)} | ForEach-Object {"lunatest.suite('$_')"} | ForEach-Object {$_.Replace('\', '.')} |  Add-Content -Path "tests\run.lua"
"lunatest.run()" | Add-Content -Path "tests\run.lua"


lua.exe tests\run.lua -v

Remove-Item *.dll
Remove-Item *.exe
Remove-Item .\tests\run.lua