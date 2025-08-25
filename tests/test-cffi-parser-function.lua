local lunatest = require("tests.lunatest.lunatest")

local test_cffi_parser_function = {}


function test_cffi_parser_function.test_function_declaration()
  local cffi = require("cffi")      
  cffi.options.VERBOSE = false
  local parser = cffi.Parser:init()
  local tokens = parser:tokens([[
    void (__thiscall * f)(int a, unsigned char * b);
  ]])

end


return test_cffi_parser_function