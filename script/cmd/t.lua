local emulator = import("script/emulator.lua")

local next_step = {
    {1612, 597,},
    {1083, 900,},
    {1848, 728,},
    {1583, 608,},
}

local accept = {
    {1140, 732},
    {1087, 442},
    {968, 454},
    {924, 554},
    {1071, 580},
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
    frame = frame + 1

    if not emulator.enable() then
        return
    end
    
    if frame % 2 == 0 then
        do_next_step()
    end 

    if frame % 5 == 0 then
        do_accept_step()
    end 
end