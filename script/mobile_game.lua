local emulator = import("script/emulator.lua")
     
disable = disable or false

function init(instance_name, cmd, ...)
    auto_cmd = cmd or "a"
    emulator.init()
end

function tick()
    emulator.on_frame_begin()
    if not disable then
        exec(auto_cmd)
    end
    emulator.on_frame_end()
end


function on_event(key, ...)
    if key == "cmd" then
        exec(...)
    elseif key == "change_enable" then
        change_enable()
    end
end

-----------------------------------------------------------------------------

local function file_exist(file)
    local file = io.open(file, "r")
    if not file then
        return false
    end
    file:close()
    return true
end

function exec(cmd)
    local cmd_path = string.format("script/cmd/%s.lua", cmd)
    if cmd ~= auto_cmd and not file_exist(cmd_path) then
        print("cmd file %s not exist.", cmd_path)
        return
    end

    if cmd ~= auto_cmd and cmd ~= "r" then
        auto_cmd = cmd 
        print("auto_cmd:", cmd_path)
    end
    local cmd_lib = import(cmd_path)
    cmd_lib.exec()

    emulator.set_title(cmd)
end

function change_enable()
    disable = not disable
    print("disable:", disable)
end