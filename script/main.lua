auto_cmd = auto_cmd or "a"

function init(arg1, ...)
    import("script/emulator.lua")
end

function tick()
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

    if cmd ~= "r" then
        auto_cmd = cmd 
    end
    local cmd_lib = import(cmd_path)
    cmd_lib.exec()
end