local emulator = import("script/emulator.lua")
local controller = import("script/coroutine/controller.lua")
local game_state = import("script/game_state.lua")
local webclient = import("script/coroutine/webclient.lua")
local log_tree = require "tree"

local function notify_rtx(msg)
    webclient.request("http://9.134.111.125:8090/event", nil, msg)
end

function init()
    game_state.set_auto_battle_mode()

    controller.fork("monitor", function ()
        while true do
            _tick()
            controller.sleep(1000)
        end
    end)
end

function _tick()
    local ok, st, dt = game_state.get_cfg_state()
    if not ok then
        print("get_cfg_state failed")
        emulator.set_state("monitor.failed.ok")
        return
    end

    if not st then
        print("get_cfg_state failed, st nil")
        emulator.set_state("monitor.failed.st")
        return
    end

    emulator.set_state("monitor")

    -- log_tree("state", {st, dt})

    local msg = ""
    if st.check_confirm_state then
        msg = msg .. "出现确定按钮\n"
    end

    if st.check_social_state and not st.check_bishop_state then
        msg = msg .. "没有发现主教\n"
    end
    
    -- if st.check_test_state then
    --     local x, y = game_state.get_detail_pos(dt.check_test_state, dt.scaling)
    --     print("出现日常按钮", x, y) 
    --     emulator.click_mouse(x, y)  
    -- end

    -- if st.check_start_battle_state then
    --     msg = msg .. "没有自动战斗\n"
    -- end

    if st.check_fight_again_state then
        msg = msg .. "副本结束\n"
    end
    
    if #msg > 0 then
        print(msg)
        notify_rtx(msg)
    end
end
