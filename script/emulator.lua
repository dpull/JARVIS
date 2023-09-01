local windows = require "windows"
local tx_emulator_pid = 239916

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

function press_keyboard(vk)
    -- #define WM_KEYDOWN                      0x0100
    -- #define WM_KEYUP                        0x0101
    -- windows.send_message(emulator, 0x0100, vk, 0)
    windows.send_input({type = 1, vk=vk, scan=0, flags=0}, {type = 1, vk=vk, scan=0, flags=2})
end

local function is_manually()
    local ret = {windows.get_async_key_state(string.byte("JHWASD", 1, -1))}
    for i, v in pairs(ret) do
        if v then
            return true
        end
    end
end

function get_click_pos()
    -- #define VK_LBUTTON        0x01
    local ret = windows.get_async_key_state(1)
    if not ret then
        return
    end
    local x, y = windows.get_cursor_pos()
    local top, left = windows.get_window_rect(emulator)
    return x - left, y - top
end

function click_mouse(x, y)
    local top, left = windows.get_window_rect(emulator)
    windows.set_cursor_pos(left+x, top+y)
    -- #define MOUSEEVENTF_LEFTDOWN    0x0002
    -- #define MOUSEEVENTF_LEFTUP      0x0004 
    windows.send_input({type = 0, x=0, y=0, flags=2}, {type = 0, x=0, y=0, flags=4})
 end

 function enable()
    if windows.get_foreground_window() ~= emulator then
        return
    end

    local now = os.time()
    if is_manually() then
        last_manually = now
        return
    end
    if last_manually == now then
        return
    end

    return true
 end

function attack()
    press_keyboard(string.byte("J"))
    print("attack")
end

init()
