local opencv = require "opencv"
local windows = require "windows"



function init(arg1, ...)
    -- local wnd = notepadwnd()
    -- if wnd then
    --     opencv.window2image(wnd, "F:\\5.bmp")
    -- end

    local img = opencv.load_image("F:\\1.png")
    local tmpl = {opencv.load_image("F:\\2.png"), opencv.load_image("F:\\3.png"),  opencv.load_image("F:\\4.png")}

    for i, v in ipairs(tmpl) do
        print("use tmpl", i)
        print("TM_CCORR_NORMED", opencv.match_template(img, v, 3))

        -- print("TM_SQDIFF", opencv.match_template(img, v, 0, true))
        -- print("TM_SQDIFF_NORMED", opencv.match_template(img, v, 1, true))
        -- print("TM_CCORR", opencv.match_template(img, v, 2, true))
        -- print("TM_CCORR_NORMED", opencv.match_template(img, v, 3, true))
        -- print("TM_CCOEFF", opencv.match_template(img, v, 4, true))
        -- print("TM_CCOEFF_NORMED", opencv.match_template(img, v, 5, true))
    end

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