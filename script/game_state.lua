local async =  import("script/coroutine/async.lua")
local emulator = import("script/emulator.lua")
local game_state_cfg = import("script/game_state_cfg.lua")

opt = opt or {}

function set_attack_mode()
    opt = {}
    opt.check_confirm_blue_state = true
    opt.check_counterattack_state = true
end

function set_auto_battle_mode()
    opt = {}
    opt.check_start_battle_state = true
    opt.check_bishop_state = true
    opt.check_social_state = true
    opt.check_confirm_state = true
    opt.check_fight_again_state = true
end

function set_task_mode()
    opt = {}
    opt.check_start_battle_state = true
    opt.check_confirm_state = true
    opt.check_new_task_state = true
    opt.check_new_task_game_state = true
    opt.check_task_next_state = true    
    opt.check_accept_state = true    
    opt.check_finish_state = true    
    opt.check_reward_state = true 
end

function set_buy_mode()
    opt = {}
    opt.check_buy_state = true 
    opt.check_confirm_state = true
end

function get_cfg_state(o, debug)
    o = o or opt
    local config = {}
    for k, v in pairs(o) do
        local conf = game_state_cfg.config[k]
        assert(conf)
        config[k] = conf
    end
    return get_state(config, debug)
end

function get_state(config, debug)
    return async.call("script/task/check_game_state.lua", "run", config, debug)
end

local function get_detail_pos(detail, scaling)
    if not detail then
        return
    end
    local x = math.floor((detail[2] + detail[4]) / 2 / scaling)
    local y = math.floor((detail[3] + detail[5]) / 2 / scaling)
    return x, y
end

function check_and_click(states, details, key)
    if not states[key] then
        return
    end

    local dt = details[key]
    assert(dt)

    local x, y = get_detail_pos(dt, details.scaling)
    print(string.format("check_and_click:%s@[%s, %s]", key, x, y)) 
    emulator.click_mouse(x, y)   

    return true
end