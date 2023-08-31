local windows = require "windows"
local tx_emulator_pid = 210472
local VJ = string.byte("J")

local function get_tx_emulator_wnd(pid)
    local wnds = {windows.find_window(pid)}
    for _, wnd in ipairs(wnds) do
        local title = windows.get_window_text(wnd)
        if string.find(title, "腾讯手游助手") then
            return wnd
        end
    end    
end

local function get_tx_emulator()
    if tx_emulator_pid then
        return get_tx_emulator_wnd(tx_emulator_pid)
    end

    local pids = {windows.find_process("AndroidEmulator.exe")}
    for _, pid in ipairs(pids) do
        local wnd = get_tx_emulator_wnd(pid)
        if wnd then
            return wnd
        end
    end
end

function init()
    if emulator then
        return
    end

    emulator = get_tx_emulator()
    assert(emulator)
    windows.set_foreground_window(emulator)    
end

function attack()
    if windows.get_foreground_window() ~= emulator then
        return
    end
   windows.keybd_event(VJ, 0, 0)
   windows.keybd_event(VJ, 0, 2)
end

init()
