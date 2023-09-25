local opencv = require "opencv"
local emulator = import("script/emulator.lua")
local controller = import("script/coroutine/controller.lua")
local game_state = import("script/game_state.lua")
local log_tree = require "tree"

function init()
    game_state.set_task_mode()

    controller.fork("task", function ()
        while true do
            _tick()
            controller.sleep(1000)
        end
    end)   
end

function _tick()
    if not emulator.enable() then
        emulator.set_state("task.disable")
        return
    end

    local ok, st, dt = game_state.get_cfg_state()
    if not ok then
        print("get_cfg_state failed")
        emulator.set_state("task.failed.ok")
        return
    end 
    if not st then
        print("get_cfg_state failed")
        emulator.set_state("task.failed.st")
        return
    end    

    emulator.set_state("task")
    log_tree("st+dt" .. os.time(), {st, dt})
    -- log_tree("st" .. os.time(), st)

    if game_state.check_and_click(st, dt, "check_task_next_state") then
        return
    end 
 
    if game_state.check_and_click(st, dt, "check_confirm_state") then
        return
    end

    if game_state.check_and_click(st, dt, "check_accept_state") then
        return
    end

    if game_state.check_and_click(st, dt, "check_reward_state") then
        return
    end

    if game_state.check_and_click(st, dt, "check_finish_state") then
        return
    end 
    
    if st.check_start_battle_state  or st.check_new_task_game_state then
        if game_state.check_and_click(st, dt, "check_new_task_state") then
            return
        end 
    end 
end
