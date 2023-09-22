local opencv = require "opencv"
local windows = require "windows"



function init(arg1, ...)
    -- local wnd = notepadwnd()
    -- if wnd then
    --     opencv.window2image(wnd, "F:\\5.bmp")
    -- end

    local img1 = opencv.load_image("F:\\1.png")
    local img2 = opencv.load_image("F:\\2.png")
    local tmpl = opencv.load_image("F:\\3.jpg")

    -- print("img1", opencv.match_template(img1, tmpl, 0, true))
    print("img2", opencv.match_template(img2, tmpl, 0, true))
end

function tick()
    collectgarbage("collect")
end

function on_event(...)
end


function notepadwnd()
    local pids = {windows.find_process("notepad")}
    for _, pid in ipairs(pids) do
        local wnds = {windows.find_window(pid)}
        for _, wnd in ipairs(wnds) do
            return wnd
        end
    end
end