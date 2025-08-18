local lunatest = require("tests.lunatest.lunatest")
local rps = require("RPS")


local test_memory = {}

function test_memory.test_allocate()
  local addr = rps.allocate(1000)-- sample is hopefully high enough to not have all 0's

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
  local addr = rps.allocate(1000, true)-- sample is hopefully high enough to not have all 0's

  local nonZero = false
  for _, value in ipairs(rps.readBytes(addr, 1000)) do
    if value ~= 0 then
      nonZero = true
      break
    end
  end

  lunatest.assert_false(nonZero, 'all zero values (unexpected)')
end
  

function test_memory.test_allocateCode()
  local addr = rps.allocateCode(1000)-- sample is hopefully high enough to not have all 0's

  local nonZero = false
  for _, value in ipairs(rps.readBytes(addr, 1000)) do
    if value ~= 0 then
      nonZero = true
      break
    end
  end

  lunatest.assert_true(nonZero, 'all zero values (unexpected)')
end

function test_memory.test_allocateCode_zero()
  local addr = rps.allocateCode(1000, true)-- sample is hopefully high enough to not have all 0's

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