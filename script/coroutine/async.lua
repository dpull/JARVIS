local serialize = require "serialize"
local asynlib = require "async_task"

local controller = import("script/coroutine/controller.lua")

requests = requests or setmetatable({}, {
    __mode = "v"
})

function init()
    assert(not async_task)
    async_task = asynlib.create()
    controller.fork("async_task", function()
        while true do
            _tick()
            controller.sleep(0)
        end
    end, true)
end

local function resopnd(token, result, arg)
    local co = requests[token]
    if not co then
        return
    end

    if result ~= 1 then
        coroutine.resume(co, false)
        return
    end

    if not arg then
        coroutine.resume(co, true)
        return
    end

    coroutine.resume(co, true, serialize.deseristring_string(arg))
end

function _tick()
    local token, result, arg = async_task:pop()
    if not token then
        return
    end

    pcall(resopnd, token, result, arg)
end

function call(file, func, ...)
    local arg = serialize.serialize_string(file, func, ...)
    local token = async_task:push("script/coroutine/async.lua", "_async_task", arg)

    requests[token] = coroutine.running()
    return coroutine.yield()
end

local function do_task(file, func, ...)
    local lib = file and import(file) or _G
    local f = assert(lib[func])
    return f(...)
end

function _async_task(arg)
    reload_all()
    return serialize.serialize_string(do_task(serialize.deseristring_string(arg)))
end
