local GLOBAL_OPTIONS = require("cffi.options")

local TOKEN_TYPES = require("cffi.parser.tokens").TOKEN_TYPES

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

function ParseState:increment(count)
  if GLOBAL_OPTIONS.VERBOSE == true then
    -- print(string.format("increment(%s): from %s to %s", count, self.index, self.index + (count or 1)))
  end

  self.index = self.index + (count or 1)
end

function ParseState:next(length, skip)
  skip = skip or 0
  ---TODO: decide on self.text or self.slice, if the latter, remove self.index
  return self.text:sub(self.index + skip, self.index + skip + length - 1)
end



function ParseState:token()
  local index = self.index
  local slice = self.text:sub(index)
  self.slice = slice
  local character = self:next(1)
  if GLOBAL_OPTIONS.VERBOSE == true then
    print(character)
  end
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

    if character == "{" then
      return 1, {
        type = TOKEN_TYPES.CURLY_BRACKET_OPEN,
        data = "{",
      }
    elseif character == "}" then
      return 1, {
        type = TOKEN_TYPES.CURLY_BRACKET_CLOSE,
        data = "}",
      }
    elseif character == "(" then
      return 1, {
        type = TOKEN_TYPES.PARENTHESIS_OPEN,
        data = "(",
      }
    elseif character == ")" then
      return 1, {
        type = TOKEN_TYPES.PARENTHESIS_CLOSE,
        data = ")",
      }
    end

    local whitespaceIndex, whiteSpaceLength = slice:find("[%s+]")
    if whitespaceIndex == 1 then
      return whiteSpaceLength
    end

    if self:next(2) == "0x" then
      local hexIndex, hexLength, hex = slice:find("([A-Fa-f0-9]+)", 3)
      if hexIndex ~= 3 then
        error(string.format("hexIndex not 1: %s", hexIndex))
      end

      return 2 + hexLength, {
        type = TOKEN_TYPES.HEXADECIMAL,
        data = tonumber(hex, 16)
      }
    end

    local integerIndex, integerLength, integerOrLong = slice:find("([0-9]+)")
    if integerIndex == 1 then
      
      if self:next(1, integerLength) == "L" then
        if token.type == TOKEN_TYPES.HEXADECIMAL then error() end
        return integerLength + 1, {
          type = TOKEN_TYPES.LONG,
          data = tonumber(integerOrLong, 10),
        }
      else
        return integerLength, {
          type = TOKEN_TYPES.INTEGER,
          data = tonumber(integerOrLong, 10),
        }
      end
    end

    local arraySpecIndex, arraySpecLength, arraySpec = slice:find("%[([0-9]+)%]")
    if arraySpecIndex == 1 then
      return arraySpecLength, {
        type = TOKEN_TYPES.ARRAY_DIM,
        data = arraySpec, -- the value
      }
    end

    local wordIndex, wordLength, word = slice:find("([_%w]+)")
    if wordIndex == 1 then
      if word == "struct" then
        return wordLength, {
          type = TOKEN_TYPES.STRUCT,
          data = word,
        }
      elseif word == "typedef" then
        return wordLength, {
          type = TOKEN_TYPES.TYPEDEF,
          data = word,
        }
      elseif word == "enum" then
        return wordLength, {
          type = TOKEN_TYPES.ENUM,
          data = word,
        }
      elseif word == "union" then
        return wordLength, {
          type = TOKEN_TYPES.UNION,
          data = word,
        }
      else
        return wordLength, {
          type = TOKEN_TYPES.SYMBOL,
          data = word,
        }
      end
    end

    return 1, {
      type = TOKEN_TYPES.CHARACTER,
      data = character,
    }
  end

  error(string.format("unhandled character: %s", character))
end

function ParseState:tokens()
  local tokens = {}

  while self.index <= self.text:len() do
    local increment, token = self:token()
    self:increment(increment)

    if token ~= nil then
      if token == self.currentToken then error() end

      table.insert(tokens, token)
    end
  end
  
  return tokens
end

return ParseState