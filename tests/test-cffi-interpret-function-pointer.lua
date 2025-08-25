local lunatest = require("tests.lunatest.lunatest")

local test_cffi_interpret_function_pointer = {}


function test_cffi_interpret_function_pointer.test_function_pointer()
  local cffi = require("cffi")
  cffi.options.VERBOSE = false
  local parser = cffi.Parser:init()
  local interpretation = parser:parse([[
    void (__thiscall * f)(int a, unsigned char * b);
    void (* f2)(int a, unsigned char * b);
  ]])

  local result = interpretation.elements

  lunatest.assert_equal(2, #result, 0)
  lunatest.assert_equal("void", result[1].typeInfo.name)
  lunatest.assert_equal(1, result[1].fpointers)
  lunatest.assert_equal("f", result[1].name)
  lunatest.assert_equal("__thiscall", result[1].callingConvention)

  lunatest.assert_equal(2, #result[1].arguments)
  
  lunatest.assert_equal("b", result[1].arguments[2].name)
  
  lunatest.assert_equal("unsigned char", result[1].arguments[2].typeInfo.name)
end

return test_cffi_interpret_function_pointer