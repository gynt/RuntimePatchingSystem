local lunatest = require("tests.lunatest.lunatest")

local test_success = {}

function test_success.test_success()
  lunatest.assert_true(true)
end


return test_success