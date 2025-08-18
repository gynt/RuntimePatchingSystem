local common = require("common")

local rps = common.rps
local lunatest = common.lunatest

local test_memory = {}

function test_memory.test_allocate()
  local addr = rps.allocate(1000)-- sample is hpoefully high enough to not have all 0's

  local nonZero = false
  for _, value in ipairs(rps.readBytes(addr, 1000)) do
    if value ~= 0 then
      nonZero = true
      break
    end
  end

  lunatest.assert_true(nonZero, 'all zero values (unexpected)')
end

function test_memory.test_allocate_zero()
  local addr = rps.allocate(1000, true)-- sample is hpoefully high enough to not have all 0's

  local nonZero = false
  for _, value in ipairs(rps.readBytes(addr, 1000)) do
    if value ~= 0 then
      nonZero = true
      break
    end
  end

  lunatest.assert_false(nonZero, 'all zero values (unexpected)')
end

return test_memory