local emulator = import("script/emulator.lua")

function exec()
    if not emulator.enable() then
        emulator.set_state("disable")
        return
    end
    emulator.set_state("attack")
    emulator.attack()
end
