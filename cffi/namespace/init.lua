local namespace = {}

---@class Namespace
---@field database table<string, any>
namespace.Namespace = {}

function namespace.Namespace:init(params)
  local o = {
    database = {},
  }
  setmetatable(o, self)
  self.__index = self
  return o
end

function namespace.Namespace:register(name, type)
  if self.database[name] ~= nil then error("already registered") end
  self.database[name] = type
end

function namespace.Namespace:exists(name)
  local value = self.database[name]
  return value ~= nil, value
end

namespace.GLOBAL = namespace.Namespace:init()

return namespace