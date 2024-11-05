local system = require "system"
local controller = import("script/coroutine/controller.lua")
local emulator = import("script/emulator.lua")
local notify = import("script/notify.lua")
local app_name = "com.blizzard.arc"
local app_activity = "com.blizzard.arc/com.blizzard.gryphon.unity.player.GryphonUnityPlayerActivity"

function init(instance_name, cmd, ...)
    emulator.init()

    controller.fork("check", function()
        while true do
            check()
            controller.sleep(1000)
        end
    end)
end

function on_event(key, ...)
    if key == "change_enable" then
    end
end

function check()
    local pid = emulator.get_pid(app_name)
    if pid then
        notify.text("running pid=" .. pid)
        return
    end

    emulator.start(app_activity)
    controller.sleep(2000)

    local pid = emulator.get_pid(app_name) or ""
    notify.text("not running, restart pid=" .. pid)
end
