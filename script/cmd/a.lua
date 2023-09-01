local emulator = import("script/emulator.lua")

function exec()
    if not emulator.enable() then
        return
    end
    emulator.attack()
end
