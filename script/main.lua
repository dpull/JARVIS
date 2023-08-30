local log_tree = require "tree"
local system = require "system"
local VJ = string.byte("J")

function init(arg1, ...)
    log_tree("init", {VJ, arg1, ...})
    wnd = system.find_window(43436)
    system.set_foreground_window(wnd)
end

function tick()
    system.keybd_event(VJ, 0, 0);
    system.keybd_event(VJ, 0, 2);
end