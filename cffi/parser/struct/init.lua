local common = require("cffi.parser.common")

local struct = {}

---@class StructPartHeader : BaseType
struct.StructPartHeader = common.BaseType:new("StructPartHeader")

function struct.StructPartHeader:init(name)
  return {
    name = name,
  }
end

---@class StructPartBody
struct.StructPartBody = common.BaseType:new("StructPartBody")

function struct.StructPartBody:init()
  local o = { fields = {}, }
  setmetatable(o, self)
  self.__index = self
  return o
end

---@class StructPartField
struct.StructPartField = common.BaseType:new("StructPartField")

function struct.StructPartField:init()
  local o = { name = nil, type = nil, pointer = 0, array = 0, array_dim = {}, }
  setmetatable(o, self)
  self.__index = self
  return o
end

return struct