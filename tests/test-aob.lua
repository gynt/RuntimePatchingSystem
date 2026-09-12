local lunatest = require("tests.lunatest.lunatest")
local rps = require("RPS")
local test_aob = {}

function test_aob.test_overlapping_main_image_results()
  local first, second = rps.scanForAOBInMainModule('? ? ?')
  lunatest.assert_true(type(first) == 'number')
  lunatest.assert_equal(first + 1, second)
end

function test_aob.test_inclusive_exact_and_short_ranges()
  local first = rps.scanForAOBInMainModule('? ? ?')
  lunatest.assert_equal(first, rps.scanForAOB('? ? ?', first, first + 2))
  lunatest.assert_nil(rps.scanForAOB('? ? ?', first, first + 1))
end

function test_aob.test_missing_result_shape()
  local first, second = rps.scanForAOBInMainModule(string.rep('37 9B E1 46 02 AF DC 85 ', 32))
  lunatest.assert_nil(first)
  lunatest.assert_nil(second)
end

function test_aob.test_invalid_pattern()
  lunatest.assert_false(pcall(rps.scanForAOBInMainModule, 'ZZ'))
end

return test_aob
