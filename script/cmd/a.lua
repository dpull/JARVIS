local emulator = import("script/emulator.lua")

function exec()
    local x, y = emulator.get_click_pos()
    if x then
        print(string.format("{%s, %s},", x, y))
    end

    if not emulator.enable() then
        return
    end
    emulator.attack()
end
