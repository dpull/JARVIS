local system = require "system"
local log_tree = require "tree"

system_co = system_co or {}
user_co = user_co or {}

sleep_co = sleep_co or setmetatable({}, {__mode = "k"}) 
co2name = co2name or setmetatable({}, {__mode = "k"})

function create(name, fn, is_system)
    assert(get_co(name) == nil)

    local tb = is_system and system_co or user_co
    local co = coroutine.create(function ()
        local ok, msg = xpcall(fn, debug.traceback)
        if not ok then
            print(name, msg)
        end
        tb[name] = nil
    end)
    tb[name] = co
    co2name[co] = name
    return co
end

function fork(...)
    local co = create(...)
    coroutine.resume(co)
end

function clear_all_user()
    user_co = {}
    collectgarbage("collect")
end

function clear(name)
    system_co[name] = nil
    user_co[name] = nil
    collectgarbage("collect")
end

function get_name(co)
    co = co or coroutine.running()
    local name = assert(co2name[co])
    assert(get_co(name) ~= nil, name)
    return name
end

function get_co(name)
    return system_co[name] or user_co[name]
end

function sleep(time)
    local now = system.get_time_ms()
    local co = coroutine.running()
    sleep_co[co] = now + time
    coroutine.yield()
end

function tick()
    local now = system.get_time_ms()

    local weekup_co = {}
    for k, v in pairs(sleep_co) do
        if v <= now then
            sleep_co[k] = nil
            table.insert(weekup_co, k)
        end
    end

    for _, co in pairs(weekup_co) do
        coroutine.resume(co)
    end
end

function stat()
    local tb = {}
    for k, v in pairs(system_co) do
        tb[k] = {}
        tb[k].is_sleep = sleep_co[v]
    end
    for k, v in pairs(user_co) do
        tb[k] = {}
        tb[k].is_sleep = sleep_co[v]
    end
    log_tree("co stat:", tb)
end