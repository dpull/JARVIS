local controller = import("script/coroutine/controller.lua")

function init()
    controller.fork("reload", function()
        while true do
            reload_all()
            controller.sleep(3000)
        end
    end, true)
end
