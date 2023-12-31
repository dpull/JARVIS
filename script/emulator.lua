local windows = require "windows"
local system = require "system"
local controller = import("script/coroutine/controller.lua")

now = now or system.get_time_ms()
frame = frame or 0
fps = fps or 1
start_time = start_time or now
last_state = last_state or ""
last_manually = last_manually or 0

local function get_tx_emulator_wnd(pid)
    local wnds = {windows.find_window(pid)}
    for _, wnd in ipairs(wnds) do
        local title = windows.get_window_text(wnd)
        if string.find(title, "腾讯手游助手", 1, true) then
            local child = { windows.get_child_window(wnd)}
            for _, subwnd in pairs(child) do
                title = windows.get_window_text(subwnd)
                if string.find(title, "EngineRenderWindow", 1, true) then
                    return wnd, subwnd
                end
            end
            return nil
        end
    end    
end

function get_tx_emulator()
    local pids = {windows.find_process("AndroidEmulator")}
    for _, pid in ipairs(pids) do
        local wnd, subwnd = get_tx_emulator_wnd(pid)
        if wnd then
            print("find tx emulator", pid)
            return wnd, subwnd
        end
    end
end

local function get_mumu_emulator_wnd(pid)
    print("get_mumu_emulator_wnd", pid)
    local wnds = {windows.find_window(pid)}
    for _, wnd in ipairs(wnds) do
        local title = windows.get_window_text(wnd)
        print("get_mumu_emulator_wnd", pid, title)
        if string.find(title, "MuMu模拟器", 1, true) then
            local child = { windows.get_child_window(wnd)}
            for _, subwnd in pairs(child) do
                title = windows.get_window_text(subwnd)
                if string.find(title, "MuMuPlayer", 1, true) then
                    return wnd, subwnd
                end
            end
            return nil
        end
    end    
end

function get_mumu_emulator()
    local pids = {windows.find_process("MuMuPlayer")}
    for _, pid in ipairs(pids) do
        local wnd, subwnd = get_mumu_emulator_wnd(pid)
        if wnd then
            print("find mumu emulator", pid)
            return wnd, subwnd
        end
    end
end

local function change_window_size(emulator)
    local x, y, w, h = windows.get_window_rect(emulator)
    local scaling = 1024 / w;
    local newh =  math.floor(610 / scaling)
    windows.set_window_pos(emulator, x, y, w, newh)
end    

function init()
    emulator, emulator_msg_proc = get_mumu_emulator()
    assert(emulator)
    windows.set_foreground_window(emulator)   
    
    controller.fork("emulator", function ()
        while true do
            _on_frame_begin()
            controller.sleep(70)
            _on_frame_end()
            _set_title()
        end    
    end, true)     
end

function _on_frame_begin()
    now = system.get_time_ms()
    frame = frame + 1
    cur_frame_input = next_frame_input or {}
    next_frame_input = {}
    cur_frame_unique = {}
end

function _on_frame_end()
    for _, msg in ipairs(cur_frame_input) do
        windows.post_message(emulator_msg_proc, table.unpack(msg))
    end
    fps = math.ceil(frame / ((system.get_time_ms() - start_time) / 1000))
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

local function remove_if(t, callback)
    for i = #t, 1, -1 do
        local v = t[i]
        if callback(v) then
            table.remove(t, i)
        end
    end
end

function clear_keyboard(key)
    local k, scan = windows.kb_code(key)
    if not k then
        return
    end
    remove_if(cur_frame_input, function (v)
        return v[1] == 0x0100 and v[2] == k
    end)
    remove_if(next_frame_input, function (v)
        return v[1] == 0x0101 and v[2] == k
    end)
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
        last_manually = frame + fps / 2
        return
    end
    if last_manually >= frame then
        return
    end
    return true
end

function _set_title()
    local title = string.format("[%d][%d]%s", fps, frame, last_state)
    windows.set_console_title(title)
end

function set_state(stat)
    last_state = stat
end
