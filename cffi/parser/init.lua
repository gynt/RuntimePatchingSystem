
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

local PARSER_DEFAULTS = {}

---@return Parser instance
function Parser:init(options)
  local opts = options or PARSER_DEFAULTS

  local o = {}
  setmetatable(o, self)
  self.__index = self

  ---@diagnostic disable-next-line: return-type-mismatch
  return o
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

local TYPE_EXTENDERS = {
  ["unsigned"] = true,
  ["union"] = true,
  ["struct"] = true,
  ["enum"] = true,
}

function Parser:_parse_type_info()
  local info = {}

  local fieldTypeNameToken = self:_next_token()
  if fieldTypeNameToken.type ~= TOKEN_TYPES.SYMBOL then
    error(string.format("expected a symbol but received: %s", fieldTypeNameToken.data))
  end

  if TYPE_EXTENDERS[fieldTypeNameToken.data] == true then
    info.name = fieldTypeNameToken.data
    local fieldTypeNameToken2 = self:_next_token()
    if fieldTypeNameToken2.type ~= TOKEN_TYPES.SYMBOL then
      error(string.format("expected a symbol but received: %s", fieldTypeNameToken2.data))
    end
    info.name = info.name .. " " .. fieldTypeNameToken2.data
  else
    info.name = fieldTypeNameToken.data
  end

  info.pointers = 0
  local nextToken = self:_peek_next_token()
  while nextToken.type == TOKEN_TYPES.CHARACTER and nextToken.data == "*" do
    info.pointers = info.pointers + 1
    nextToken = self:_next_token()
    nextToken = self:_peek_next_token()
  end

  return info
end

function Parser:_parse_field_function_pointer(field)
  
  local nextToken = self:_next_token()
  if nextToken.type ~= TOKEN_TYPES.PARENTHESIS_OPEN then
    error("expected '('")
  end

  local nextToken = self:_next_token()
  if nextToken.type == TOKEN_TYPES.SYMBOL then
    field.callingConvention = nextToken.data
    nextToken = self:_next_token()
  end

  if nextToken.type ~= TOKEN_TYPES.CHARACTER or nextToken.data ~= "*" then
    error("expected '*'")
  end

  while nextToken.type == TOKEN_TYPES.CHARACTER and nextToken.data == "*" do
    if field.fpointers == nil then field.fpointers = 0 end
    field.fpointers = field.fpointers + 1
    nextToken = self:_next_token()
  end

  if nextToken.type == TOKEN_TYPES.SYMBOL then
    field.fieldName = nextToken.data
  end

  nextToken = self:_next_token()
  if nextToken.type ~= TOKEN_TYPES.PARENTHESIS_CLOSE then
    error(string.format("expected ')' but received: %s", nextToken.data))
  end

  local nextToken = self:_next_token()
  if nextToken.type ~= TOKEN_TYPES.PARENTHESIS_OPEN then
    error("expected '('")
  end

  field.arguments = {}
  nextToken = self:_next_token()
  while nextToken.type ~= TOKEN_TYPES.PARENTHESIS_CLOSE do
    local argument = {}
    table.insert(field.arguments, argument)

    argument.typeInfo = self:_parse_type_info()
    nextToken = self:_next_token()
    if nextToken.type == TOKEN_TYPES.SYMBOL then
      argument.name = nextToken.data
      nextToken = self:_next_token()
    end

    if nextToken.type == TOKEN_TYPES.PARENTHESIS_CLOSE then
      break
    end
    if nextToken.type ~= TOKEN_TYPES.CHARACTER or nextToken.data ~= "," then
      -- do nothing
      error()
    end
  end

  local finalToken = self:_next_token()
  if finalToken.type ~= TOKEN_TYPES.CHARACTER or finalToken.data ~= ";" then
    error("expected ';'")
  end

  return field
end

function Parser:_parse_field_data(field)
  local nextToken = self:_next_token()

  local fieldNameToken = nextToken
  if fieldNameToken.type ~= TOKEN_TYPES.SYMBOL then
    error("expected fieldname but received: ")
  end
  field.fieldName = fieldNameToken.data

  nextToken = self:_next_token()
  while nextToken.type == TOKEN_TYPES.ARRAY_DIM do
    if field.array_dim == nil then field.array_dim = {} end
    table.insert(field.array_dim, nextToken.data)
    nextToken = self:_next_token()
  end

  local colonToken = nextToken
  if colonToken.type ~= TOKEN_TYPES.CHARACTER or colonToken.data ~= ";" then
    error(string.format("expected ';' but received: %s", colonToken.data))
  end

  return field
end

function Parser:_parse_field()
  local field = {}
  
  local typeInfo = self:_parse_type_info()
  field.typeInfo = typeInfo

  if self:_peek_next_token(1).type == TOKEN_TYPES.PARENTHESIS_OPEN then
    return self:_parse_field_function_pointer(field)
  end

  return Parser:_parse_field_data(field)
