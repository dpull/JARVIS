local windows = require "windows"

frame = frame or 0
last_state = last_state or ""
last_manually = last_manually or 0

local function get_tx_emulator_wnd(pid)
    local wnds = {windows.find_window(pid)}
    for _, wnd in ipairs(wnds) do
        local title = windows.get_window_text(wnd)
        if string.find(title, "腾讯手游助手") then
            local child = { windows.get_child_window(wnd)}
            for _, subwnd in pairs(child) do
                title = windows.get_window_text(subwnd)
                if string.find(title, "EngineRenderWindow") then
                    return wnd, subwnd
                end
            end
            return nil
        end
    end    
end

local function get_tx_emulator()
    local pids = {windows.find_process("AndroidEmulator")}
    for _, pid in ipairs(pids) do
        local wnd, subwnd = get_tx_emulator_wnd(pid)
        if wnd then
            print("find tx emulator", pid)
            return wnd, subwnd
        end
    end
end

function init()
    emulator, emulator_msg_proc = get_tx_emulator()
    assert(emulator)
    windows.set_foreground_window(emulator)    
end

function on_frame_begin()
    frame = frame + 1
    cur_frame_input = next_frame_input or {}
    next_frame_input = {}
    cur_frame_unique = {}
end

function on_frame_end()
    for _, msg in ipairs(cur_frame_input) do
        windows.post_message(emulator_msg_proc, table.unpack(msg))
    end
end

function press_keyboard(key)
    if cur_frame_unique[key] then
        return
    end
    cur_frame_unique[key] = true

    local k, scan = windows.kb_code(key)
    if not k then
        return
    end

    -- #define WM_KEYDOWN                      0x0100
    -- #define WM_KEYUP                        0x0101
    local lparam = 0x1 | (scan << 16) 
    table.insert(cur_frame_input, {0x0100, k, lparam})

    lparam = lparam | (0x1 << 30)  | (0x1 << 31) 
    table.insert(next_frame_input, {0x0101, k, lparam})
end

function get_click_pos()
    -- #define VK_LBUTTON        0x01
    local ret = windows.get_async_key_state(1)
    if not ret then
        return
    end
    local x, y = windows.get_cursor_pos()
    local left, top = windows.get_window_rect(emulator)
    return x - left, y - top
end

function click_mouse(x, y)
    local left, top = windows.get_window_rect(emulator)
    windows.set_cursor_pos(x + left, y + top)

    -- #define MOUSEEVENTF_LEFTDOWN    0x0002
    -- #define MOUSEEVENTF_LEFTUP      0x0004 
    windows.send_input({type = 0, x=0, y=0, flags=0x0002}, {type = 0, x=0, y=0, flags=0x0004})
end

local function is_manually()
    local ret = {windows.get_async_key_state(string.byte("J WASDHUIK", 1, -1))}
    for i, v in pairs(ret) do
        if v then
            print("is_manually", i)
            return true
        end
    end
end

function enable()
    if windows.get_foreground_window() ~= emulator then
        return
    end

    if is_manually() then
        last_manually = frame + 10
        return
    end
    if last_manually >= frame then
        return
    end
    return true
 end

function attack()
    press_keyboard('j')
end

function set_title(cmd)
    local title = string.format("[%d]%s.%s", frame, cmd, last_state)
    windows.set_console_title(title)
end

function set_state(stat)
    last_state = stat
end
