local controller = import("script/coroutine/controller.lua")
local service = {import("script/coroutine/reload.lua"), import("script/coroutine/async.lua"),
                 import("script/coroutine/webclient.lua")}

instance = instance

function init(instance_name, ...)
    for _, s in ipairs(service) do
        s.init()
    end

    instance_name = instance_name or "script/warcraft.lua"
    instance = import(instance_name)
    instance.init(instance_name, ...)
end

function tick()
    controller.tick()
end

function on_event(...)
    instance.on_event(...)
end
