adb_path = adb_path or "\"c:\\Program Files\\Netease\\MuMu Player 12\\shell\\adb.exe\""

function set_path(path)
    adb_path = path
end

local function trim(s)
    if not s then
        return s
    end
    return s:match("^%s*(.-)%s*$")
end

function adb(...)
     local args = table.concat({...}, " ")
     local command = string.format("%s %s", adb_path, args);
 
     local handle = io.popen(command)
     if not handle then
         return nil, "failed to execute ADB command."
     end

     local result = handle:read("*a")
     handle:close()

     return trim(result)
end

function connect(ip, port)
    local host = string.format("%s:%s", ip, port)
    return adb("connect", host)
end

function shell(...)
    return adb("shell", ...)
end

function pull(...)
    return adb("pull", ...)
end