﻿#pragma once

#define DO_AUTOPICK       0x01
#define DO_AUTODESTROY    0x02
#define DO_DISPLAY        0x04
#define DONT_AUTOPICK     0x08
#define ITEM_DISPLAY      0x10
#define DO_QUERY_AUTOPICK 0x20

/*!
 * @struct autopick_type
 * @brief 自動拾い/破壊設定データの構造体 / A structure type for entry of auto-picker/destroyer
 */
typedef struct {
	concptr name;          /*!< 自動拾い/破壊定義の名称一致基準 / Items which have 'name' as part of its name match */
	concptr insc;          /*!< 対象となったアイテムに自動で刻む内容 / Items will be auto-inscribed as 'insc' */
	BIT_FLAGS flag[2];       /*!< キーワードに関する汎用的な条件フラグ / Misc. keyword to be matched */
	byte action;        /*!< 対象のアイテムを拾う/破壊/放置するかの指定フラグ / Auto-pickup or Destroy or Leave items */
	byte dice;          /*!< 武器のダイス値基準値 / Weapons which have more than 'dice' dice match */
	byte bonus;         /*!< アイテムのボーナス基準値 / Items which have more than 'bonus' magical bonus match */
} autopick_type;

/*
 *  List for auto-picker/destroyer entries
 */
extern int max_autopick;
extern int max_max_autopick;
extern autopick_type *autopick_list;

extern void autopick_load_pref(player_type *player_ptr, bool disp_mes);
extern void process_autopick_file_command(char *buf);
extern concptr autopick_line_from_entry(autopick_type *entry);
extern int is_autopick(player_type *player_ptr, object_type *o_ptr);
extern void autopick_alter_item(player_type *player_ptr, INVENTORY_IDX item, bool destroy);
extern void autopick_delayed_alter(player_type *player_ptr);
extern void autopick_pickup_items(player_type *player_ptr, grid_type *g_ptr);
extern bool autopick_autoregister(player_type *player_ptr, object_type *o_ptr);
extern void do_cmd_edit_autopick(player_type *player_ptr);