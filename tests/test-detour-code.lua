local lunatest = require("tests.lunatest.lunatest")
local rps = require("RPS")

local test_detour_code = {}

function test_detour_code.test_detourCode()
  local func1_code = { 0xB8, 0x02, 0x00, 0x00, 0x00, 0xBA, 0x03, 0x00, 0x00, 0x00, 0x0F, 0xAF, 0xC2, 0xC3 }
  local func1_address = rps.allocateCode(#func1_code)
  rps.writeCode(func1_address, func1_code)

  local func1 = rps.exposeCode(func1_address, 0, 0) -- this call with 3 arguments

  lunatest.assert_equal(6, func1())

  rps.detourCode(function(registers)
    registers.EAX = 100
    return registers
  end, func1_address + 5, 5)

  lunatest.assert_equal(300, func1())

end

return test_detour_code