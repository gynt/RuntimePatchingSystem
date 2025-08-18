local lunatest = require("tests.lunatest.lunatest")
local rps = require("RPS")


local test_code = {}

function test_code.test_exposeCode_cdecl()

  local code1 = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 }
  local codeAddr1 = rps.allocateCode(#code1)
  rps.writeCode(codeAddr1, code1)
  local f1 = rps.exposeCode(codeAddr1, 0, 0)

  lunatest.assert_equal(1, f1(), 0, 'f1 return value')

  rps.deallocateCode(codeAddr1)

end



function test_code.test_exposeCode_thiscall()

  local code1 = { 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08, 0x8B, 0x09, 0x0F, 0xAF, 0xC2, 0x0F, 0xAF, 0xC1, 0xC2, 0x08, 0x00 }
  local codeAddr1 = rps.allocateCode(#code1)
  rps.writeCode(codeAddr1, code1)
  local f1 = rps.exposeCode(codeAddr1, 3, 1)

  local this = rps.allocate(4)
  rps.writeInteger(this, 100)

  lunatest.assert_equal(600, f1(this, 2, 3), 0, 'f1 return value')

  rps.deallocateCode(codeAddr1)

end


function test_code.test_exposeCode_stdcall()

  local code1 = { 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08, 0x0F, 0xAF, 0xC2, 0xC2, 0x08, 0x00 }
  local codeAddr1 = rps.allocateCode(#code1)
  rps.writeCode(codeAddr1, code1)
  local f1 = rps.exposeCode(codeAddr1, 2, 2)

  lunatest.assert_equal(200, f1(10, 20), 0, 'unexpected return value')

  rps.deallocateCode(codeAddr1)

end

return test_code