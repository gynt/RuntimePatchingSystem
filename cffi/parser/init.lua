
local TOKEN_TYPES = require("cffi.parser.tokens").TOKEN_TYPES
local ParseState = require("cffi.parser.lexer")
local namespace = require("cffi.namespace")
local GLOBAL = namespace.GLOBAL
local Struct = require("cffi.model.struct")
local options = require("cffi.options")

---@class StackElement
---@field type string

---@class Parser
local Parser = {}

local PARSER_DEFAULTS = {
  namespace = GLOBAL,
}

---@return Parser instance
function Parser:init(opts)
  opts = opts or PARSER_DEFAULTS

  local o = {
    namespace = opts.namespace or PARSER_DEFAULTS.namespace,
  }
  setmetatable(o, self)
  self.__index = self

  ---@diagnostic disable-next-line: return-type-mismatch
  return o
end

local FMT_STRING_PARSE_ERROR_EXPECT = "expected %s but received %s: '%s'"

function Parser:raise_parse_error(fmt, ...)
  print(string.format("index: %s", self.state.index))
  for _, token in ipairs(self.state.tokens) do
    print(string.format("%s %s %s", _, token.type, token.data)) 
  end
  error(string.format(fmt, ...))
end

function Parser:tokens(str)
  self.lex = ParseState:init({text = str})

  local tokens = self.lex:tokens()

  return tokens
end



function Parser:_next_token()
  self.state.index = self.state.index + 1
  return self.state.tokens[self.state.index]
end

function Parser:_peek_next_token(which)
  return self.state.tokens[self.state.index + (which or 1)]
end

function Parser:_current_token()
  return self:_peek_next_token(0)
end

local TYPE_EXTENDERS = {
  ["unsigned"] = true,
  ["union"] = true,
  ["struct"] = true,
  ["enum"] = true,
}

function Parser:_parse_type_info()
  local info = {}

  -- unsigned int **

  local fieldTypeNameToken = self:_current_token() -- unsigned
  if fieldTypeNameToken.type ~= TOKEN_TYPES.SYMBOL then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.SYMBOL, fieldTypeNameToken.type, fieldTypeNameToken.data)
  end

  if TYPE_EXTENDERS[fieldTypeNameToken.data] == true then
    info.name = fieldTypeNameToken.data -- unsigned
    local fieldTypeNameToken2 = self:_next_token() -- int
    if fieldTypeNameToken2.type ~= TOKEN_TYPES.SYMBOL then
      self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.SYMBOL, fieldTypeNameToken2.type, fieldTypeNameToken2.data)
    end
    info.name = info.name .. " " .. fieldTypeNameToken2.data -- unsigned int
  else
    info.name = fieldTypeNameToken.data
  end

  info.pointers = 0
  local nextToken = self:_next_token() -- *
  while nextToken.type == TOKEN_TYPES.CHARACTER and nextToken.data == "*" do
    info.pointers = info.pointers + 1
    
    -- nextToken = self:_peek_next_token() -- *, second round: a
    -- if nextToken.type ~= TOKEN_TYPES.CHARACTER or nextToken.data ~= "*" then
    --   break
    -- end
    nextToken = self:_next_token() -- *
  end

  return info
end

function Parser:_previous_token()
  self.state.index = self.state.index - 1
  return self:_current_token()
end

