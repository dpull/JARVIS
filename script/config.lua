local json = require "json"

function read_config()
    local file = assert(io.open("save/config.json", "r"))
    local content = file:read("*a")  
    file:close()  

    local kv = json.decode(content)
    for k, v in pairs(kv) do
        _ENV[k] = v
    end
end

read_config()
