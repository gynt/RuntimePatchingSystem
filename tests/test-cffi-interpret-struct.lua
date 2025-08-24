local lunatest = require("tests.lunatest.lunatest")

local test_cffi_parser_struct = {}


function test_cffi_parser_struct.test_struct()
  local cffi = require("cffi")      
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

  for _, field in ipairs(result[1].fields) do
    for k, v in pairs(field) do print(k, v) end
  end

  lunatest.assert_equal("int", result[1].fields[1].typeName)
  lunatest.assert_equal("b", result[1].fields[2].fieldName)
end


function test_cffi_parser_struct.test_typedef_struct()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local tokens = parser:tokens([[
    typedef struct _A {
      int a;
      int b[100];
      unsigned long address;
      unsigned char * str;
      unsigned long* pointer;
    } B__;
  ]])

end

return test_cffi_parser_struct