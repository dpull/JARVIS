local opencv = require "opencv"
local emulator = import("script/emulator.lua")
local controller = import("script/coroutine/controller.lua")
local game_state = import("script/game_state.lua")
local webclient = import("script/coroutine/webclient.lua")
local log_tree = require "tree"

function init()
    game_state.set_auto_battle_mode()

    controller.fork("record-click", function ()
        while true do
            _click()
            controller.sleep(1000)
        end
    end)

    controller.fork("record-screenshot", function ()
        while true do
            _screenshot()
            controller.sleep(3000)
        end
    end)    
end

function _click()
    if not emulator.enable() then
        emulator.set_state("record.disable")
        return
    end
    emulator.set_state("record")

    local x, y = emulator.get_click_pos()
    if x then
        print(os.date("%H:%M:%S"), string.format("{%s, %s}", x, y))
    end
end

function _screenshot()
    local img = opencv.window2image(emulator.emulator)
    if not img then
        print("opencv.window2image failed")
        return
    end
    local w, h = opencv.image_size(img)
    img = opencv.resize(img, 1024, math.floor(1024/w*h))
    local img_gray = opencv.cvt_color(img, 6)
    
    local name = "save/" .. os.date("%Y_%m_%d_%H_%M_%S")
    opencv.save_image(img, name .. ".bmp")
    opencv.save_image(img_gray, name .. "_gray.bmp")
end