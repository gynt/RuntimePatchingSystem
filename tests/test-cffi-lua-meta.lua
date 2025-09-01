rps = require("RPS")

StructObjectMetaTable = {
  -- retrieve all the fields and values of this struct
  __pairs = function(tt)
    print("StructObjectMetaTable: __pairs", tt)
    local itF, itT, itK, itV = ipairs(tt[".meta"].fieldArray)
    return function(t, k)
      -- t and k are ignored because we provide pairs return values on an ipairs call
      itK, itV = itF(itT, itK)
      return itV, tt[itV]
    end, tt, nil
  end,
  __index = function(tt, k) 
    print("StructObjectMetaTable: __index", tt, k)
   
    local field = tt[".meta"].fields[k]
    return field:read(tt['.address'])
  end,
  __newindex = function(tt, k, v)
    print("StructObjectMetaTable: __newindex", tt, k,v)

    local field = tt[".meta"].fields[k]
    field:write(tt['.address'], v)
  end,
  __gc = function(tt)
    if tt['.managed'] == true then
      print("__gc: deallocating")
      rps.deallocate(tt['.address'])
      return
    end
    print("__gc: not a managed object")
  end,
}

IntField = {
  read = function(t, address)
    -- TODO: where to set the offset? Optimize so the number of fields is low and can be reused dynamically?
    -- metatable this stuff? Should the field know about its embedder, or be agnostic?
    local addr = address + t.offset
    print("int:read: at: ", addr)
    return rps.readInteger(addr)
  end,
  write = function(t, address, v)
    local addr = address + t.offset
    print("int:write: at: ", addr, v)
    rps.writeInteger(addr, v)
  end,
}

function generateFieldInstance(field, offset)
  return setmetatable({
    offset = offset,
  }, {
    __index = field,
  })
end

ExampleStruct = {
  size = 4,
  fields = {
    field1 = generateFieldInstance(IntField, 0),
    -- field1 = {
    --   read = function(f, t, offset)
    --     -- TODO: where to set the offset? Optimize so the number of fields is low and can be reused dynamically?
    --     -- metatable this stuff? Should the field know about its embedder, or be agnostic?
    --     local addr = t['.address'] + offset
    --     print("int:read: at: ", addr)
    --     return rps.readInteger(addr)
    --   end,
    --   write = function(f, t, offset, v)
    --     local addr = t['.address'] + offset
    --     print("int:write: at: ", addr, v)
    --     rps.writeInteger(addr, v)
    --   end,
    -- },
  },
  fieldArray = {"field1", }
}

function generateStruct(t)
  return setmetatable({
    ['.meta'] = t, 
    ['.address'] = rps.allocate(t.size, true),
    ['.managed'] = true,
  }, StructObjectMetaTable)
end

function generateStructExample()
  return generateStruct(ExampleStruct)
end

s = generateStructExample()

