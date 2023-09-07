instance = instance

function init(arg1, ...)
    local instance_name = arg1 or "script/mobile_game.lua"
    instance = import(instance_name)
    instance.init(arg1, ...)
end

function tick(...)
    instance.tick(...)
end

function on_event(...)
    instance.on_event(...)
end
