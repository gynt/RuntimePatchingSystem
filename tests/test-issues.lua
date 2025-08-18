local common = require("common")

local rps = common.rps
local lunatest = common.lunatest

local test_issues = {}

function test_issues.test_issue_05()
  local addr = rps.allocate(1001, true)
  rps.writeString(addr, "hello world!")
  
  local s1 = rps.readString(addr, 1001)
  if s1:len() ~= 1001 then error(s1:len()) end

  rps.writeString(addr, s1)

  local s2 = rps.readString(addr, 1001)

  lunatest.assert_equal(s1, s2, "", 'strings not equal, issue 05')
end

return test_issues