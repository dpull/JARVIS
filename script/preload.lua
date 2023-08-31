package.path = package.path .. ";./?.lua;./script/?.lua"

local loadfile = loadfile
local setmetatable = setmetatable
local xpcall = xpcall
local debug_traceback = debug.traceback
local log_error = print

_G.__IMPORT_FILES = _G.__IMPORT_FILES or {}

local try_load = function(node)
    local path = node.fullpath;
    local fn, msg = loadfile(path, "t", node.env)
    if not fn then
        log_error(string.format("load file: %s ... ... [failed]", node.filename));
        log_error(msg);
        return;
    end
    local ok, err = xpcall(fn, debug_traceback)
    if not ok then
        log_error(string.format("exec file: %s ... ... [failed]", node.filename));
        log_error(err);
        return;
    end
end

function _G.import(filename)
    local fullpath = filename;
    local node = _G.__IMPORT_FILES[fullpath];
    if node then
        return _G.__IMPORT_FILES[fullpath].env;
    end

    local env = {};
    setmetatable(env, {__index = _G});
    node = {env=env, fullpath=fullpath, filename=filename};
    _G.__IMPORT_FILES[fullpath] = node;
    try_load(node);
    return node.env;
end

function _G.reload_all()
    for _, node in pairs(_G.__IMPORT_FILES) do
        pcall(try_load, node)
    end
end

local local_import = _G.import
local function xpcall_ret(ok, ...)
    assert(ok, (...))
    return ...
end
local function call_module_function(path, func, ...)
    local module = local_import(path)
    assert(module)
    local fn = module[func]
    assert(fn)
    return xpcall_ret(xpcall(fn, debug_traceback, ...))
end
return call_module_function
