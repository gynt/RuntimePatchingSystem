
local ParseState = require("cffi.parser.state")

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

function Parser:parse(str)
  local state = ParseState:init({text = str})

  local tokens = state:tokens()

  return tokens
end

return Parser