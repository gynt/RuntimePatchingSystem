local common = {}

common.BaseType = {}

function common.BaseType:new(type)
  local o = setmetatable({
    type = type
  }, self)
  self.__index = self
  return o
end

function common.BaseType:isInstance(o)
  return self.type == o.type
end

return common