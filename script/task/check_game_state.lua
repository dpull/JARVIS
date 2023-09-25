local opencv = require "opencv"
local system = require "system"
local serialize = require "serialize"
local webclientlib = require "webclient"
local log_tree = require "tree"
local json = require "json"
local emulator = import("script/emulator.lua")

local emulator_wnd = emulator.get_tx_emulator()
assert(emulator_wnd)

webclient = webclient or webclientlib.create()
match_template_img = match_template_img or {}

local function load_image(path)
    local img = assert(opencv.load_image(path), path)
    img = opencv.cvt_color(img, 1)
    return img
end

local function load_gray_image(path)
    local img = assert(opencv.load_image(path))
    local gray_img = assert(opencv.cvt_color(img, 6))
    return gray_img
end

local function get_check_img(img, w, h, roi)
    if img.x1 == 0 and img.y1 == 0 and img.x2 == 1 and img.y2 == 1 then
        return img, 0, 0
    end
    local x1 = math.floor(w * roi.x1)
    local y1 = math.floor(h * roi.y1)
    local x2 = math.floor(w * roi.x2)
    local y2 = math.floor(h * roi.y2)

    local sub_image = opencv.sub_image(img, x1, y1, x2-x1, y2-y1)
    return sub_image, x1, y1
end

local function match_template(config, img, img_gray, w, h)
    local tmpl = match_template_img[config.img]
    if not tmpl then
        tmpl = config.gray and load_gray_image(config.img) or load_image(config.img)
    end

    local check_img_raw = config.gray and img_gray or img
    local last_result;
    for _, roi in pairs(config.roi) do
        local check_img, offset_x, offset_y = get_check_img(check_img_raw, w, h, roi)
        local match, minx, miny, maxx, maxy = opencv.match_template(check_img, tmpl, 5)
        last_result = {match, minx + offset_x, miny + offset_y, maxx + offset_x, maxy + offset_y}
        if match > config.match then
            return true, last_result
        end
    end
    return false, last_result
end

local function slipt(str, sep)
    local tb = {}
    local offset = 1
    while true do
        local pos = str:find(sep, offset)
        if not pos then
            break
        end
        
        local sub = str:sub(offset, pos - 1)
        table.insert(tb, sub)
        offset = pos + #sep
    end
    if offset < #str then
        table.insert(tb, str:sub(offset))
    end
    return tb
end

local function rect_intersect(left1, top1, right1, bottom1, left2, top2, right2, bottom2)
    return not (left1 > right2 or right1 < left2 or top1 > bottom2 or bottom1 < top2)
end

local function get_rect(line, text)
    local word_start;
    local word_end;
    for _, word in pairs(line) do
        if text:find(word.text) then
            if not word_start then
                word_start = word
            end
            word_end = word
        end
    end
    assert(word_start)
    return word_start.left, word_start.top, word_end.right, word_end.bottom
end

local function try_get_rect(line, conf, w, h)
    local left1, top1 = line[1].left, line[1].top
    local right1, bottom1 = line[#line].right, line[#line].bottom

    for _, roi in ipairs(conf.roi) do
        local left2, top2, right2, bottom2 = w * roi.x1, h * roi.y1, w * roi.x2, h * roi.y2
        if rect_intersect(left1, top1, right1, bottom1, left2, top2, right2, bottom2) then
            return true, get_rect(line, conf.text)
        end
    end
    return false
end

local function bench_ocr(config, img_raw, scaling, state, detail, debug)
    local url = "http://localhost:7788/get"
    local post = opencv.imencode(img_raw, ".bmp")
    local ok, resp = webclientlib.easy_perform(url, post)

    if not ok then
        print("ocr faild", resp)
        return
    end

    local lines = json.decode(resp)
    local w, h = opencv.image_size(img_raw)

    if debug then
        print("bench_ocr")
    end

    for _, line in ipairs(lines) do
        local text = ""
        for _, word in ipairs(line) do
            text = text .. word.text
        end

        if debug then
            print("\t\t", text)
        end

        for k, conf in pairs(config) do
            if text:find(conf.text) then
                local match, minx, miny, maxx, maxy = try_get_rect(line, conf, w, h)
                if match then
                    state[k] = true
                    detail[k] = {1, math.floor(minx * scaling), math.floor(miny * scaling), math.floor(maxx * scaling), math.floor(maxy * scaling)}
                else
                    state[k] = false
                    detail[k] = {0, 0, 0, 0, 0}
                end
            end
        end
    end
end

local function window2image()
    local img_raw = opencv.window2image(emulator_wnd)
    if not img_raw then
        print("opencv.window2image failed")
        return
    end
    
    local w, h = opencv.image_size(img_raw)
    if w < 1024 then
        print("opencv.image_size", w, h)
        opencv.save_image(img_raw, "save/window2image_small.bmp")
        return
    end
    
    local scaling = 1024 / w;
    local img = opencv.resize(img_raw, math.floor(scaling*w), math.floor(scaling*h))
    img = opencv.cvt_color(img, 1)

    -- print("window2image", w, h, scaling, math.floor(scaling*w), math.floor(scaling*h))

    local img_gray = opencv.cvt_color(img, 6)
    w, h = opencv.image_size(img) 
    
    return img_raw, img, img_gray, w, h, scaling
end

function run(config, debug)
    local start = system.get_time_ms()
    local img_raw, img, img_gray, w, h, scaling = window2image()
    if not img then
        return
    end

    local state = {}
    local detail = {}

    local ocr_conf = {}
    for k, conf in pairs(config) do
        if conf.img then
            state[k], detail[k] = match_template(conf, img, img_gray, w, h, debug)
        elseif conf.text then
            ocr_conf[k] = conf
        else
            assert(false)
        end            
    end

    if next(ocr_conf) then
        bench_ocr(ocr_conf, img_raw, scaling, state, detail, debug)
    end

    state.cost = system.get_time_ms() - start
    detail.scaling = scaling
    return state, detail
end
