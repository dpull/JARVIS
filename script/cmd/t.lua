local emulator = import("script/emulator.lua")

local next_step = {
    {1581, 984,},
}

local accept = {
}

frame = frame or 0
next_step_idx = next_step_idx or 1
accept_idx = accept_idx or 1

local function do_next_step()
    if next_step_idx > #next_step then
        next_step_idx = 1
    end

    local p = next_step[next_step_idx]
    next_step_idx = next_step_idx + 1
    emulator.click_mouse(table.unpack(p))    
end

local function do_accept_step()
    if accept_idx > #accept then
        accept_idx = 1
    end

    local p = accept[accept_idx]
    accept_idx = accept_idx + 1
    emulator.click_mouse(table.unpack(p))    
end

function exec()
    if not emulator.enable() then
        emulator.set_state("disable")
        return
    end
    emulator.set_state("click")

    frame = frame + 1

    if not emulator.enable() then
        return
    end
    
    if frame % 2 == 0 then
        do_next_step()
    end 

    if frame % 5 == 0 then
       -- do_accept_step()
    end 
end