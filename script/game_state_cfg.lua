config = {}


config.check_start_battle_state = {
    img = "resource/auto_battle.png",
    gray = true,
    match = 0.8,
    -- text = "自动战斗",
    roi = {
        {x1 = 0, y1 = 3/4, x2 = 1/2, y2 = 1}
    },
}

config.check_confirm_state = {
    img = "resource/confirm.png",
    match = 0.9,
    -- text = "确定",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1, y2 = 1}
    }
}

config.check_confirm_blue_state = {
    img = "resource/confirm_blue.png",
    match = 0.9,
    -- text = "确定",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1, y2 = 1}
    }
}

config.check_plus_state = {
    img = "resource/plus.png",
    match = 0.9,
    -- text = "确定",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1, y2 = 1}
    }
}

config.check_enter_state = {
    img = "resource/enter.png",
    match = 0.9,
    -- text = "入场",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1, y2 = 1}
    }
}

config.check_fight_again_state = {
    -- img = "resource/fight_again.png",
    -- match = 0.9,
    text = "挑战",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1, y2 = 1}
    }
}

config.check_counterattack_state = {
    img = "resource/counterattack.png",
    gray = true,
    match = 0.95,
    roi = {
        {x1 = 1/4, y1 = 0, x2 = 3/4, y2 = 1/3}
    }
}

config.check_new_task_state = {
    img = "resource/new_task.png",
    match = 0.9,
    roi = {
        {x1 = 0, y1 = 0, x2 = 1/3, y2 = 1}
    }
}

config.check_new_task_game_state = {
    img = "resource/new_task_game.png",
    match = 0.9,
    roi = {
        {x1 = 0, y1 = 0, x2 = 1/3, y2 = 1}
    }
}

config.check_task_next_state = {
    img = "resource/task_next.png",
    match = 0.9,
    roi = {
        {x1 = 1/2, y1 = 1/2, x2 = 1, y2 = 1}
    }
}

config.check_accept_state = {
    img = "resource/accept.png",
    match = 0.9,
    -- text = "接受",
    roi = {
        {x1 = 1/2, y1 = 1/2, x2 = 1, y2 = 1}
    }
}

config.check_finish_state = {
    img = "resource/finish.png",
    match = 0.9,
    -- text = "完成",
    roi = {
        {x1 = 1/2, y1 = 1/2, x2 = 1, y2 = 1}
    }
}

config.check_reward_state = {
    -- img = "resource/reward.png",
    -- match = 0.9,
    text = "领取奖励",
    roi = {
        {x1 = 0, y1 = 1/2, x2 = 1, y2 = 1}
    }
}

config.check_test_state = {
    text = "日常",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1, y2 = 1}
    }
}

config.check_bishop_state = {
    text = "光之魔导师",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1, y2 = 1}
    }
}

config.check_social_state = {
    text = "社交",
    roi = {
        {x1 = 0, y1 = 0, x2 = 1/7, y2 = 1/7}
    }
}
