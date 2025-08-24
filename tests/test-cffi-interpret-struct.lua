local lunatest = require("tests.lunatest.lunatest")

local test_cffi_parser_struct = {}


function test_cffi_parser_struct.test_struct()
  local cffi = require("cffi")      
  local parser = cffi.Parser:init()
  local result = parser:parse([[
    struct A {
      int a;
      int b[100];
      unsigned long address;
    };
  ]])

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

  local result = ""
  for _, token in pairs(tokens) do
    result = result .. string.format("TOKEN: type '%s', data: %s\n", token.type, token.data)
  end

  lunatest.assert_equal(27, #tokens, 0)
  lunatest.assert_equal([[
TOKEN: type 'TYPEDEF', data: typedef
TOKEN: type 'STRUCT', data: struct
TOKEN: type 'SYMBOL', data: _A
TOKEN: type 'CHARACTER', data: {
TOKEN: type 'SYMBOL', data: int
TOKEN: type 'SYMBOL', data: a
TOKEN: type 'CHARACTER', data: ;
TOKEN: type 'SYMBOL', data: int
TOKEN: type 'SYMBOL', data: b[100]
TOKEN: type 'CHARACTER', data: ;
TOKEN: type 'SYMBOL', data: unsigned
TOKEN: type 'SYMBOL', data: long
TOKEN: type 'SYMBOL', data: address
TOKEN: type 'CHARACTER', data: ;
TOKEN: type 'SYMBOL', data: unsigned
TOKEN: type 'SYMBOL', data: char
TOKEN: type 'CHARACTER', data: *
TOKEN: type 'SYMBOL', data: str
TOKEN: type 'CHARACTER', data: ;
TOKEN: type 'SYMBOL', data: unsigned
TOKEN: type 'SYMBOL', data: long
TOKEN: type 'CHARACTER', data: *
TOKEN: type 'SYMBOL', data: pointer
TOKEN: type 'CHARACTER', data: ;
TOKEN: type 'CHARACTER', data: }
TOKEN: type 'SYMBOL', data: B__
TOKEN: type 'CHARACTER', data: ;
]], result)
end

return test_cffi_parser_struct