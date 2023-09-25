instance = instance

function init(instance_name, ...)
    instance_name = instance_name or "script/mobile_game.lua"
    instance = import(instance_name)
    instance.init(instance_name, ...)
end

function tick(...)
    instance.tick(...)
end

function on_event(...)
    instance.on_event(...)
end
