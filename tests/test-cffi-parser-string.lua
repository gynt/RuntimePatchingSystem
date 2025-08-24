local lunatest = require("tests.lunatest.lunatest")

local test_cffi_parser_string = {}


function test_cffi_parser_string.test_double_quoted()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local tokens = parser:tokens('"Hello world!"')

  lunatest.assert_equal(1, #tokens, 0)

  local token = tokens[1]

  lunatest.assert_equal(token.type, "DOUBLE_QUOTED_STRING")
  lunatest.assert_equal(token.data, 'Hello world!')
end

function test_cffi_parser_string.test_single_quoted()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local tokens = parser:tokens("'Hello world!'")

  lunatest.assert_equal(1, #tokens, 0)

  local token = tokens[1]

  lunatest.assert_equal(token.type, "SINGLE_QUOTED_STRING")
  lunatest.assert_equal(token.data, 'Hello world!')
end

return test_cffi_parser_string