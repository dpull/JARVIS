local emulator = import("script/emulator.lua")
local controller = import("script/coroutine/controller.lua")
local game_state = import("script/game_state.lua")

function init()
    counterattack = false
    confirm_blue_state = false

    game_state.set_attack_mode()

    controller.fork("attack", function ()
        while true do
            _tick()
            controller.sleep(50)
        end
    end)
    
    controller.fork("attack-state", function ()
        while true do
            _state()
        end
    end)
end

function _tick()
    if not emulator.enable() then
        emulator.set_state("attack.disable")
        return
    end
    if confirm_blue_state then
        emulator.set_state("attack.finish")
        return
    end
    emulator.set_state("attack")
    
    if counterattack then
        emulator.press_keyboard('h')
        print("counterattack")
    else
        emulator.press_keyboard('j')
    end
end

function _state()
    local ok, st = game_state.get_cfg_state()
    if ok then
        counterattack = st.check_counterattack_state
        confirm_blue_state = confirm_blue_state or st.check_confirm_blue_state
    end
end
