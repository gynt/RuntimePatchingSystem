local lunatest = require("tests.lunatest.lunatest")

local test_cffi_parser_struct = {}


function test_cffi_parser_struct.test_struct()
  local cffi = require("cffi")
  cffi.options.VERBOSE = false
  local parser = cffi.Parser:init()
  local interpretation = parser:parse([[
    struct A {
      int a;
      int b[100];
      unsigned long address;
    };
  ]])

  local result = interpretation.elements

  lunatest.assert_equal(#result, 1, 0)
  lunatest.assert_equal("struct", result[1].type)
  lunatest.assert_equal(3, #result[1].fields)

  lunatest.assert_equal("int", result[1].fields[1].fieldTypeName)
  lunatest.assert_equal("b", result[1].fields[2].name)
end


function test_cffi_parser_struct.test_typedef_struct()
  local cffi = require("cffi")
  cffi.options.VERBOSE = false
  local parser = cffi.Parser:init()
  local interpretation = parser:parse([[
    typedef struct _A {
      int a;
      int b[100];
      unsigned long address;
      unsigned char * str;
      unsigned long* * pointer;
    } B__;
  ]])

  
  local result = interpretation.elements

  lunatest.assert_equal(#result, 1, 0)
  lunatest.assert_equal("typedef", result[1].type)
  lunatest.assert_equal("struct", result[1].typed.type)
  lunatest.assert_equal(5, #result[1].typed.fields)
  lunatest.assert_equal("unsigned long", result[1].typed.fields[5].fieldTypeName)
  lunatest.assert_equal("str", result[1].typed.fields[4].name)
  
  lunatest.assert_equal(2,  result[1].typed.fields[5].pointers, 0)
end

return test_cffi_parser_struct