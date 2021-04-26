﻿#pragma once

#include "system/angband.h"

typedef struct monster_type monster_type;
struct monster_race;
typedef struct monster_death_type {
    MONSTER_IDX m_idx;
    monster_type *m_ptr;
    monster_race *r_ptr;
    bool do_gold;
    bool do_item;
    bool cloned;
    int force_coin;
    bool drop_chosen_item;
    POSITION md_y;
    POSITION md_x;
    u32b mo_mode;
} monster_death_type;

typedef struct player_type player_type;
monster_death_type *initialize_monster_death_type(player_type *player_ptr, monster_death_type *md_ptr, MONSTER_IDX m_idx, bool drop_item);
