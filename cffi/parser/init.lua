
local TOKEN_TYPES = require("cffi.parser.tokens").TOKEN_TYPES
local ParseState = require("cffi.parser.state")
local Struct = require("cffi.model.struct")

local struct = require("cffi.parser.struct")

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

function Parser:interpret_tokens(tokens)
  self.state = {
    tokens = tokens,
    stack = {},
  }
  local state = self.state
  local stack = state.stack

  for _, token in ipairs(tokens) do
    ---@type BaseType
    local current = stack[#stack]
    if token.type == TOKEN_TYPES.TYPEDEF then
      if #self.state.stack > 0 then 
        error("typedef must be start of statement")
      end

    elseif token.type == TOKEN_TYPES.STRUCT then
      table.insert(stack, struct.StructPartHeader:init(nil))
    elseif token.type == TOKEN_TYPES.SYMBOL then
      if current ~= nil then
        if current:isInstance(struct.StructPartHeader) then
          current.name = token.data
        elseif current:isInstance(struct.StructPartBody) then
        elseif current:isInstance(struct.StructPartField) then

        end
      end
    end
  end

  return stack
end

function Parser:parse(text)
  return self:interpret_tokens(self:tokens(text))
end

return Parser