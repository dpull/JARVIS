local system = require "system"
local controller = import("script/coroutine/controller.lua")
local service = {
    import("script/coroutine/reload.lua"),
    import("script/emulator.lua"),
    import("script/coroutine/async.lua"),
    import("script/coroutine/webclient.lua"),
}

disable = disable or false

local function file_exist(file)
    local file = io.open(file, "r")
    if not file then
        return false
    end
    file:close()
    return true
end

local function start(cmd)
    local cmd_path = string.format("script/cmd/%s.lua", cmd)
    if cmd ~= auto_cmd and not file_exist(cmd_path) then
        print("cmd file %s not exist.", cmd_path)
        return
    end

    controller.clear_all_user()
    local cmd_lib = import(cmd_path)
    cmd_lib.init()   

    auto_cmd = cmd
end

local function change_enable()
    disable = not disable
    print("disable:", disable)
    if disable then
        controller.clear_all_user()
    else
        start(auto_cmd)
    end
end

function init(instance_name, cmd, ...)
    for _, s in ipairs(service) do
       s.init() 
    end
    start(cmd or "a")
end

function tick()
    local now = system.get_time_ms()
    controller.tick()
    local cost = system.get_time_ms() - now
    if cost > 25 then
        print("tick so slow", cost)
    end  
end

function on_event(key, ...)
    if key == "cmd" then
        start(...)
    elseif key == "change_enable" then
        change_enable()
    end    
end
