﻿#pragma once

typedef struct player_type player_type;
void do_cmd_save_game(player_type *creature_ptr, int is_autosave);
void do_cmd_save_and_exit(player_type *player_ptr);
