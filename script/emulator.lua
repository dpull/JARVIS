local adb = import("script/adb.lua")

function init(path)
    assert(adb.connect("127.0.0.1", 16416))
end

function get_pid(app_name)
    local stdout, msg = adb.shell("pidof", app_name)
    if stdout == nil or stdout == "" then
        return nil
    end
    return stdout
end

function start(app_activity)
    adb.shell("am start -n", app_activity)
end