function Parser:_parse_function_pointer(field)
  -- void (__thiscall * f)(int a, unsigned char * b);
  local parenthesisToken = self:_current_token() -- (
  if parenthesisToken.type ~= TOKEN_TYPES.PARENTHESIS_OPEN then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.PARENTHESIS_OPEN, parenthesisToken.type, parenthesisToken.data)
  end

  local nextToken = self:_next_token() -- __thiscall
  if nextToken.type == TOKEN_TYPES.SYMBOL then
    field.callingConvention = nextToken.data
    nextToken = self:_next_token() -- *
  end

  -- *
  if nextToken.type ~= TOKEN_TYPES.CHARACTER or nextToken.data ~= "*" then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, "'*'", nextToken.type, nextToken.data)
  end

  -- *
  while nextToken.type == TOKEN_TYPES.CHARACTER and nextToken.data == "*" do
    if field.fpointers == nil then field.fpointers = 0 end
    field.fpointers = field.fpointers + 1
    nextToken = self:_next_token() -- f
  end

  -- f
  if nextToken.type == TOKEN_TYPES.SYMBOL then
    field.name = nextToken.data
  end

  nextToken = self:_next_token() -- )
  if nextToken.type ~= TOKEN_TYPES.PARENTHESIS_CLOSE then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.PARENTHESIS_CLOSE, nextToken.type, nextToken.data)
  end

  local nextToken = self:_next_token() -- (
  if nextToken.type ~= TOKEN_TYPES.PARENTHESIS_OPEN then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.PARENTHESIS_OPEN, nextToken.type, nextToken.data)
  end

  field.arguments = {}
  nextToken = self:_next_token() -- int
  while self:_current_token().type ~= TOKEN_TYPES.PARENTHESIS_CLOSE do
    local argument = {}
    table.insert(field.arguments, argument)

    argument.typeInfo = self:_parse_type_info() -- a
    nextToken = self:_current_token() -- a
    if nextToken.type == TOKEN_TYPES.SYMBOL then
      argument.name = nextToken.data -- a
      nextToken = self:_next_token() -- , or )
      if nextToken.type == TOKEN_TYPES.CHARACTER and nextToken.data == "," then
        self:_next_token() -- consume comma
      elseif nextToken.type == TOKEN_TYPES.PARENTHESIS_CLOSE then
        self:_next_token() -- consume parenthesis close
        break
      else
        self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, "',' or ')'", nextToken.type, nextToken.data)
      end
    elseif nextToken.type == TOKEN_TYPES.PARENTHESIS_CLOSE then
      self:_next_token() -- consume parenthesis close
      break
    elseif nextToken.type == TOKEN_TYPES.CHARACTER and nextToken.data == "," then
      self:_next_token() -- consume comma
    else
      self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, "','", nextToken.type, nextToken.data)
    end
    
  end

  local finalToken = self:_current_token()
  if finalToken.type ~= TOKEN_TYPES.CHARACTER or finalToken.data ~= ";" then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, "';'", finalToken.type, finalToken.data)
  end

  self:_next_token() -- consume ;

  return field
end

function Parser:_parse_data(field)
  -- unsigned int ** a[100][200];
  local nameToken = self:_current_token() -- a

  if nameToken.type ~= TOKEN_TYPES.SYMBOL then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.SYMBOL, nameToken.type, nameToken.data)
  end
  field.name = nameToken.data

  local nextToken = self:_next_token() -- [100]
  while nextToken.type == TOKEN_TYPES.ARRAY_DIM do
    if field.typeInfo.array_dim == nil then field.typeInfo.array_dim = {} end
    table.insert(field.typeInfo.array_dim, nextToken.data)
    nextToken = self:_next_token() -- [200] / ;
  end

  local colonToken = nextToken
  if colonToken.type ~= TOKEN_TYPES.CHARACTER or colonToken.data ~= ";" then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, "';'", colonToken.type, colonToken.data)
  end

  self:_next_token() -- ?

  return field
end

function Parser:_parse_variable()
  local field = {}
  
  field.typeInfo = self:_parse_type_info()

  if self:_current_token().type == TOKEN_TYPES.PARENTHESIS_OPEN then
    return self:_parse_function_pointer(field)
  end

  return self:_parse_data(field)
end

---@class _parse_struct_Params
---@field start "struct"|"name"

