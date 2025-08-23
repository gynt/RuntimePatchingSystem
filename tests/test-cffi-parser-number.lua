local lunatest = require("tests.lunatest.lunatest")

local test_cffi_parser_number = {}

function test_cffi_parser_number.test_integer()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local tokens = parser:parse('9032320')

  lunatest.assert_equal(1, #tokens, 0)

  local token = tokens[1]

  lunatest.assert_equal("INTEGER", token.type)
  lunatest.assert_equal(9032320, token.data)
end

function test_cffi_parser_number.test_integer_hexadecimal()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local tokens = parser:parse("0xDEADBEED")

  lunatest.assert_equal(1, #tokens, 0)

  local token = tokens[1]

  lunatest.assert_equal(token.type, "HEXADECIMAL")
  lunatest.assert_equal(token.data, 0xDEADBEED)
end

function test_cffi_parser_number.test_float()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local tokens = parser:parse("'Hello world!'")

  lunatest.assert_equal(1, #tokens, 0)

  local token = tokens[1]

  lunatest.assert_equal(token.type, "SINGLE_QUOTED_STRING")
  lunatest.assert_equal(token.data, 'Hello world!')
end

function test_cffi_parser_number.test_long()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local tokens = parser:parse("100L")

  lunatest.assert_equal(1, #tokens, 0)

  local token = tokens[1]

  lunatest.assert_equal(token.type, "LONG")
  lunatest.assert_equal(token.data, 100)
end

return test_cffi_parser_number