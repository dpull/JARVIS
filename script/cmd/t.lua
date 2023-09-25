local opencv = require "opencv"

local emulator = import("script/emulator.lua")
local img_can_task = opencv.load_image("resource/cantask.jpg")

local next_step = {
    {1765, 836},
    {1440, 719},
    {1433, 714},
    {965, 1002},
    {1757, 835},
    {1447, 714},
}

local function do_next_step()
    next_step_idx = next_step_idx or 1
    if next_step_idx > #next_step then
        next_step_idx = 1
    end

    local p = next_step[next_step_idx]
    next_step_idx = next_step_idx + 1
    emulator.click_mouse(table.unpack(p))    
end

local function match_template(img, template_img, expect_x, expect_y)
    local _, minx, miny, maxx, maxy = opencv.match_template(img, template_img, 0)
    return minx <= expect_x and maxx >= expect_x and miny <= expect_y and maxy >= expect_y
end

local function check_can_accept()
    local img = opencv.window2image(emulator.emulator)
    if not img then
        print("opencv.window2image failed")
        return
    end

    local x = 1000
    local y = 1550
    if not match_template(img, img_can_task, x, y) then
        return
    end  
    
    print("can accept task")
    emulator.click_mouse(356, 545)  
    return true
end


function exec()
    if not emulator.enable() then
        emulator.set_state("disable")
        return
    end
    emulator.set_state("click")

    local frame = emulator.frame
    if frame % 8 ~= 0 then
        return
    end 
    
    if check_can_accept() then
        return
    end
    do_next_step()
end