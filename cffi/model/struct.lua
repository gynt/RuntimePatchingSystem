
---@class StructField
---@field name string
---@field pointer number
---@field array number
---@field array_dim table<number>
---@field basetype string

---@class StructParams
---@field name string|nil
---@field fields table<StructField>
---@field extends Struct|nil

---@class Struct
local Struct = {}



---@param params StructParams
function Struct:new(params)
  local o = setmetatable(params, self)
  self.__index = self
  return o
end


return Struct