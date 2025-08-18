local common = require("common")

local rps = common.rps
local lunatest = common.lunatest

local test_string = {}

function test_string.test_string()
  local testString = "hello world!"
  local stringAddr1 = rps.allocate(testString:len() + 1, true)
  rps.writeString(stringAddr1, testString)

  local result = rps.readString(stringAddr1)

  lunatest.assert_equal(testString, result, "", "did not receive exact same string")
end

function test_string.test_string_len()
  local testString = "hello world!"
  local stringAddr1 = rps.allocate(testString:len() + 1, true)
  rps.writeString(stringAddr1, testString)

  local result = rps.readString(stringAddr1, testString:len() + 1)

  lunatest.assert_equal(testString:len() + 1, result:len(), "", "did not receive exact same string")
end

return test_string