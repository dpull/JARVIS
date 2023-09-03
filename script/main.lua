local emulator = import("script/emulator.lua")
     
auto_cmd = auto_cmd or "a"
disable = disable or false

function init(arg1, ...)
    emulator.init()
end

function tick()
    emulator.frame = emulator.frame + 1
    if disable then
        return
    end
    exec(auto_cmd)
end

function file_exist(file)
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
    if disable then
        emulator.stop_hack()
    end
end