function Parser:_parse_struct()
  -- struct A { unsigned int ** a[100][200]; };
  local structToken = self:_current_token() -- struct
  if structToken.type ~= TOKEN_TYPES.STRUCT then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.STRUCT, structToken.type, structToken.data)
  end

  local stack = self.state.stack
  local struct = {
    type = "struct",
    fields = {},
  }
  local stackPosition = table.insert(stack, struct)

  local tokenName = self:_next_token() -- A
  if tokenName.type == TOKEN_TYPES.CURLY_BRACKET_OPEN then
    error("anonymous structs are not implemented")
  end

  if tokenName.type ~= TOKEN_TYPES.SYMBOL then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.SYMBOL, tokenName.data, tokenName.type)
  end

  struct.name = tokenName.data
  
  local curlyBraceOpen = self:_next_token() -- {
  if curlyBraceOpen.type ~= TOKEN_TYPES.CURLY_BRACKET_OPEN then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.CURLY_BRACKET_OPEN, curlyBraceOpen.data, curlyBraceOpen.type)
  end

  local fieldCounter = 0
  local nextToken = self:_next_token() -- unsigned
  while self:_current_token().type ~= TOKEN_TYPES.CURLY_BRACKET_CLOSE do
    fieldCounter = fieldCounter + 1

    local field = self:_parse_variable() -- }
    table.insert(struct.fields, field)
    
  end

  local curlyBracketCloseToken = self:_current_token() -- }
  if curlyBracketCloseToken.type == TOKEN_TYPES.CURLY_BRACKET_CLOSE then
    nextToken = self:_next_token() -- consume the bracket: ?
    table.remove(self.state.stack, stackPosition)
  else
    error("unfinished struct")
  end

  if nextToken.type == TOKEN_TYPES.SYMBOL then
    -- leave this for a typedef parser...
    if self:_peek_next_token().type ~= TOKEN_TYPES.CHARACTER or self:_peek_next_token().data ~= ";" then
      error()
    end
  else
    if nextToken.type ~= TOKEN_TYPES.CHARACTER or nextToken.data ~= ";" then
      error()
    end
    self:_next_token() -- consume the ';'
  end

  -- if nextToken.type ~= TOKEN_TYPES.CHARACTER or nextToken.data ~= ";" then
  --   self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, "';'", nextToken.type, nextToken.data)
  -- end

  -- self:_next_token() -- ?

  return struct
end

function Parser:_parse_typedef()
  -- typedef struct A {unsigned int ** a[100];} _B;
  local typedefToken = self:_current_token() -- typedef
  if typedefToken.type ~= TOKEN_TYPES.TYPEDEF then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, TOKEN_TYPES.TYPEDEF, typedefToken.type, typedefToken.data)
  end

  local typedef = {
    type = "typedef",
    typed = nil,
    name = nil,
  }

  local nextToken = self:_next_token() -- struct

  if nextToken.type == TOKEN_TYPES.STRUCT then
    local struct = self:_parse_struct() -- => _B
    typedef.typed = struct
  else
    error("not implemented yet")
  end

  local typeDefNameToken = self:_current_token() -- _B
  if typeDefNameToken.type ~= TOKEN_TYPES.SYMBOL then error(typeDefNameToken.data) end
  typedef.name = typeDefNameToken.data

  local finishToken = self:_next_token() -- ;
  if finishToken.type ~= TOKEN_TYPES.CHARACTER or finishToken.data ~= ';' then
    self:raise_parse_error(FMT_STRING_PARSE_ERROR_EXPECT, "';'", finishToken.type, finishToken.data)
  end

  self:_next_token() -- ?

  return typedef
end

function Parser:interpret_tokens(tokens)
  local interpretation = {
    names = {}, -- new names
    elements = {},
  }
  self.state = {
    tokens = tokens,
    stack = {},
    index = 1, -- uninitialized
    interpretation = interpretation,
  }
  local state = self.state
  ---@type table<StackElement>
  local stack = state.stack

  while state.index <= #tokens do
    local token = self:_current_token() -- first time selects 1
    if token == nil then break end

    ---@type BaseType
    local current = stack[#stack]
    if token.type == TOKEN_TYPES.TYPEDEF then
      if #self.state.stack > 0 then 
        error("typedef must be defined in the root of the document")
      end

      if options.VERBOSE then
        print(string.format("typedef: %s", token.type))
      end

      local typedef = self:_parse_typedef()      
      table.insert(interpretation.elements, typedef)

    elseif token.type == TOKEN_TYPES.STRUCT then
      local struct = self:_parse_struct()
      table.insert(interpretation.elements, struct)

    elseif token.type == TOKEN_TYPES.SYMBOL then
      local o = {}
      local typeInfo = self:_parse_type_info()
      o.typeInfo = typeInfo
      self:_parse_function_pointer(o)
      table.insert(interpretation.elements, o)
    else
      error(string.format("uncaught situation: %s, %s", self:_current_token().type, self:_current_token().data))
    end
  end

  return interpretation
end

function Parser:parse(text)
  return self:interpret_tokens(self:tokens(text))
end

return Parser