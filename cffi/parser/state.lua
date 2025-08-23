
---@enum TOKEN_TYPES
local TOKEN_TYPES = {
  NONE = "NONE",
  DOUBLE_QUOTED_STRING = "DOUBLE_QUOTED_STRING",
  SINGLE_QUOTED_STRING = "SINGLE_QUOTED_STRING",
}

---@class ParseState
---@field doubleQuoteLevel integer
---@field singleQuoteLevel integer
---@field curlyBraceLevel integer
---@field bracketLevel integer
---@field text string
---@field index integer
---@field tokenType string
local ParseState = {}

---@return ParseState instance
function ParseState:init(options)
  local opts = options

  if options == nil then error("no options specified") end
  if options.text == nil then error("no text specified") end

  local o = {
    doubleQuoteLevel = 0,
    singleQuoteLevel = 0,
    curlyBraceLevel = 0,
    bracketLevel = 0,
    text = options.text,
    tokenType = '',
    index = 1,
    currentToken = {type = TOKEN_TYPES.NONE},
    character = '',
    slice = '',
  }
  setmetatable(o, self)
  self.__index = self

  ---@diagnostic disable-next-line: return-type-mismatch
  return o
end


---@generic Token: table
---@return Token token
local function copyToken(token)
  local copy = {}
  for k, v in pairs(token) do copy[k] = v end
  return copy
end

function ParseState:next(count)
  self.index = self.index + (count or 1)
end

function ParseState:token()
  local index = self.index
  local slice = self.text:sub(index)
  self.slice = slice
  local character = slice:sub(1, 1)
  self.character = character
  local token = self.currentToken

  if character == '"' then
    if self.singleQuoteLevel == 1 then
      token.data = token.data .. character
      return 1
    elseif self.doubleQuoteLevel == 1 then
      self.doubleQuoteLevel = 0

      local t = copyToken(token)

      token.type = TOKEN_TYPES.NONE
      token.data = nil
      
      return 1, t
    elseif self.doubleQuoteLevel == 0 then
      self.doubleQuoteLevel = 1
      token.type = TOKEN_TYPES.DOUBLE_QUOTED_STRING
      token.data = ""
      
      return 1
    end
  elseif character == "'" then
    if self.doubleQuoteLevel == 1 then
      token.data = token.data .. character
      
      return 1
    elseif self.singleQuoteLevel == 1 then
      self.singleQuoteLevel = 0

      local t = copyToken(token)

      token.type = TOKEN_TYPES.NONE
      token.data = nil
      
      return 1, t
    elseif self.singleQuoteLevel == 0 then
      self.singleQuoteLevel = 1
      token.type = TOKEN_TYPES.SINGLE_QUOTED_STRING
      token.data = ""
      
      return 1
    end
  else
    if self.singleQuoteLevel > 0 or self.doubleQuoteLevel > 0 then
       token.data = token.data .. character
       return 1
    end
  end

  error(string.format("unhandled character: %s", character))
end

function ParseState:tokens()
  local tokens = {}

  while self.index <= self.text:len() do
    local increment, token = self:token()
    self:next(increment)

    if token ~= nil then
      if token == self.currentToken then error() end

      table.insert(tokens, token)
    end
  end
  
  return tokens
end

return ParseState