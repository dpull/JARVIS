local system = require "system"
local opencv = require "opencv"
local windows = require "windows"
local log_tree = require "tree"

local webclientlib = require "webclient"
local webclient = webclientlib.create()

local emulator = import("script/emulator.lua")
local emulator_wnd = emulator.get_tx_emulator()
assert(emulator_wnd)

function init(arg1, ...)
    local img = opencv.window2image(emulator_wnd)
    if not img then
        print("opencv.window2image failed")
        return
    end
    -- img = opencv.cvt_color(img, 6)
    local w, h = opencv.image_size(img)
    if w < 1024 then
        return
    end
    print("image_size1", w, h)
    img = opencv.resize(img, 1024, math.floor(1024/w*h))

    w, h = opencv.image_size(img)
    print("image_size2", w, h)

    -- img = opencv.cvt_color(img, 6)
    -- img = opencv.equalizeHist(img)

    local imgbin = opencv.imencode(img, ".bmp")
    local ok, resp = webclientlib.easy_perform("http://localhost:7788/find?text=" .. webclient:url_encoding("日常"), imgbin)
    print(ok, resp)

    local files = system.get_directory_files("D:\\123")
    for _, path in ipairs(files) do
        local img = opencv.load_image(path)
        img = opencv.cvt_color(img, 1)

        local match = check_buy_state(img)
        if match then
            print(path, match)
        end
    end
end

function tick()
    collectgarbage("collect")
end

function on_event(...)
end

local function load_image(path)
    local img = assert(opencv.load_image(path))
    img = opencv.cvt_color(img, 1)
    return img
end

function check_buy_state(img, img_gray, w, h)
    if not img_buy then
        img_buy = load_image("resource/buy.png")
    end    

    local match, minx, miny, maxx, maxy = opencv.match_template(img, img_buy, 5)
    return match > 0.9, {match, minx, miny, maxx, maxy}    
end
