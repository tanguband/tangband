﻿#pragma once

#include "system/angband.h"

typedef struct floor_type floor_type;
typedef struct player_type player_type;
void place_random_stairs(player_type *player_ptr, POSITION y, POSITION x);
bool cave_valid_bold(floor_type *floor_ptr, POSITION y, POSITION x);