end

---@class _parse_struct_Params
---@field start "struct"|"name"

---@param params _parse_struct_Params
function Parser:_parse_struct(params)
  params = params or {start = "name"}
  local stack = self.state.stack
  local struct = {
    type = "struct",
    fields = {},
  }
  local stackPosition = table.insert(stack, struct)

  if params.start == "struct" then 
    if self:_next_token().type ~= TOKEN_TYPES.STRUCT then error("not a struct") end
  end
  local tokenName = self:_next_token()
  if tokenName.type == TOKEN_TYPES.CURLY_BRACKET_OPEN then
    error("anonymous structs are not implemented")
  end

  if tokenName.type ~= TOKEN_TYPES.SYMBOL then
    error(string.format("expected symbol but received: %s (%s)", tokenName.data, tokenName.type))
  end

  struct.name = tokenName.data
  
  local curlyBraceOpen = self:_next_token()
  if curlyBraceOpen.type ~= TOKEN_TYPES.CURLY_BRACKET_OPEN then
    error(string.format("expected '{' but received: %s (%s)", tokenName.data, tokenName.type))
  end

  local fieldCounter = 0
  local nextToken = self:_next_token()
  while nextToken.type ~= TOKEN_TYPES.CURLY_BRACKET_CLOSE do
    fieldCounter = fieldCounter + 1

    local field = {}
    table.insert(struct.fields, field)
    
    local fieldTypeNameToken = nextToken
    if fieldTypeNameToken.type ~= TOKEN_TYPES.SYMBOL then
      error(string.format("expected a symbol but received: %s", fieldTypeNameToken.data))
    end

    if TYPE_EXTENDERS[fieldTypeNameToken.data] == true then
      field.fieldTypeName = fieldTypeNameToken.data
      local fieldTypeNameToken2 = self:_next_token()
      if fieldTypeNameToken2.type ~= TOKEN_TYPES.SYMBOL then
        error(string.format("expected a symbol but received: %s", fieldTypeNameToken2.data))
      end
      field.fieldTypeName = field.fieldTypeName .. " " .. fieldTypeNameToken2.data
    else
      field.fieldTypeName = fieldTypeNameToken.data
    end

    field.pointers = 0
    nextToken = self:_next_token()
    while nextToken.type == TOKEN_TYPES.CHARACTER and nextToken.data == "*" do
      field.pointers = field.pointers + 1
      nextToken = self:_next_token()
    end

    local fieldNameToken = nextToken
    if fieldNameToken.type ~= TOKEN_TYPES.SYMBOL then
      error("expected fieldname but received: ")
    end
    field.fieldName = fieldNameToken.data

    nextToken = self:_next_token()
    while nextToken.type == TOKEN_TYPES.ARRAY_DIM do
      if field.array_dim == nil then field.array_dim = {} end
      table.insert(field.array_dim, nextToken.data)
      nextToken = self:_next_token()
    end

    local colonToken = nextToken
    if colonToken.type ~= TOKEN_TYPES.CHARACTER or colonToken.data ~= ";" then
      error(string.format("expected ';' but received: %s", colonToken.data))
    end

    nextToken = self:_next_token()
  end

  if nextToken.type == TOKEN_TYPES.CURLY_BRACKET_CLOSE then
    table.remove(self.state.stack, stackPosition)
  else
    error("unfinished struct")
  end

  return struct
end

function Parser:_parse_typedef()
  local typedef = {
    type = "typedef",
    typed = nil,
    name = nil,
  }

  local nextToken = self:_peek_next_token()

  if options.VERBOSE then
    print(string.format("nextToken: %s", nextToken.type))
  end

  if nextToken.type == TOKEN_TYPES.STRUCT then
    local struct = self:_parse_struct({start = "struct"})
    typedef.typed = struct
  else
    error("not implemented")
  end

  local typeDefNameToken = self:_next_token()
  if typeDefNameToken.type ~= TOKEN_TYPES.SYMBOL then error(typeDefNameToken.data) end
  typedef.name = typeDefNameToken.data

  local finishToken = self:_next_token()
  if finishToken.type ~= TOKEN_TYPES.CHARACTER or finishToken.data ~= ';' then
    error(string.format("expected end of typedef, not: %s", finishToken.data))
  end

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
    index = 0, -- uninitialized
    interpretation = interpretation,
  }
  local state = self.state
  ---@type table<StackElement>
  local stack = state.stack

  while state.index <= #tokens do
    local token = self:_next_token() -- first time selects 1
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
      state.index = state.index - 1
      local o = {}
      local typeInfo = self:_parse_type_info()
      o.typeInfo = typeInfo
      local functionPointer = self:_parse_field_function_pointer(o)
      table.insert(interpretation.elements, o)
    end
  end

  return interpretation
end

function Parser:parse(text)
  return self:interpret_tokens(self:tokens(text))
end

return Parser