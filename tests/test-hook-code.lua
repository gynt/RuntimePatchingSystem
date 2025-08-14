local lunatest = require("tests.lunatest.lunatest")
local rps = require("RPS")

local test_hook_code = {}

function test_hook_code.test_hookCode()
  local func1_code = { 0x8B, 0x09, 0x8B, 0x44, 0x24, 0x04, 0x0F, 0xAF, 0xC1, 0x8B, 0x54, 0x24, 0x08, 0x0F, 0xAF, 0xC2, 0xC2, 0x08, 0x00 }
  local func1_address = rps.allocateCode(#func1_code)
  rps.writeCode(func1_address, func1_code)
  local this = rps.allocate(4)
  rps.writeInteger(this, 100)
  local func1 = rps.exposeCode(func1_address, 3, 1) -- this call with 3 arguments

  lunatest.assert_equal(600, func1(this, 2, 3))

  local o_func1
  o_func1 = rps.hookCode(function(thiss, a, b)
    rps.writeInteger(thiss, rps.readInteger(thiss) / 10)
    return o_func1(thiss, a, b)
  end, func1_address, 3, 1, 6)

  lunatest.assert_equal(600, o_func1(this, 2, 3))

  lunatest.assert_equal(60, func1(this, 2, 3))

end

return test_hook_code