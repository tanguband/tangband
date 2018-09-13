﻿/*!
    @file cmd-spell.c
    @brief 魔法のインターフェイスと発動 / Purpose: Do everything for each spell
    @date 2013/12/31
    @author
    2013 Deskull rearranged comment for Doxygen.
 */

#include "angband.h"
#include "selfinfo.h"
#include "cmd-quaff.h"
#include "spells-summon.h"
#include "realm-arcane.h"
#include "realm-chaos.h"
#include "realm-life.h"
#include "realm-nature.h"
#include "realm-sorcery.h"

/*!
 * @brief
 * 魔法の効果を「キャプション:ダイス＋定数値」のフォーマットで出力する / Generate dice info string such as "foo 2d10"
 * @param str キャプション
 * @param dice ダイス数
 * @param sides ダイス目
 * @param base 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_string_dice(cptr str, int dice, int sides, int base)
{
	/* Fix value */
	if (!dice)
		return format("%s%d", str, base);

	/* Dice only */
	else if (!base)
		return format("%s%dd%d", str, dice, sides);

	/* Dice plus base value */
	else
		return format("%s%dd%d%+d", str, dice, sides, base);
}


/*!
 * @brief 魔法によるダメージを出力する / Generate damage-dice info string such as "dam 2d10"
 * @param dice ダイス数
 * @param sides ダイス目
 * @param base 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_damage(int dice, int sides, int base)
{
	return info_string_dice(_("損傷:", "dam "), dice, sides, base);
}

/*!
 * @brief 魔法の効果時間を出力する / Generate duration info string such as "dur 20+1d20"
 * @param base 固定値
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
cptr info_duration(int base, int sides)
{
	return format(_("期間:%d+1d%d", "dur %d+1d%d"), base, sides);
}

/*!
 * @brief 魔法の効果範囲を出力する / Generate range info string such as "range 5"
 * @param range 効果範囲
 * @return フォーマットに従い整形された文字列
 */
cptr info_range(POSITION range)
{
	return format(_("範囲:%d", "range %d"), range);
}

/*!
 * @brief 魔法による回復量を出力する / Generate heal info string such as "heal 2d8"
 * @param dice ダイス数
 * @param sides ダイス目
 * @param base 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_heal(int dice, int sides, int base)
{
	return info_string_dice(_("回復:", "heal "), dice, sides, base);
}

/*!
 * @brief 魔法効果発動までの遅延ターンを出力する / Generate delay info string such as "delay 15+1d15"
 * @param base 固定値
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
cptr info_delay(int base, int sides)
{
	return format(_("遅延:%d+1d%d", "delay %d+1d%d"), base, sides);
}


/*!
 * @brief 魔法によるダメージを出力する(固定値＆複数回処理) / Generate multiple-damage info string such as "dam 25 each"
 * @param dam 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_multi_damage(HIT_POINT dam)
{
	return format(_("損傷:各%d", "dam %d each"), dam);
}


/*!
 * @brief 魔法によるダメージを出力する(ダイスのみ＆複数回処理) / Generate multiple-damage-dice info string such as "dam 5d2 each"
 * @param dice ダイス数
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
cptr info_multi_damage_dice(int dice, int sides)
{
	return format(_("損傷:各%dd%d", "dam %dd%d each"), dice, sides);
}

/*!
 * @brief 魔法による一般的な効力値を出力する（固定値） / Generate power info string such as "power 100"
 * @param power 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_power(int power)
{
	return format(_("効力:%d", "power %d"), power);
}


/*!
 * @brief 魔法による一般的な効力値を出力する（ダイス値） / Generate power info string such as "power 100"
 * @param dice ダイス数
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
/*
 * Generate power info string such as "power 1d100"
 */
cptr info_power_dice(int dice, int sides)
{
	return format(_("効力:%dd%d", "power %dd%d"), dice, sides);
}


/*!
 * @brief 魔法の効果半径を出力する / Generate radius info string such as "rad 100"
 * @param rad 効果半径
 * @return フォーマットに従い整形された文字列
 */
cptr info_radius(int rad)
{
	return format(_("半径:%d", "rad %d"), rad);
}


/*!
 * @brief 魔法効果の限界重量を出力する / Generate weight info string such as "max wgt 15"
 * @param weight 最大重量
 * @return フォーマットに従い整形された文字列
 */
cptr info_weight(int weight)
{
#ifdef JP
	return format("最大重量:%d.%dkg", lbtokg1(weight), lbtokg2(weight));
#else
	return format("max wgt %d", weight/10);
#endif
}


/*!
 * @brief 歌の開始を処理する / Start singing if the player is a Bard 
 * @param spell 領域魔法としてのID
 * @param song 魔法効果のID
 * @return なし
 */
static void start_singing(SPELL_IDX spell, MAGIC_NUM1 song)
{
	/* Remember the song index */
	SINGING_SONG_EFFECT(p_ptr) = (MAGIC_NUM1)song;

	/* Remember the index of the spell which activated the song */
	SINGING_SONG_ID(p_ptr) = (MAGIC_NUM2)spell;


	/* Now the player is singing */
	set_action(ACTION_SING);


	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw status bar */
	p_ptr->redraw |= (PR_STATUS);
}

/*!
 * @brief 歌の停止を処理する / Stop singing if the player is a Bard 
 * @return なし
 */
void stop_singing(void)
{
	if (p_ptr->pclass != CLASS_BARD) return;

 	/* Are there interupted song? */
	if (INTERUPTING_SONG_EFFECT(p_ptr))
	{
		/* Forget interupted song */
		INTERUPTING_SONG_EFFECT(p_ptr) = MUSIC_NONE;
		return;
	}

	/* The player is singing? */
	if (!SINGING_SONG_EFFECT(p_ptr)) return;

	/* Hack -- if called from set_action(), avoid recursive loop */
	if (p_ptr->action == ACTION_SING) set_action(ACTION_NONE);

	/* Message text of each song or etc. */
	do_spell(REALM_MUSIC, SINGING_SONG_ID(p_ptr), SPELL_STOP);

	SINGING_SONG_EFFECT(p_ptr) = MUSIC_NONE;
	SINGING_SONG_ID(p_ptr) = 0;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw status bar */
	p_ptr->redraw |= (PR_STATUS);
}







/*!
 * @brief 暗黒領域魔法の各処理を行う
 * @param spell 魔法ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST時はNULL文字列を返す。
 */
static cptr do_death_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	static const char s_dam[] = _("損傷:", "dam ");
	static const char s_random[] = _("ランダム", "random");

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("無生命感知", "Detect Unlife");
		if (desc) return _("近くの生命のないモンスターを感知する。", "Detects all nonliving monsters in your vicinity.");
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_nonliving(rad);
			}
		}
		break;

	case 1:
		if (name) return _("呪殺弾", "Malediction");
		if (desc) return _("ごく小さな邪悪な力を持つボールを放つ。善良なモンスターには大きなダメージを与える。", 
			"Fires a tiny ball of evil power which hurts good monsters greatly.");
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;
			int rad = 0;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				/*
				 * A radius-0 ball may (1) be aimed at
				 * objects etc., and will affect them;
				 * (2) may be aimed at ANY visible
				 * monster, unlike a 'bolt' which must
				 * travel to the monster.
				 */

				fire_ball(GF_HELL_FIRE, dir, damroll(dice, sides), rad);

				if (one_in_(5))
				{
					/* Special effect first */
					int effect = randint1(1000);

					if (effect == 666)
						fire_ball_hide(GF_DEATH_RAY, dir, plev * 200, 0);
					else if (effect < 500)
						fire_ball_hide(GF_TURN_ALL, dir, plev, 0);
					else if (effect < 800)
						fire_ball_hide(GF_OLD_CONF, dir, plev, 0);
					else
						fire_ball_hide(GF_STUN, dir, plev, 0);
				}
			}
		}
		break;

	case 2:
		if (name) return _("邪悪感知", "Detect Evil");
		if (desc) return _("近くの邪悪なモンスターを感知する。", "Detects all evil monsters in your vicinity.");
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_evil(rad);
			}
		}
		break;

	case 3:
		if (name) return _("悪臭雲", "Stinking Cloud");
		if (desc) return _("毒の球を放つ。", "Fires a ball of poison.");
    
		{
			HIT_POINT dam = 10 + plev / 2;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_POIS, dir, dam, rad);
			}
		}
		break;

	case 4:
		if (name) return _("黒い眠り", "Black Sleep");
		if (desc) return _("1体のモンスターを眠らせる。抵抗されると無効。", "Attempts to sleep a monster.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				sleep_monster(dir, plev);
			}
		}
		break;

	case 5:
		if (name) return _("耐毒", "Resist Poison");
		if (desc) return _("一定時間、毒への耐性を得る。装備による耐性に累積する。", 
			"Gives resistance to poison. This resistance can be added to which from equipment for more powerful resistance.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_pois(randint1(base) + base, FALSE);
			}
		}
		break;

	case 6:
		if (name) return _("恐慌", "Horrify");
		if (desc) return _("モンスター1体を恐怖させ、朦朧させる。抵抗されると無効。", "Attempts to scare and stun a monster.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fear_monster(dir, power);
				stun_monster(dir, power);
			}
		}
		break;

	case 7:
		if (name) return _("アンデッド従属", "Enslave Undead");
		if (desc) return _("アンデッド1体を魅了する。抵抗されると無効。", "Attempts to charm an undead monster.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				control_one_undead(dir, power);
			}
		}
		break;

	case 8:
		if (name) return _("エントロピーの球", "Orb of Entropy");
		if (desc) return _("生命のある者のHPと最大HP双方にダメージを与える効果のある球を放つ。", "Fires a ball which damages to both HP and MaxHP of living monsters.");

		{
			int dice = 3;
			int sides = 6;
			int rad = (plev < 30) ? 2 : 3;
			int base;

			if (p_ptr->pclass == CLASS_MAGE ||
			    p_ptr->pclass == CLASS_HIGH_MAGE ||
			    p_ptr->pclass == CLASS_SORCERER)
				base = plev + plev / 2;
			else
				base = plev + plev / 4;


			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_HYPODYNAMIA, dir, damroll(dice, sides) + base, rad);
			}
		}
		break;

	case 9:
		if (name) return _("地獄の矢", "Nether Bolt");
		if (desc) return _("地獄のボルトもしくはビームを放つ。", "Fires a bolt or beam of nether.");
    
		{
			int dice = 8 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_NETHER, dir, damroll(dice, sides));
			}
		}
		break;

	case 10:
		if (name) return _("殺戮雲", "Cloud kill");
		if (desc) return _("自分を中心とした毒の球を発生させる。", "Generate a ball of poison centered on you.");
    
		{
			HIT_POINT dam = (30 + plev) * 2;
			int rad = plev / 10 + 2;

			if (info) return info_damage(0, 0, dam/2);

			if (cast)
			{
				project(0, rad, p_ptr->y, p_ptr->x, dam, GF_POIS, PROJECT_KILL | PROJECT_ITEM, -1);
			}
		}
		break;

	case 11:
		if (name) return _("モンスター消滅", "Genocide One");
		if (desc) return _("モンスター1体を消し去る。経験値やアイテムは手に入らない。抵抗されると無効。", "Attempts to vanish a monster.");
    
		{
			int power = plev + 50;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball_hide(GF_GENOCIDE, dir, power, 0);
			}
		}
		break;

	case 12:
		if (name) return _("毒の刃", "Poison Branding");
		if (desc) return _("武器に毒の属性をつける。", "Makes current weapon poison branded.");
    
		{
			if (cast)
			{
				brand_weapon(3);
			}
		}
		break;

	case 13:
		if (name) return _("吸血の矢", "Vampiric Bolt");
		if (desc) return _("ボルトによりモンスター1体から生命力を吸いとる。吸いとった生命力によって満腹度が上がる。", 
			"Absorbs some HP from a monster and gives them to you by bolt. You will also gain nutritional sustenance from this.");
    
		{
			int dice = 1;
			int sides = plev * 2;
			int base = plev * 2;

			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				HIT_POINT dam = base + damroll(dice, sides);

				if (!get_aim_dir(&dir)) return NULL;

				if (hypodynamic_bolt(dir, dam))
				{
					chg_virtue(V_SACRIFICE, -1);
					chg_virtue(V_VITALITY, -1);

					hp_player(dam);

					/*
					 * Gain nutritional sustenance:
					 * 150/hp drained
					 *
					 * A Food ration gives 5000
					 * food points (by contrast)
					 * Don't ever get more than
					 * "Full" this way But if we
					 * ARE Gorged, it won't cure
					 * us
					 */
					dam = p_ptr->food + MIN(5000, 100 * dam);

					/* Not gorged already */
					if (p_ptr->food < PY_FOOD_MAX)
						set_food(dam >= PY_FOOD_MAX ? PY_FOOD_MAX - 1 : dam);
				}
			}
		}
		break;

	case 14:
		if (name) return _("反魂の術", "Animate dead");
		if (desc) return _("周囲の死体や骨を生き返す。", "Resurrects nearby corpse and skeletons. And makes these your pets.");
    
		{
			if (cast)
			{
				animate_dead(0, p_ptr->y, p_ptr->x);
			}
		}
		break;

	case 15:
		if (name) return _("抹殺", "Genocide");
		if (desc) return _("指定した文字のモンスターを現在の階から消し去る。抵抗されると無効。", 
			"Eliminates an entire class of monster, exhausting you.  Powerful or unique monsters may resist.");
    
		{
			int power = plev+50;

			if (info) return info_power(power);

			if (cast)
			{
				symbol_genocide(power, TRUE);
			}
		}
		break;

	case 16:
		if (name) return _("狂戦士化", "Berserk");
		if (desc) return _("狂戦士化し、恐怖を除去する。", "Gives bonus to hit and HP, immunity to fear for a while. But decreases AC.");
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_shero(randint1(base) + base, FALSE);
				hp_player(30);
				set_afraid(0);
			}
		}
		break;

	case 17:
		if (name) return _("悪霊召喚", "Invoke Spirits");
		if (desc) return _("ランダムで様々な効果が起こる。", "Causes random effects.");
    
		{
			if (info) return s_random;

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				cast_invoke_spirits(dir);
			}
		}
		break;

	case 18:
		if (name) return _("暗黒の矢", "Dark Bolt");
		if (desc) return _("暗黒のボルトもしくはビームを放つ。", "Fires a bolt or beam of darkness.");
    
		{
			int dice = 4 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_DARK, dir, damroll(dice, sides));
			}
		}
		break;

	case 19:
		if (name) return _("狂乱戦士", "Battle Frenzy");
		if (desc) return _("狂戦士化し、恐怖を除去し、加速する。", 
			"Gives another bonus to hit and HP, immunity to fear for a while. Hastes you. But decreases AC.");
    
		{
			int b_base = 25;
			int sp_base = plev / 2;
			int sp_sides = 20 + plev / 2;

			if (info) return info_duration(b_base, b_base);

			if (cast)
			{
				set_shero(randint1(25) + 25, FALSE);
				hp_player(30);
				set_afraid(0);
				set_fast(randint1(sp_sides) + sp_base, FALSE);
			}
		}
		break;

	case 20:
		if (name) return _("吸血の刃", "Vampiric Branding");
		if (desc) return _("武器に吸血の属性をつける。", "Makes current weapon Vampiric.");
    
		{
			if (cast)
			{
				brand_weapon(4);
			}
		}
		break;

	case 21:
		if (name) return _("吸血の連矢", "Vampiric Bolts");
		if (desc) return _("3連射のボルトによりモンスター1体から生命力を吸いとる。吸いとった生命力によって体力が回復する。", 
			"Fires 3 bolts. Each of the bolts absorbs some HP from a monster and gives them to you.");
		{
			HIT_POINT dam = 100;

			if (info) return format("%s3*%d", s_dam, dam);

			if (cast)
			{
				int i;

				if (!get_aim_dir(&dir)) return NULL;

				chg_virtue(V_SACRIFICE, -1);
				chg_virtue(V_VITALITY, -1);

				for (i = 0; i < 3; i++)
				{
					if (hypodynamic_bolt(dir, dam))
						hp_player(dam);
				}
			}
		}
		break;

	case 22:
		if (name) return _("死の言魂", "Nether Wave");
		if (desc) return _("視界内の生命のあるモンスターにダメージを与える。", "Damages all living monsters in sight.");
    
		{
			int sides = plev * 3;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_living(randint1(sides));
			}
		}
		break;

	case 23:
		if (name) return _("暗黒の嵐", "Darkness Storm");
		if (desc) return _("巨大な暗黒の球を放つ。", "Fires a huge ball of darkness.");
    
		{
			HIT_POINT dam = 100 + plev * 2;
			int rad = 4;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_DARK, dir, dam, rad);
			}
		}
		break;

	case 24:
		if (name) return _("死の光線", "Death Ray");
		if (desc) return _("死の光線を放つ。", "Fires a beam of death.");
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				death_ray(dir, plev);
			}
		}
		break;

	case 25:
		if (name) return _("死者召喚", "Raise the Dead");
		if (desc) return _("1体のアンデッドを召喚する。", "Summons an undead monster.");
    
		{
			if (cast)
			{
				int type;
				bool pet = one_in_(3);
				u32b flg = 0L;

				type = (plev > 47 ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD);

				if (!pet || (pet && (plev > 24) && one_in_(3)))
					flg |= PM_ALLOW_GROUP;

				if (pet) flg |= PM_FORCE_PET;
				else flg |= (PM_ALLOW_UNIQUE | PM_NO_PET);

				if (summon_specific((pet ? -1 : 0), p_ptr->y, p_ptr->x, (plev * 3) / 2, type, flg))
				{
					msg_print(_("冷たい風があなたの周りに吹き始めた。それは腐敗臭を運んでいる...",
								"Cold winds begin to blow around you, carrying with them the stench of decay..."));


					if (pet)
					{
						msg_print(_("古えの死せる者共があなたに仕えるため土から甦った！",
									"Ancient, long-dead forms arise from the ground to serve you!"));
					}
					else
					{
						msg_print(_("死者が甦った。眠りを妨げるあなたを罰するために！",
									"'The dead arise... to punish you for disturbing them!'"));
					}

					chg_virtue(V_UNLIFE, 1);
				}
			}
		}
		break;

	case 26:
		if (name) return _("死者の秘伝", "Esoteria");
		if (desc) return _("アイテムを1つ識別する。レベルが高いとアイテムの能力を完全に知ることができる。",
			"Identifies an item. Or *identifies* an item at higher level.");
    
		{
			if (cast)
			{
				if (randint1(50) > plev)
				{
					if (!ident_spell(FALSE)) return NULL;
				}
				else
				{
					if (!identify_fully(FALSE)) return NULL;
				}
			}
		}
		break;

	case 27:
		if (name) return _("吸血鬼変化", "Polymorph Vampire");
		if (desc) return _("一定時間、吸血鬼に変化する。変化している間は本来の種族の能力を失い、代わりに吸血鬼としての能力を得る。", 
			"Mimic a vampire for a while. Loses abilities of original race and gets abilities as a vampire.");
    
		{
			int base = 10 + plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_mimic(base + randint1(base), MIMIC_VAMPIRE, FALSE);
			}
		}
		break;

	case 28:
		if (name) return _("経験値復活", "Restore Life");
		if (desc) return _("失った経験値を回復する。", "Restore lost experience.");
    
		{
			if (cast)
			{
				restore_level();
			}
		}
		break;

	case 29:
		if (name) return _("周辺抹殺", "Mass Genocide");
		if (desc) return _("自分の周囲にいるモンスターを現在の階から消し去る。抵抗されると無効。", 
			"Eliminates all nearby monsters, exhausting you.  Powerful or unique monsters may be able to resist.");
    
		{
			int power = plev + 50;

			if (info) return info_power(power);

			if (cast)
			{
				mass_genocide(power, TRUE);
			}
		}
		break;

	case 30:
		if (name) return _("地獄の劫火", "Hellfire");
		if (desc) return _("邪悪な力を持つ宝珠を放つ。善良なモンスターには大きなダメージを与える。", 
			"Fires a powerful ball of evil power. Hurts good monsters greatly.");
    
		{
			HIT_POINT dam = 666;
			int rad = 3;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_HELL_FIRE, dir, dam, rad);
				take_hit(DAMAGE_USELIFE, 20 + randint1(30), _("地獄の劫火の呪文を唱えた疲労", "the strain of casting Hellfire"), -1);
			}
		}
		break;

	case 31:
		if (name) return _("幽体化", "Wraithform");
		if (desc) return _("一定時間、壁を通り抜けることができ受けるダメージが軽減される幽体の状態に変身する。", 
			"Becomes wraith form which gives ability to pass walls and makes all damages half.");
    
		{
			int base = plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_wraith_form(randint1(base) + base, FALSE);
			}
		}
		break;
	}

	return "";
}


/*!
 * @brief トランプ領域魔法の各処理を行う
 * @param spell 魔法ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST時はNULL文字列を返す。
 */
static cptr do_trump_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
	bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;
	static const char s_random[] = _("ランダム", "random");

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("ショート・テレポート", "Phase Door");
		if (desc) return _("近距離のテレポートをする。", "Teleport short distance.");
    
		{
			POSITION range = 10;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 1:
		if (name) return _("蜘蛛のカード", "Trump Spiders");
		if (desc) return _("蜘蛛を召喚する。", "Summons spiders.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたは蜘蛛のカードに集中する...", "You concentrate on the trump of an spider..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_SPIDER, PM_ALLOW_GROUP))
				{
					if (fail)
					{
						msg_print(_("召喚された蜘蛛は怒っている！", "The summoned spiders get angry!"));
					}
				}
			}
		}
		break;

	case 2:
		if (name) return _("シャッフル", "Shuffle");
		if (desc) return _("カードの占いをする。", "Causes random effects.");
    
		{
			if (info) return s_random;

			if (cast)
			{
				cast_shuffle();
			}
		}
		break;

	case 3:
		if (name) return _("フロア・リセット", "Reset Recall");
		if (desc) return _("最深階を変更する。", "Resets the 'deepest' level for recall spell.");
    
		{
			if (cast)
			{
				if (!reset_recall()) return NULL;
			}
		}
		break;

	case 4:
		if (name) return _("テレポート", "Teleport");
		if (desc) return _("遠距離のテレポートをする。", "Teleport long distance.");
    
		{
			POSITION range = plev * 4;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 5:
		if (name) return _("感知のカード", "Trump Spying");
		if (desc) return _("一定時間、テレパシー能力を得る。", "Gives telepathy for a while.");
    
		{
			int base = 25;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_tim_esp(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 6:
		if (name) return _("テレポート・モンスター", "Teleport Away");
		if (desc) return _("モンスターをテレポートさせるビームを放つ。抵抗されると無効。", "Teleports all monsters on the line away unless resisted.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_AWAY_ALL, dir, power);
			}
		}
		break;

	case 7:
		if (name) return _("動物のカード", "Trump Animals");
		if (desc) return _("1体の動物を召喚する。", "Summons an animal.");
    
		{
			if (cast || fail)
			{
				int type = (!fail ? SUMMON_ANIMAL_RANGER : SUMMON_ANIMAL);
				msg_print(_("あなたは動物のカードに集中する...", "You concentrate on the trump of an animal..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, type, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚された動物は怒っている！", "The summoned animal gets angry!"));
					}
				}
			}
		}
		break;

	case 8:
		if (name) return _("移動のカード", "Trump Reach");
		if (desc) return _("アイテムを自分の足元へ移動させる。", "Pulls a distant item close to you.");
    
		{
			int weight = plev * 15;

			if (info) return info_weight(weight);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fetch(dir, weight, FALSE);
			}
		}
		break;

	case 9:
		if (name) return _("カミカゼのカード", "Trump Kamikaze");
		if (desc) return _("複数の爆発するモンスターを召喚する。", "Summons monsters which explode by itself.");
    
		{
			if (cast || fail)
			{
				int x, y;
				int type;

				if (cast)
				{
					if (!target_set(TARGET_KILL)) return NULL;
					x = target_col;
					y = target_row;
				}
				else
				{
					/* Summons near player when failed */
					x = p_ptr->x;
					y = p_ptr->y;
				}

				if (p_ptr->pclass == CLASS_BEASTMASTER)
					type = SUMMON_KAMIKAZE_LIVING;
				else
					type = SUMMON_KAMIKAZE;

				msg_print(_("あなたはカミカゼのカードに集中する...", "You concentrate on several trumps at once..."));
				if (trump_summoning(2 + randint0(plev / 7), !fail, y, x, 0, type, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚されたモンスターは怒っている！", "The summoned creatures get angry!"));
					}
				}
			}
		}
		break;

	case 10:
		if (name) return _("幻霊召喚", "Phantasmal Servant");
		if (desc) return _("1体の幽霊を召喚する。", "Summons a ghost.");
    
		{
			/* Phantasmal Servant is not summoned as enemy when failed */
			if (cast)
			{
				int summon_lev = plev * 2 / 3 + randint1(plev / 2);

				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, (summon_lev * 3 / 2), SUMMON_PHANTOM, 0L))
				{
					msg_print(_("御用でございますか、御主人様？", "'Your wish, master?'"));
				}
			}
		}
		break;

	case 11:
		if (name) return _("スピード・モンスター", "Haste Monster");
		if (desc) return _("モンスター1体を加速させる。", "Hastes a monster.");
    
		{
			if (cast)
			{
				bool result;

				/* Temporary enable target_pet option */
				bool old_target_pet = target_pet;
				target_pet = TRUE;

				result = get_aim_dir(&dir);

				/* Restore target_pet option */
				target_pet = old_target_pet;

				if (!result) return NULL;

				speed_monster(dir, plev);
			}
		}
		break;

	case 12:
		if (name) return _("テレポート・レベル", "Teleport Level");
		if (desc) return _("瞬時に上か下の階にテレポートする。", "Teleport to up or down stairs in a moment.");
    
		{
			if (cast)
			{
				if (!get_check(_("本当に他の階にテレポートしますか？", "Are you sure? (Teleport Level)"))) return NULL;
				teleport_level(0);
			}
		}
		break;

	case 13:
		if (name) return _("次元の扉", "Dimension Door");
		if (desc) return _("短距離内の指定した場所にテレポートする。", "Teleport to given location.");
    
		{
			POSITION range = plev / 2 + 10;

			if (info) return info_range(range);

			if (cast)
			{
				msg_print(_("次元の扉が開いた。目的地を選んで下さい。", "You open a dimensional gate. Choose a destination."));
				if (!dimension_door()) return NULL;
			}
		}
		break;

	case 14:
		if (name) return _("帰還の呪文", "Word of Recall");
		if (desc) return _("地上にいるときはダンジョンの最深階へ、ダンジョンにいるときは地上へと移動する。",
			"Recalls player from dungeon to town, or from town to the deepest level of dungeon.");
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			if (cast)
			{
				if (!word_of_recall()) return NULL;
			}
		}
		break;

	case 15:
		if (name) return _("怪物追放", "Banish");
		if (desc) return _("視界内の全てのモンスターをテレポートさせる。抵抗されると無効。", "Teleports all monsters in sight away unless resisted.");
    
		{
			int power = plev * 4;

			if (info) return info_power(power);

			if (cast)
			{
				banish_monsters(power);
			}
		}
		break;

	case 16:
		if (name) return _("位置交換のカード", "Swap Position");
		if (desc) return _("1体のモンスターと位置を交換する。", "Swap positions of you and a monster.");
    
		{
			if (cast)
			{
				bool result;

				/* HACK -- No range limit */
				project_length = -1;

				result = get_aim_dir(&dir);

				/* Restore range to default */
				project_length = 0;

				if (!result) return NULL;

				teleport_swap(dir);
			}
		}
		break;

	case 17:
		if (name) return _("アンデッドのカード", "Trump Undead");
		if (desc) return _("1体のアンデッドを召喚する。", "Summons an undead monster.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたはアンデッドのカードに集中する...", "You concentrate on the trump of an undead creature..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_UNDEAD, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚されたアンデッドは怒っている！", "The summoned undead creature gets angry!"));
					}
				}
			}
		}
		break;

	case 18:
		if (name) return _("爬虫類のカード", "Trump Reptiles");
		if (desc) return _("1体のヒドラを召喚する。", "Summons a hydra.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたは爬虫類のカードに集中する...", "You concentrate on the trump of a reptile..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_HYDRA, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚された爬虫類は怒っている！", "The summoned reptile gets angry!"));
					}
				}
			}
		}
		break;

	case 19:
		if (name) return _("モンスターのカード", "Trump Monsters");
		if (desc) return _("複数のモンスターを召喚する。", "Summons some monsters.");
    
		{
			if (cast || fail)
			{
				int type;
				msg_print(_("あなたはモンスターのカードに集中する...", "You concentrate on several trumps at once..."));
				if (p_ptr->pclass == CLASS_BEASTMASTER)
					type = SUMMON_LIVING;
				else
					type = 0;

				if (trump_summoning((1 + (plev - 15)/ 10), !fail, p_ptr->y, p_ptr->x, 0, type, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚されたモンスターは怒っている！", "The summoned creatures get angry!"));
					}
				}

			}
		}
		break;

	case 20:
		if (name) return _("ハウンドのカード", "Trump Hounds");
		if (desc) return _("1グループのハウンドを召喚する。", "Summons a group of hounds.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたはハウンドのカードに集中する...", "You concentrate on the trump of a hound..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_HOUND, PM_ALLOW_GROUP))
				{
					if (fail)
					{
						msg_print(_("召喚されたハウンドは怒っている！", "The summoned hounds get angry!"));
					}
				}
			}
		}
		break;

	case 21:
		if (name) return _("トランプの刃", "Trump Branding");
		if (desc) return _("武器にトランプの属性をつける。", "Makes current weapon a Trump weapon.");
    
		{
			if (cast)
			{
				brand_weapon(5);
			}
		}
		break;

	case 22:
		if (name) return _("人間トランプ", "Living Trump");
		if (desc) return _("ランダムにテレポートする突然変異か、自分の意思でテレポートする突然変異が身につく。", 
			"Gives mutation which makes you teleport randomly or makes you able to teleport at will.");
    
		{
			if (cast)
			{
				int mutation;

				if (one_in_(7))
					/* Teleport control */
					mutation = 12;
				else
					/* Random teleportation (uncontrolled) */
					mutation = 77;

				/* Gain the mutation */
				if (gain_random_mutation(mutation))
				{
					msg_print(_("あなたは生きているカードに変わった。", "You have turned into a Living Trump."));
				}
			}
		}
		break;

	case 23:
		if (name) return _("サイバーデーモンのカード", "Trump Cyberdemon");
		if (desc) return _("1体のサイバーデーモンを召喚する。", "Summons a cyber demon.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたはサイバーデーモンのカードに集中する...", "You concentrate on the trump of a Cyberdemon..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_CYBER, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚されたサイバーデーモンは怒っている！", "The summoned Cyberdemon gets angry!"));
					}
				}
			}
		}
		break;

	case 24:
		if (name) return _("予見のカード", "Trump Divination");
		if (desc) return _("近くの全てのモンスター、罠、扉、階段、財宝、そしてアイテムを感知する。",
			"Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.");
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_all(rad);
			}
		}
		break;

	case 25:
		if (name) return _("知識のカード", "Trump Lore");
		if (desc) return _("アイテムの持つ能力を完全に知る。", "*Identifies* an item.");
    
		{
			if (cast)
			{
				if (!identify_fully(FALSE)) return NULL;
			}
		}
		break;

	case 26:
		if (name) return _("回復モンスター", "Heal Monster");
		if (desc) return _("モンスター1体の体力を回復させる。", "Heal a monster.");
    
		{
			int heal = plev * 10 + 200;

			if (info) return info_heal(0, 0, heal);

			if (cast)
			{
				bool result;

				/* Temporary enable target_pet option */
				bool old_target_pet = target_pet;
				target_pet = TRUE;

				result = get_aim_dir(&dir);

				/* Restore target_pet option */
				target_pet = old_target_pet;

				if (!result) return NULL;

				heal_monster(dir, heal);
			}
		}
		break;

	case 27:
		if (name) return _("ドラゴンのカード", "Trump Dragon");
		if (desc) return _("1体のドラゴンを召喚する。", "Summons a dragon.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたはドラゴンのカードに集中する...", "You concentrate on the trump of a dragon..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_DRAGON, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚されたドラゴンは怒っている！", "The summoned dragon gets angry!"));
					}
				}
			}
		}
		break;

	case 28:
		if (name) return _("隕石のカード", "Trump Meteor");
		if (desc) return _("自分の周辺に隕石を落とす。", "Makes meteor balls fall down to nearby random locations.");
    
		{
			HIT_POINT dam = plev * 2;
			int rad = 2;

			if (info) return info_multi_damage(dam);

			if (cast)
			{
				cast_meteor(dam, rad);
			}
		}
		break;

	case 29:
		if (name) return _("デーモンのカード", "Trump Demon");
		if (desc) return _("1体の悪魔を召喚する。", "Summons a demon.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたはデーモンのカードに集中する...", "You concentrate on the trump of a demon..."));
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_DEMON, 0L))
				{
					if (fail)
					{
						msg_print(_("召喚されたデーモンは怒っている！", "The summoned demon gets angry!"));
					}
				}
			}
		}
		break;

	case 30:
		if (name) return _("地獄のカード", "Trump Greater Undead");
		if (desc) return _("1体の上級アンデッドを召喚する。", "Summons a greater undead.");
    
		{
			if (cast || fail)
			{
				msg_print(_("あなたは強力なアンデッドのカードに集中する...", "You concentrate on the trump of a greater undead being..."));
				/* May allow unique depend on level and dice roll */
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, SUMMON_HI_UNDEAD, PM_ALLOW_UNIQUE))
				{
					if (fail)
					{
						msg_print(_("召喚された上級アンデッドは怒っている！", "The summoned greater undead creature gets angry!"));
					}
				}
			}
		}
		break;

	case 31:
		if (name) return _("古代ドラゴンのカード", "Trump Ancient Dragon");
		if (desc) return _("1体の古代ドラゴンを召喚する。", "Summons an ancient dragon.");
    
		{
			if (cast)
			{
				int type;

				if (p_ptr->pclass == CLASS_BEASTMASTER)
					type = SUMMON_HI_DRAGON_LIVING;
				else
					type = SUMMON_HI_DRAGON;

				msg_print(_("あなたは古代ドラゴンのカードに集中する...", "You concentrate on the trump of an ancient dragon..."));
				/* May allow unique depend on level and dice roll */
				if (trump_summoning(1, !fail, p_ptr->y, p_ptr->x, 0, type, PM_ALLOW_UNIQUE))
				{
					if (fail)
					{
						msg_print(_("召喚された古代ドラゴンは怒っている！", "The summoned ancient dragon gets angry!"));
					}
				}
			}
		}
		break;
	}

	return "";
}


/*!
 * @brief 匠領域魔法の各処理を行う
 * @param spell 魔法ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST時はNULL文字列を返す。
 */
static cptr do_craft_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("赤外線視力", "Infravision");
		if (desc) return _("一定時間、赤外線視力が増強される。", "Gives infravision for a while.");
    
		{
			int base = 100;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_infra(base + randint1(base), FALSE);
			}
		}
		break;

	case 1:
		if (name) return _("回復力強化", "Regeneration");
		if (desc) return _("一定時間、回復力が増強される。", "Gives regeneration ability for a while.");
    
		{
			int base = 80;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_regen(base + randint1(base), FALSE);
			}
		}
		break;

	case 2:
		if (name) return _("空腹充足", "Satisfy Hunger");
		if (desc) return _("満腹になる。", "Satisfies hunger.");
    
		{
			if (cast)
			{
				set_food(PY_FOOD_MAX - 1);
			}
		}
		break;

	case 3:
		if (name) return _("耐冷気", "Resist Cold");
		if (desc) return _("一定時間、冷気への耐性を得る。装備による耐性に累積する。", 
			"Gives resistance to cold. This resistance can be added to which from equipment for more powerful resistance.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_cold(randint1(base) + base, FALSE);
			}
		}
		break;

	case 4:
		if (name) return _("耐火炎", "Resist Fire");
		if (desc) return _("一定時間、炎への耐性を得る。装備による耐性に累積する。", 
			"Gives resistance to fire. This resistance can be added to which from equipment for more powerful resistance.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_fire(randint1(base) + base, FALSE);
			}
		}
		break;

	case 5:
		if (name) return _("士気高揚", "Heroism");
		if (desc) return _("一定時間、ヒーロー気分になる。", "Removes fear, and gives bonus to hit and 10 more HP for a while.");
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_hero(randint1(base) + base, FALSE);
				hp_player(10);
				set_afraid(0);
			}
		}
		break;

	case 6:
		if (name) return _("耐電撃", "Resist Lightning");
		if (desc) return _("一定時間、電撃への耐性を得る。装備による耐性に累積する。",
			"Gives resistance to electricity. This resistance can be added to which from equipment for more powerful resistance.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_elec(randint1(base) + base, FALSE);
			}
		}
		break;

	case 7:
		if (name) return _("耐酸", "Resist Acid");
		if (desc) return _("一定時間、酸への耐性を得る。装備による耐性に累積する。",
			"Gives resistance to acid. This resistance can be added to which from equipment for more powerful resistance.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_acid(randint1(base) + base, FALSE);
			}
		}
		break;

	case 8:
		if (name) return _("透明視認", "See Invisibility");
		if (desc) return _("一定時間、透明なものが見えるようになる。", "Gives see invisible for a while.");
    
		{
			int base = 24;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_invis(randint1(base) + base, FALSE);
			}
		}
		break;

	case 9:
		if (name) return _("解呪", "Remove Curse");
		if (desc) return _("アイテムにかかった弱い呪いを解除する。", "Removes normal curses from equipped items.");
    
		{
			if (cast)
			{
				if (remove_curse())
				{
					msg_print(_("誰かに見守られているような気がする。", "You feel as if someone is watching over you."));
				}
			}
		}
		break;

	case 10:
		if (name) return _("耐毒", "Resist Poison");
		if (desc) return _("一定時間、毒への耐性を得る。装備による耐性に累積する。",
			"Gives resistance to poison. This resistance can be added to which from equipment for more powerful resistance.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_pois(randint1(base) + base, FALSE);
			}
		}
		break;

	case 11:
		if (name) return _("狂戦士化", "Berserk");
		if (desc) return _("狂戦士化し、恐怖を除去する。", "Gives bonus to hit and HP, immunity to fear for a while. But decreases AC.");
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_shero(randint1(base) + base, FALSE);
				hp_player(30);
				set_afraid(0);
			}
		}
		break;

	case 12:
		if (name) return _("自己分析", "Self Knowledge");
		if (desc) return _("現在の自分の状態を完全に知る。",
			"Gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.");
    
		{
			if (cast)
			{
				self_knowledge();
			}
		}
		break;

	case 13:
		if (name) return _("対邪悪結界", "Protection from Evil");
		if (desc) return _("邪悪なモンスターの攻撃を防ぐバリアを張る。", "Gives aura which protect you from evil monster's physical attack.");
    
		{
			int base = 3 * plev;
			int sides = 25;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_protevil(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 14:
		if (name) return _("癒し", "Cure");
		if (desc) return _("毒、朦朧状態、負傷を全快させ、幻覚を直す。", "Heals poison, stun, cut and hallucination completely.");
    
		{
			if (cast)
			{
				set_poisoned(0);
				set_stun(0);
				set_cut(0);
				set_image(0);
			}
		}
		break;

	case 15:
		if (name) return _("魔法剣", "Mana Branding");
		if (desc) return _("一定時間、武器に冷気、炎、電撃、酸、毒のいずれかの属性をつける。武器を持たないと使えない。",
			"Makes current weapon some elemental branded. You must wield weapons.");
    
		{
			int base = plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				if (!choose_ele_attack()) return NULL;
			}
		}
		break;

	case 16:
		if (name) return _("テレパシー", "Telepathy");
		if (desc) return _("一定時間、テレパシー能力を得る。", "Gives telepathy for a while.");
    
		{
			int base = 25;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_tim_esp(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 17:
		if (name) return _("肌石化", "Stone Skin");
		if (desc) return _("一定時間、ACを上昇させる。", "Gives bonus to AC for a while.");
    
		{
			int base = 30;
			int sides = 20;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_shield(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 18:
		if (name) return _("全耐性", "Resistance");
		if (desc) return _("一定時間、酸、電撃、炎、冷気、毒に対する耐性を得る。装備による耐性に累積する。", 
			"Gives resistance to fire, cold, electricity, acid and poison for a while. These resistances can be added to which from equipment for more powerful resistances.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_acid(randint1(base) + base, FALSE);
				set_oppose_elec(randint1(base) + base, FALSE);
				set_oppose_fire(randint1(base) + base, FALSE);
				set_oppose_cold(randint1(base) + base, FALSE);
				set_oppose_pois(randint1(base) + base, FALSE);
			}
		}
		break;

	case 19:
		if (name) return _("スピード", "Haste Self");
		if (desc) return _("一定時間、加速する。", "Hastes you for a while.");
    
		{
			int base = plev;
			int sides = 20 + plev;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_fast(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 20:
		if (name) return _("壁抜け", "Walk through Wall");
		if (desc) return _("一定時間、半物質化し壁を通り抜けられるようになる。", "Gives ability to pass walls for a while.");
    
		{
			int base = plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_kabenuke(randint1(base) + base, FALSE);
			}
		}
		break;

	case 21:
		if (name) return _("盾磨き", "Polish Shield");
		if (desc) return _("盾に反射の属性をつける。", "Makes a shield a shield of reflection.");
    
		{
			if (cast)
			{
				pulish_shield();
			}
		}
		break;

	case 22:
		if (name) return _("ゴーレム製造", "Create Golem");
		if (desc) return _("1体のゴーレムを製造する。", "Creates a golem.");
    
		{
			if (cast)
			{
				if (summon_specific(-1, p_ptr->y, p_ptr->x, plev, SUMMON_GOLEM, PM_FORCE_PET))
				{
					msg_print(_("ゴーレムを作った。", "You make a golem."));
				}
				else
				{
					msg_print(_("うまくゴーレムを作れなかった。", "No Golems arrive."));
				}
			}
		}
		break;

	case 23:
		if (name) return _("魔法の鎧", "Magical armor");
		if (desc) return _("一定時間、魔法防御力とACが上がり、混乱と盲目の耐性、反射能力、麻痺知らず、浮遊を得る。",
			"Gives resistance to magic, bonus to AC, resistance to confusion, blindness, reflection, free action and levitation for a while.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_magicdef(randint1(base) + base, FALSE);
			}
		}
		break;

	case 24:
		if (name) return _("装備無力化", "Remove Enchantment");
		if (desc) return _("武器・防具にかけられたあらゆる魔力を完全に解除する。", "Removes all magics completely from any weapon or armor.");
    
		{
			if (cast)
			{
				if (!mundane_spell(TRUE)) return NULL;
			}
		}
		break;

	case 25:
		if (name) return _("呪い粉砕", "Remove All Curse");
		if (desc) return _("アイテムにかかった強力な呪いを解除する。", "Removes normal and heavy curse from equipped items.");
    
		{
			if (cast)
			{
				if (remove_all_curse())
				{
					msg_print(_("誰かに見守られているような気がする。", "You feel as if someone is watching over you."));
				}
			}
		}
		break;

	case 26:
		if (name) return _("完全なる知識", "Knowledge True");
		if (desc) return _("アイテムの持つ能力を完全に知る。", "*Identifies* an item.");
    
		{
			if (cast)
			{
				if (!identify_fully(FALSE)) return NULL;
			}
		}
		break;

	case 27:
		if (name) return _("武器強化", "Enchant Weapon");
		if (desc) return _("武器の命中率修正とダメージ修正を強化する。", "Attempts to increase +to-hit, +to-dam of a weapon.");
    
		{
			if (cast)
			{
				if (!enchant_spell(randint0(4) + 1, randint0(4) + 1, 0)) return NULL;
			}
		}
		break;

	case 28:
		if (name) return _("防具強化", "Enchant Armor");
		if (desc) return _("鎧の防御修正を強化する。", "Attempts to increase +AC of an armor.");
    
		{
			if (cast)
			{
				if (!enchant_spell(0, 0, randint0(3) + 2)) return NULL;
			}
		}
		break;

	case 29:
		if (name) return _("武器属性付与", "Brand Weapon");
		if (desc) return _("武器にランダムに属性をつける。", "Makes current weapon a random ego weapon.");
    
		{
			if (cast)
			{
				brand_weapon(randint0(18));
			}
		}
		break;

	case 30:
		if (name) return _("人間トランプ", "Living Trump");
		if (desc) return _("ランダムにテレポートする突然変異か、自分の意思でテレポートする突然変異が身につく。",
			"Gives mutation which makes you teleport randomly or makes you able to teleport at will.");
    
		{
			if (cast)
			{
				int mutation;

				if (one_in_(7))
					/* Teleport control */
					mutation = 12;
				else
					/* Random teleportation (uncontrolled) */
					mutation = 77;

				/* Gain the mutation */
				if (gain_random_mutation(mutation))
				{
					msg_print(_("あなたは生きているカードに変わった。", "You have turned into a Living Trump."));
				}
			}
		}
		break;

	case 31:
		if (name) return _("属性への免疫", "Immunity");
		if (desc) return _("一定時間、冷気、炎、電撃、酸のいずれかに対する免疫を得る。",
			"Gives an immunity to fire, cold, electricity or acid for a while.");
    
		{
			int base = 13;

			if (info) return info_duration(base, base);

			if (cast)
			{
				if (!choose_ele_immune(base + randint1(base))) return NULL;
			}
		}
		break;
	}

	return "";
}

/*!
 * @brief 悪魔領域魔法の各処理を行う
 * @param spell 魔法ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST時はNULL文字列を返す。
 */
static cptr do_daemon_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
	static const char s_dam[] = _("損傷:", "dam ");

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("マジック・ミサイル", "Magic Missile");
		if (desc) return _("弱い魔法の矢を放つ。", "Fires a weak bolt of magic.");
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir, damroll(dice, sides));
			}
		}
		break;

	case 1:
		if (name) return _("無生命感知", "Detect Unlife");
		if (desc) return _("近くの生命のないモンスターを感知する。", "Detects all nonliving monsters in your vicinity.");
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_nonliving(rad);
			}
		}
		break;

	case 2:
		if (name) return _("邪なる祝福", "Evil Bless");
		if (desc) return _("一定時間、命中率とACにボーナスを得る。", "Gives bonus to hit and AC for a few turns.");
    
		{
			int base = 12;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_blessed(randint1(base) + base, FALSE);
			}
		}
		break;

	case 3:
		if (name) return _("耐火炎", "Resist Fire");
		if (desc) return _("一定時間、炎への耐性を得る。装備による耐性に累積する。", 
			"Gives resistance to fire, cold and electricity for a while. These resistances can be added to which from equipment for more powerful resistances.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_fire(randint1(base) + base, FALSE);
			}
		}
		break;

	case 4:
		if (name) return _("恐慌", "Horrify");
		if (desc) return _("モンスター1体を恐怖させ、朦朧させる。抵抗されると無効。", "Attempts to scare and stun a monster.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fear_monster(dir, power);
				stun_monster(dir, power);
			}
		}
		break;

	case 5:
		if (name) return _("地獄の矢", "Nether Bolt");
		if (desc) return _("地獄のボルトもしくはビームを放つ。", "Fires a bolt or beam of nether.");
    
		{
			int dice = 6 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_NETHER, dir, damroll(dice, sides));
			}
		}
		break;

	case 6:
		if (name) return _("古代の死霊召喚", "Summon Manes");
		if (desc) return _("古代の死霊を召喚する。", "Summons a manes.");
    
		{
			if (cast)
			{
				if (!summon_specific(-1, p_ptr->y, p_ptr->x, (plev * 3) / 2, SUMMON_MANES, (PM_ALLOW_GROUP | PM_FORCE_PET)))
				{
					msg_print(_("古代の死霊は現れなかった。", "No Manes arrive."));
				}
			}
		}
		break;

	case 7:
		if (name) return _("地獄の焔", "Hellish Flame");
		if (desc) return _("邪悪な力を持つボールを放つ。善良なモンスターには大きなダメージを与える。",
			"Fires a ball of evil power. Hurts good monsters greatly.");
    
		{
			int dice = 3;
			int sides = 6;
			int rad = (plev < 30) ? 2 : 3;
			int base;

			if (p_ptr->pclass == CLASS_MAGE ||
			    p_ptr->pclass == CLASS_HIGH_MAGE ||
			    p_ptr->pclass == CLASS_SORCERER)
				base = plev + plev / 2;
			else
				base = plev + plev / 4;


			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_HELL_FIRE, dir, damroll(dice, sides) + base, rad);
			}
		}
		break;

	case 8:
		if (name) return _("デーモン支配", "Dominate Demon");
		if (desc) return _("悪魔1体を魅了する。抵抗されると無効", "Attempts to charm a demon.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				control_one_demon(dir, power);
			}
		}
		break;

	case 9:
		if (name) return _("ビジョン", "Vision");
		if (desc) return _("周辺の地形を感知する。", "Maps nearby area.");
    
		{
			int rad = DETECT_RAD_MAP;

			if (info) return info_radius(rad);

			if (cast)
			{
				map_area(rad);
			}
		}
		break;

	case 10:
		if (name) return _("耐地獄", "Resist Nether");
		if (desc) return _("一定時間、地獄への耐性を得る。", "Gives resistance to nether for a while.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_res_nether(randint1(base) + base, FALSE);
			}
		}
		break;

	case 11:
		if (name) return _("プラズマ・ボルト", "Plasma bolt");
		if (desc) return _("プラズマのボルトもしくはビームを放つ。", "Fires a bolt or beam of plasma.");
    
		{
			int dice = 11 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_PLASMA, dir, damroll(dice, sides));
			}
		}
		break;

	case 12:
		if (name) return _("ファイア・ボール", "Fire Ball");
		if (desc) return _("炎の球を放つ。", "Fires a ball of fire.");
    
		{
			HIT_POINT dam = plev + 55;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_FIRE, dir, dam, rad);
			}
		}
		break;

	case 13:
		if (name) return _("炎の刃", "Fire Branding");
		if (desc) return _("武器に炎の属性をつける。", "Makes current weapon fire branded.");
    
		{
			if (cast)
			{
				brand_weapon(1);
			}
		}
		break;

	case 14:
		if (name) return _("地獄球", "Nether Ball");
		if (desc) return _("大きな地獄の球を放つ。", "Fires a huge ball of nether.");
    
		{
			HIT_POINT dam = plev * 3 / 2 + 100;
			int rad = plev / 20 + 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_NETHER, dir, dam, rad);
			}
		}
		break;

	case 15:
		if (name) return _("デーモン召喚", "Summon Demon");
		if (desc) return _("悪魔1体を召喚する。", "Summons a demon.");
    
		{
			if (cast)
			{
				bool pet = !one_in_(3);
				u32b flg = 0L;

				if (pet) flg |= PM_FORCE_PET;
				else flg |= PM_NO_PET;
				if (!(pet && (plev < 50))) flg |= PM_ALLOW_GROUP;

				if (summon_specific((pet ? -1 : 0), p_ptr->y, p_ptr->x, plev*2/3+randint1(plev/2), SUMMON_DEMON, flg))
				{
					msg_print(_("硫黄の悪臭が充満した。", "The area fills with a stench of sulphur and brimstone."));

					if (pet)
					{
						msg_print(_("「ご用でございますか、ご主人様」", "'What is thy bidding... Master?'"));
					}
					else
					{
						msg_print(_("「卑しき者よ、我は汝の下僕にあらず！ お前の魂を頂くぞ！」",
									"'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'"));
					}
				}
				else
				{
					msg_print(_("悪魔は現れなかった。", "No demons arrive."));
				}
				break;
			}
		}
		break;

	case 16:
		if (name) return _("悪魔の目", "Devilish Eye");
		if (desc) return _("一定時間、テレパシー能力を得る。", "Gives telepathy for a while.");
    
		{
			int base = 30;
			int sides = 25;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_tim_esp(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 17:
		if (name) return _("悪魔のクローク", "Devil Cloak");
		if (desc) return _("恐怖を取り除き、一定時間、炎と冷気の耐性、炎のオーラを得る。耐性は装備による耐性に累積する。", 
			"Removes fear. Gives resistance to fire and cold, and aura of fire. These resistances can be added to which from equipment for more powerful resistances.");
    
		{
			TIME_EFFECT base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				TIME_EFFECT dur = randint1(base) + base;
					
				set_oppose_fire(dur, FALSE);
				set_oppose_cold(dur, FALSE);
				set_tim_sh_fire(dur, FALSE);
				set_afraid(0);
				break;
			}
		}
		break;

	case 18:
		if (name) return _("溶岩流", "The Flow of Lava");
		if (desc) return _("自分を中心とした炎の球を作り出し、床を溶岩に変える。", 
			"Generates a ball of fire centered on you which transforms floors to magma.");
    
		{
			HIT_POINT dam = (55 + plev) * 2;
			int rad = 3;

			if (info) return info_damage(0, 0, dam/2);

			if (cast)
			{
				fire_ball(GF_FIRE, 0, dam, rad);
				fire_ball_hide(GF_LAVA_FLOW, 0, 2 + randint1(2), rad);
			}
		}
		break;

	case 19:
		if (name) return _("プラズマ球", "Plasma Ball");
		if (desc) return _("プラズマの球を放つ。", "Fires a ball of plasma.");
    
		{
			HIT_POINT dam = plev * 3 / 2 + 80;
			int rad = 2 + plev / 40;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_PLASMA, dir, dam, rad);
			}
		}
		break;

	case 20:
		if (name) return _("悪魔変化", "Polymorph Demon");
		if (desc) return _("一定時間、悪魔に変化する。変化している間は本来の種族の能力を失い、代わりに悪魔としての能力を得る。", 
			"Mimic a demon for a while. Loses abilities of original race and gets abilities as a demon.");
    
		{
			int base = 10 + plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_mimic(base + randint1(base), MIMIC_DEMON, FALSE);
			}
		}
		break;

	case 21:
		if (name) return _("地獄の波動", "Nather Wave");
		if (desc) return _("視界内の全てのモンスターにダメージを与える。善良なモンスターに特に大きなダメージを与える。", 
			"Damages all monsters in sight. Hurts good monsters greatly.");
    
		{
			int sides1 = plev * 2;
			int sides2 = plev * 2;

			if (info) return format("%sd%d+d%d", s_dam, sides1, sides2);

			if (cast)
			{
				dispel_monsters(randint1(sides1));
				dispel_good(randint1(sides2));
			}
		}
		break;

	case 22:
		if (name) return _("サキュバスの接吻", "Kiss of Succubus");
		if (desc) return _("因果混乱の球を放つ。", "Fires a ball of nexus.");
    
		{
			HIT_POINT dam = 100 + plev * 2;
			int rad = 4;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball(GF_NEXUS, dir, dam, rad);
			}
		}
		break;

	case 23:
		if (name) return _("破滅の手", "Doom Hand");
		if (desc) return _("破滅の手を放つ。食らったモンスターはそのときのHPの半分前後のダメージを受ける。", "Attempts to make a monster's HP almost half.");
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir))
					return NULL;
				else 
					msg_print(_("<破滅の手>を放った！", "You invoke the Hand of Doom!"));
				
				fire_ball_hide(GF_HAND_DOOM, dir, plev * 2, 0);
			}
		}
		break;

	case 24:
		if (name) return _("士気高揚", "Raise the Morale");
		if (desc) return _("一定時間、ヒーロー気分になる。", "Removes fear, and gives bonus to hit and 10 more HP for a while.");
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_hero(randint1(base) + base, FALSE);
				hp_player(10);
				set_afraid(0);
			}
		}
		break;

	case 25:
		if (name) return _("不滅の肉体", "Immortal Body");
		if (desc) return _("一定時間、時間逆転への耐性を得る。", "Gives resistance to time for a while.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_res_time(randint1(base)+base, FALSE);
			}
		}
		break;

	case 26:
		if (name) return _("狂気の円環", "Insanity Circle");
		if (desc) return _("自分を中心としたカオスの球、混乱の球を発生させ、近くのモンスターを魅了する。", 
			"Generate balls of chaos, confusion and charm centered on you.");
    
		{
			HIT_POINT dam = 50 + plev;
			int power = 20 + plev;
			int rad = 3 + plev / 20;

			if (info) return format("%s%d+%d", s_dam, dam/2, dam/2);

			if (cast)
			{
				fire_ball(GF_CHAOS, 0, dam, rad);
				fire_ball(GF_CONFUSION, 0, dam, rad);
				fire_ball(GF_CHARM, 0, power, rad);
			}
		}
		break;

	case 27:
		if (name) return _("ペット爆破", "Explode Pets");
		if (desc) return _("全てのペットを強制的に爆破させる。", "Makes all pets explode.");
    
		{
			if (cast)
			{
				discharge_minion();
			}
		}
		break;

	case 28:
		if (name) return _("グレーターデーモン召喚", "Summon Greater Demon");
		if (desc) return _("上級デーモンを召喚する。召喚するには人間('p','h','t'で表されるモンスター)の死体を捧げなければならない。", 
			"Summons greater demon. It need to sacrifice a corpse of human ('p','h' or 't').");
    
		{
			if (cast)
			{
				if (!cast_summon_greater_demon()) return NULL;
			}
		}
		break;

	case 29:
		if (name) return _("地獄嵐", "Nether Storm");
		if (desc) return _("超巨大な地獄の球を放つ。", "Generate a huge ball of nether.");
    
		{
			HIT_POINT dam = plev * 15;
			int rad = plev / 5;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_NETHER, dir, dam, rad);
			}
		}
		break;

	case 30:
		if (name) return _("血の呪い", "Bloody Curse");
		if (desc) return _("自分がダメージを受けることによって対象に呪いをかけ、ダメージを与え様々な効果を引き起こす。",
			"Puts blood curse which damages and causes various effects on a monster. You also take damage.");
    
		{
			HIT_POINT dam = 600;
			int rad = 0;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball_hide(GF_BLOOD_CURSE, dir, dam, rad);
				take_hit(DAMAGE_USELIFE, 20 + randint1(30), _("血の呪い", "Blood curse"), -1);
			}
		}
		break;

	case 31:
		if (name) return _("魔王変化", "Polymorph Demonlord");
		if (desc) return _("悪魔の王に変化する。変化している間は本来の種族の能力を失い、代わりに悪魔の王としての能力を得、壁を破壊しながら歩く。",
			"Mimic a demon lord for a while. Loses abilities of original race and gets great abilities as a demon lord. Even hard walls can't stop your walking.");
    
		{
			int base = 15;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_mimic(base + randint1(base), MIMIC_DEMON_LORD, FALSE);
			}
		}
		break;
	}

	return "";
}

/*!
 * @brief 破邪領域魔法の各処理を行う
 * @param spell 魔法ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST時はNULL文字列を返す。
 */
static cptr do_crusade_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("懲罰", "Punishment");
		if (desc) return _("電撃のボルトもしくはビームを放つ。", "Fires a bolt or beam of lightning.");
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir, damroll(dice, sides));
			}
		}
		break;

	case 1:
		if (name) return _("邪悪存在感知", "Detect Evil");
		if (desc) return _("近くの邪悪なモンスターを感知する。", "Detects all evil monsters in your vicinity.");
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_evil(rad);
			}
		}
		break;

	case 2:
		if (name) return _("恐怖除去", "Remove Fear");
		if (desc) return _("恐怖を取り除く。", "Removes fear.");
    
		{
			if (cast)
			{
				set_afraid(0);
			}
		}
		break;

	case 3:
		if (name) return _("威圧", "Scare Monster");
		if (desc) return _("モンスター1体を恐怖させる。抵抗されると無効。", "Attempts to scare a monster.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fear_monster(dir, power);
			}
		}
		break;

	case 4:
		if (name) return _("聖域", "Sanctuary");
		if (desc) return _("隣接した全てのモンスターを眠らせる。抵抗されると無効。", "Attempts to sleep monsters in the adjacent squares.");
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				sleep_monsters_touch();
			}
		}
		break;

	case 5:
		if (name) return _("入口", "Portal");
		if (desc) return _("中距離のテレポートをする。", "Teleport medium distance.");
    
		{
			POSITION range = 25 + plev / 2;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 6:
		if (name) return _("スターダスト", "Star Dust");
		if (desc) return _("ターゲット付近に閃光のボルトを連射する。", "Fires many bolts of light near the target.");
    
		{
			int dice = 3 + (plev - 1) / 9;
			int sides = 2;

			if (info) return info_multi_damage_dice(dice, sides);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_blast(GF_LITE, dir, dice, sides, 10, 3);
			}
		}
		break;

	case 7:
		if (name) return _("身体浄化", "Purify");
		if (desc) return _("傷、毒、朦朧から全快する。", "Heals all cut, stun and poison status.");
    
		{
			if (cast)
			{
				set_cut(0);
				set_poisoned(0);
				set_stun(0);
			}
		}
		break;

	case 8:
		if (name) return _("邪悪飛ばし", "Scatter Evil");
		if (desc) return _("邪悪なモンスター1体をテレポートさせる。抵抗されると無効。", "Attempts to teleport an evil monster away.");
    
		{
			int power = MAX_SIGHT * 5;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball(GF_AWAY_EVIL, dir, power, 0);
			}
		}
		break;

	case 9:
		if (name) return _("聖なる光球", "Holy Orb");
		if (desc) return _("聖なる力をもつ宝珠を放つ。邪悪なモンスターに対して大きなダメージを与えるが、善良なモンスターには効果がない。", 
			"Fires a ball with holy power. Hurts evil monsters greatly, but don't effect good monsters.");
    
		{
			int dice = 3;
			int sides = 6;
			int rad = (plev < 30) ? 2 : 3;
			int base;

			if (p_ptr->pclass == CLASS_PRIEST ||
			    p_ptr->pclass == CLASS_HIGH_MAGE ||
			    p_ptr->pclass == CLASS_SORCERER)
				base = plev + plev / 2;
			else
				base = plev + plev / 4;


			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_HOLY_FIRE, dir, damroll(dice, sides) + base, rad);
			}
		}
		break;

	case 10:
		if (name) return _("悪魔払い", "Exorcism");
		if (desc) return _("視界内の全てのアンデッド及び悪魔にダメージを与え、邪悪なモンスターを恐怖させる。", 
			"Damages all undead and demons in sight, and scares all evil monsters in sight.");
    
		{
			int sides = plev;
			int power = plev;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_undead(randint1(sides));
				dispel_demons(randint1(sides));
				turn_evil(power);
			}
		}
		break;

	case 11:
		if (name) return _("解呪", "Remove Curse");
		if (desc) return _("アイテムにかかった弱い呪いを解除する。", "Removes normal curses from equipped items.");
    
		{
			if (cast)
			{
				if (remove_curse())
				{
					msg_print(_("誰かに見守られているような気がする。", "You feel as if someone is watching over you."));
				}
			}
		}
		break;

	case 12:
		if (name) return _("透明視認", "Sense Unseen");
		if (desc) return _("一定時間、透明なものが見えるようになる。", "Gives see invisible for a while.");
    
		{
			int base = 24;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_invis(randint1(base) + base, FALSE);
			}
		}
		break;

	case 13:
		if (name) return _("対邪悪結界", "Protection from Evil");
		if (desc) return _("邪悪なモンスターの攻撃を防ぐバリアを張る。", "Gives aura which protect you from evil monster's physical attack.");
    
		{
			int base = 25;
			int sides = 3 * plev;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_protevil(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 14:
		if (name) return _("裁きの雷", "Judgment Thunder");
		if (desc) return _("強力な電撃のボルトを放つ。", "Fires a powerful bolt of lightning.");
    
		{
			HIT_POINT dam = plev * 5;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_bolt(GF_ELEC, dir, dam);
			}
		}
		break;

	case 15:
		if (name) return _("聖なる御言葉", "Holy Word");
		if (desc) return _("視界内の邪悪な存在に大きなダメージを与え、体力を回復し、毒、恐怖、朦朧状態、負傷から全快する。",
			"Damages all evil monsters in sight, heals HP somewhat, and completely heals poison, fear, stun and cut status.");
    
		{
			int dam_sides = plev * 6;
			int heal = 100;

			if (info) return format(_("損:1d%d/回%d", "dam:d%d/h%d"), dam_sides, heal);
			if (cast)
			{
				dispel_evil(randint1(dam_sides));
				hp_player(heal);
				set_afraid(0);
				set_poisoned(0);
				set_stun(0);
				set_cut(0);
			}
		}
		break;

	case 16:
		if (name) return _("開かれた道", "Unbarring Ways");
		if (desc) return _("一直線上の全ての罠と扉を破壊する。", "Fires a beam which destroy traps and doors.");
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				destroy_door(dir);
			}
		}
		break;

	case 17:
		if (name) return _("封魔", "Arrest");
		if (desc) return _("邪悪なモンスターの動きを止める。", "Attempts to paralyze an evil monster.");
    
		{
			int power = plev * 2;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				stasis_evil(dir);
			}
		}
		break;

	case 18:
		if (name) return _("聖なるオーラ", "Holy Aura");
		if (desc) return _("一定時間、邪悪なモンスターを傷つける聖なるオーラを得る。",
			"Gives aura of holy power which injures evil monsters which attacked you for a while.");
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_sh_holy(randint1(base) + base, FALSE);
			}
		}
		break;

	case 19:
		if (name) return _("アンデッド&悪魔退散", "Dispel Undead & Demons");
		if (desc) return _("視界内の全てのアンデッド及び悪魔にダメージを与える。", "Damages all undead and demons in sight.");
    
		{
			int sides = plev * 4;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_undead(randint1(sides));
				dispel_demons(randint1(sides));
			}
		}
		break;

	case 20:
		if (name) return _("邪悪退散", "Dispel Evil");
		if (desc) return _("視界内の全ての邪悪なモンスターにダメージを与える。", "Damages all evil monsters in sight.");
    
		{
			int sides = plev * 4;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_evil(randint1(sides));
			}
		}
		break;

	case 21:
		if (name) return _("聖なる刃", "Holy Blade");
		if (desc) return _("通常の武器に滅邪の属性をつける。", "Makes current weapon especially deadly against evil monsters.");
    
		{
			if (cast)
			{
				brand_weapon(13);
			}
		}
		break;

	case 22:
		if (name) return _("スターバースト", "Star Burst");
		if (desc) return _("巨大な閃光の球を放つ。", "Fires a huge ball of powerful light.");
    
		{
			HIT_POINT dam = 100 + plev * 2;
			int rad = 4;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_LITE, dir, dam, rad);
			}
		}
		break;

	case 23:
		if (name) return _("天使召喚", "Summon Angel");
		if (desc) return _("天使を1体召喚する。", "Summons an angel.");
    
		{
			if (cast)
			{
				bool pet = !one_in_(3);
				u32b flg = 0L;

				if (pet) flg |= PM_FORCE_PET;
				else flg |= PM_NO_PET;
				if (!(pet && (plev < 50))) flg |= PM_ALLOW_GROUP;

				if (summon_specific((pet ? -1 : 0), p_ptr->y, p_ptr->x, (plev * 3) / 2, SUMMON_ANGEL, flg))
				{
					if (pet)
					{
						msg_print(_("「ご用でございますか、ご主人様」", "'What is thy bidding... Master?'"));
					}
					else
					{
						msg_print(_("「我は汝の下僕にあらず！ 悪行者よ、悔い改めよ！」", "Mortal! Repent of thy impiousness."));
					}
				}
			}
		}
		break;

	case 24:
		if (name) return _("士気高揚", "Heroism");
		if (desc) return _("一定時間、ヒーロー気分になる。", "Removes fear, and gives bonus to hit and 10 more HP for a while.");
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_hero(randint1(base) + base, FALSE);
				hp_player(10);
				set_afraid(0);
			}
		}
		break;

	case 25:
		if (name) return _("呪い退散", "Dispel Curse");
		if (desc) return _("アイテムにかかった強力な呪いを解除する。", "Removes normal and heavy curse from equipped items.");
    
		{
			if (cast)
			{
				if (remove_all_curse())
				{
					msg_print(_("誰かに見守られているような気がする。", "You feel as if someone is watching over you."));
				}
			}
		}
		break;

	case 26:
		if (name) return _("邪悪追放", "Banish Evil");
		if (desc) return _("視界内の全ての邪悪なモンスターをテレポートさせる。抵抗されると無効。", 
			"Teleports all evil monsters in sight away unless resisted.");
    
		{
			int power = 100;

			if (info) return info_power(power);

			if (cast)
			{
				if (banish_evil(power))
				{
					msg_print(_("神聖な力が邪悪を打ち払った！", "The holy power banishes evil!"));
				}
			}
		}
		break;

	case 27:
		if (name) return _("ハルマゲドン", "Armageddon");
		if (desc) return _("周辺のアイテム、モンスター、地形を破壊する。", "Destroy everything in nearby area.");
    
		{
			int base = 12;
			int sides = 4;

			if (cast)
			{
				destroy_area(p_ptr->y, p_ptr->x, base + randint1(sides), FALSE);
			}
		}
		break;

	case 28:
		if (name) return _("目には目を", "An Eye for an Eye");
		if (desc) return _("一定時間、自分がダメージを受けたときに攻撃を行ったモンスターに対して同等のダメージを与える。", 
			"Gives special aura for a while. When you are attacked by a monster, the monster are injured with same amount of damage as you take.");
    
		{
			int base = 10;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_eyeeye(randint1(base) + base, FALSE);
			}
		}
		break;

	case 29:
		if (name) return _("神の怒り", "Wrath of the God");
		if (desc) return _("ターゲットの周囲に分解の球を多数落とす。", "Drops many balls of disintegration near the target.");
    
		{
			HIT_POINT dam = plev * 3 + 25;
			int rad = 2;

			if (info) return info_multi_damage(dam);

			if (cast)
			{
				if (!cast_wrath_of_the_god(dam, rad)) return NULL;
			}
		}
		break;

	case 30:
		if (name) return _("神威", "Divine Intervention");
		if (desc) return _("隣接するモンスターに聖なるダメージを与え、視界内のモンスターにダメージ、減速、朦朧、混乱、恐怖、眠りを与える。さらに体力を回復する。", 
			"Damages all adjacent monsters with holy power. Damages and attempt to slow, stun, confuse, scare and freeze all monsters in sight. And heals HP.");
    
		{
			int b_dam = plev * 11;
			int d_dam = plev * 4;
			int heal = 100;
			int power = plev * 4;

			if (info) return format(_("回%d/損%d+%d", "h%d/dm%d+%d"), heal, d_dam, b_dam/2);
			if (cast)
			{
				project(0, 1, p_ptr->y, p_ptr->x, b_dam, GF_HOLY_FIRE, PROJECT_KILL, -1);
				dispel_monsters(d_dam);
				slow_monsters(plev);
				stun_monsters(power);
				confuse_monsters(power);
				turn_monsters(power);
				stasis_monsters(power);
				hp_player(heal);
			}
		}
		break;

	case 31:
		if (name) return _("聖戦", "Crusade");
		if (desc) return _("視界内の善良なモンスターをペットにしようとし、ならなかった場合及び善良でないモンスターを恐怖させる。さらに多数の加速された騎士を召喚し、ヒーロー、祝福、加速、対邪悪結界を得る。", 
			"Attempts to charm all good monsters in sight, and scare all non-charmed monsters, and summons great number of knights, and gives heroism, bless, speed and protection from evil.");
    
		{
			if (cast)
			{
				int base = 25;
				int sp_sides = 20 + plev;
				int sp_base = plev;

				int i;
				crusade();
				for (i = 0; i < 12; i++)
				{
					int attempt = 10;
					POSITION my = 0, mx = 0;

					while (attempt--)
					{
						scatter(&my, &mx, p_ptr->y, p_ptr->x, 4, 0);

						/* Require empty grids */
						if (cave_empty_bold2(my, mx)) break;
					}
					if (attempt < 0) continue;
					summon_specific(-1, my, mx, plev, SUMMON_KNIGHTS, (PM_ALLOW_GROUP | PM_FORCE_PET | PM_HASTE));
				}
				set_hero(randint1(base) + base, FALSE);
				set_blessed(randint1(base) + base, FALSE);
				set_fast(randint1(sp_sides) + sp_base, FALSE);
				set_protevil(randint1(base) + base, FALSE);
				set_afraid(0);
			}
		}
		break;
	}

	return "";
}


/*!
 * @brief 歌の各処理を行う
 * @param spell 歌ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST / SPELL_FAIL / SPELL_CONT / SPELL_STOP)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST / SPELL_FAIL / SPELL_CONT / SPELL_STOP 時はNULL文字列を返す。
 */
static cptr do_music_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
	bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;
	bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
	bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;
	static const char s_dam[] = _("損傷:", "dam ");

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("遅鈍の歌", "Song of Holding");
		if (desc) return _("視界内の全てのモンスターを減速させる。抵抗されると無効。", "Attempts to slow all monsters in sight.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("ゆっくりとしたメロディを口ずさみ始めた．．．", "You start humming a slow, steady melody..."));
			start_singing(spell, MUSIC_SLOW);
		}

		{
			int power = plev;

			if (info) return info_power(power);

			if (cont)
			{
				slow_monsters(plev);
			}
		}
		break;

	case 1:
		if (name) return _("祝福の歌", "Song of Blessing");
		if (desc) return _("命中率とACのボーナスを得る。", "Gives bonus to hit and AC for a few turns.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("厳かなメロディを奏で始めた．．．", "The holy power of the Music of the Ainur enters you..."));
			start_singing(spell, MUSIC_BLESS);
		}

		if (stop)
		{
			if (!p_ptr->blessed)
			{
				msg_print(_("高潔な気分が消え失せた。", "The prayer has expired."));
			}
		}

		break;

	case 2:
		if (name) return _("崩壊の音色", "Wrecking Note");
		if (desc) return _("轟音のボルトを放つ。", "Fires a bolt of sound.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		{
			int dice = 4 + (plev - 1) / 5;
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt(GF_SOUND, dir, damroll(dice, sides));
			}
		}
		break;

	case 3:
		if (name) return _("朦朧の旋律", "Stun Pattern");
		if (desc) return _("視界内の全てのモンスターを朦朧させる。抵抗されると無効。", "Attempts to stun all monsters in sight.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("眩惑させるメロディを奏で始めた．．．", "You weave a pattern of sounds to bewilder and daze..."));
			start_singing(spell, MUSIC_STUN);
		}

		{
			int dice = plev / 10;
			int sides = 2;

			if (info) return info_power_dice(dice, sides);

			if (cont)
			{
				stun_monsters(damroll(dice, sides));
			}
		}

		break;

	case 4:
		if (name) return _("生命の流れ", "Flow of Life");
		if (desc) return _("体力を少し回復させる。", "Heals HP a little.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("歌を通して体に活気が戻ってきた．．．", "Life flows through you as you sing a song of healing..."));
			start_singing(spell, MUSIC_L_LIFE);
		}

		{
			int dice = 2;
			int sides = 6;

			if (info) return info_heal(dice, sides, 0);

			if (cont)
			{
				hp_player(damroll(dice, sides));
			}
		}

		break;

	case 5:
		if (name) return _("太陽の歌", "Song of the Sun");
		if (desc) return _("光源が照らしている範囲か部屋全体を永久に明るくする。", "Lights up nearby area and the inside of a room permanently.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		{
			int dice = 2;
			int sides = plev / 2;
			int rad = plev / 10 + 1;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				msg_print(_("光り輝く歌が辺りを照らした。", "Your uplifting song brings brightness to dark places..."));
				lite_area(damroll(dice, sides), rad);
			}
		}
		break;

	case 6:
		if (name) return _("恐怖の歌", "Song of Fear");
		if (desc) return _("視界内の全てのモンスターを恐怖させる。抵抗されると無効。", "Attempts to scare all monsters in sight.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("おどろおどろしいメロディを奏で始めた．．．", "You start weaving a fearful pattern..."));
			start_singing(spell, MUSIC_FEAR);			
		}

		{
			int power = plev;

			if (info) return info_power(power);

			if (cont)
			{
				project_hack(GF_TURN_ALL, power);
			}
		}

		break;

	case 7:
		if (name) return _("戦いの歌", "Heroic Ballad");
		if (desc) return _("ヒーロー気分になる。", "Removes fear, and gives bonus to hit and 10 more HP for a while.");

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("激しい戦いの歌を歌った．．．", "You start singing a song of intense fighting..."));

			(void)hp_player(10);
			(void)set_afraid(0);

			/* Recalculate hitpoints */
			p_ptr->update |= (PU_HP);

			start_singing(spell, MUSIC_HERO);
		}

		if (stop)
		{
			if (!p_ptr->hero)
			{
				msg_print(_("ヒーローの気分が消え失せた。", "The heroism wears off."));
				/* Recalculate hitpoints */
				p_ptr->update |= (PU_HP);
			}
		}

		break;

	case 8:
		if (name) return _("霊的知覚", "Clairaudience");
		if (desc) return _("近くの罠/扉/階段を感知する。レベル15で全てのモンスター、20で財宝とアイテムを感知できるようになる。レベル25で周辺の地形を感知し、40でその階全体を永久に照らし、ダンジョン内のすべてのアイテムを感知する。この効果は歌い続けることで順に起こる。", 
			"Detects traps, doors and stairs in your vicinity. And detects all monsters at level 15, treasures and items at level 20. Maps nearby area at level 25. Lights and know the whole level at level 40. These effects occurs by turns while this song continues.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("静かな音楽が感覚を研ぎ澄まさせた．．．", "Your quiet music sharpens your sense of hearing..."));
			/* Hack -- Initialize the turn count */
			SINGING_COUNT(p_ptr) = 0;
			start_singing(spell, MUSIC_DETECT);
		}

		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cont)
			{
				int count = SINGING_COUNT(p_ptr);

				if (count >= 19) wiz_lite(FALSE);
				if (count >= 11)
				{
					map_area(rad);
					if (plev > 39 && count < 19)
						SINGING_COUNT(p_ptr) = count + 1;
				}
				if (count >= 6)
				{
					/* There are too many hidden treasure.  So... */
					/* detect_treasure(rad); */
					detect_objects_gold(rad);
					detect_objects_normal(rad);

					if (plev > 24 && count < 11)
						SINGING_COUNT(p_ptr) = count + 1;
				}
				if (count >= 3)
				{
					detect_monsters_invis(rad);
					detect_monsters_normal(rad);

					if (plev > 19 && count < 6)
						SINGING_COUNT(p_ptr) = count + 1;
				}
				detect_traps(rad, TRUE);
				detect_doors(rad);
				detect_stairs(rad);

				if (plev > 14 && count < 3)
					SINGING_COUNT(p_ptr) = count + 1;
			}
		}

		break;

	case 9:
		if (name) return _("魂の歌", "Soul Shriek");
		if (desc) return _("視界内の全てのモンスターに対して精神攻撃を行う。", "Damages all monsters in sight with PSI damages.");

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("精神を捻じ曲げる歌を歌った．．．", "You start singing a song of soul in pain..."));
			start_singing(spell, MUSIC_PSI);
		}

		{
			int dice = 1;
			int sides = plev * 3 / 2;

			if (info) return info_damage(dice, sides, 0);

			if (cont)
			{
				project_hack(GF_PSI, damroll(dice, sides));
			}
		}

		break;

	case 10:
		if (name) return _("知識の歌", "Song of Lore");
		if (desc) return _("自分のいるマスと隣りのマスに落ちているアイテムを鑑定する。", "Identifies all items which are in the adjacent squares.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("この世界の知識が流れ込んできた．．．", "You recall the rich lore of the world..."));
			start_singing(spell, MUSIC_ID);
		}

		{
			int rad = 1;

			if (info) return info_radius(rad);

			/*
			 * 歌の開始時にも効果発動：
			 * MP不足で鑑定が発動される前に歌が中断してしまうのを防止。
			 */
			if (cont || cast)
			{
				project(0, rad, p_ptr->y, p_ptr->x, 0, GF_IDENTIFY, PROJECT_ITEM, -1);
			}
		}

		break;

	case 11:
		if (name) return _("隠遁の歌", "Hiding Tune");
		if (desc) return _("隠密行動能力を上昇させる。", "Gives improved stealth.");

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("あなたの姿が景色にとけこんでいった．．．", "Your song carries you beyond the sight of mortal eyes..."));
			start_singing(spell, MUSIC_STEALTH);
		}

		if (stop)
		{
			if (!p_ptr->tim_stealth)
			{
				msg_print(_("姿がはっきりと見えるようになった。", "You are no longer hided."));
			}
		}

		break;

	case 12:
		if (name) return _("幻影の旋律", "Illusion Pattern");
		if (desc) return _("視界内の全てのモンスターを混乱させる。抵抗されると無効。", "Attempts to confuse all monsters in sight.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("辺り一面に幻影が現れた．．．", "You weave a pattern of sounds to beguile and confuse..."));
			start_singing(spell, MUSIC_CONF);
		}

		{
			int power = plev * 2;

			if (info) return info_power(power);

			if (cont)
			{
				confuse_monsters(power);
			}
		}

		break;

	case 13:
		if (name) return _("破滅の叫び", "Doomcall");
		if (desc) return _("視界内の全てのモンスターに対して轟音攻撃を行う。", "Damages all monsters in sight with booming sound.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("轟音が響いた．．．", "The fury of the Downfall of Numenor lashes out..."));
			start_singing(spell, MUSIC_SOUND);
		}

		{
			int dice = 10 + plev / 5;
			int sides = 7;

			if (info) return info_damage(dice, sides, 0);

			if (cont)
			{
				project_hack(GF_SOUND, damroll(dice, sides));
			}
		}

		break;

	case 14:
		if (name) return _("フィリエルの歌", "Firiel's Song");
		if (desc) return _("周囲の死体や骨を生き返す。", "Resurrects nearby corpse and skeletons. And makes these your pets.");
    
		{
			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				msg_print(_("生命と復活のテーマを奏で始めた．．．", "The themes of life and revival are woven into your song..."));
				animate_dead(0, p_ptr->y, p_ptr->x);
			}
		}
		break;

	case 15:
		if (name) return _("旅の仲間", "Fellowship Chant");
		if (desc) return _("視界内の全てのモンスターを魅了する。抵抗されると無効。", "Attempts to charm all monsters in sight.");

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("安らかなメロディを奏で始めた．．．", "You weave a slow, soothing melody of imploration..."));
			start_singing(spell, MUSIC_CHARM);
		}

		{
			int dice = 10 + plev / 15;
			int sides = 6;

			if (info) return info_power_dice(dice, sides);

			if (cont)
			{
				charm_monsters(damroll(dice, sides));
			}
		}

		break;

	case 16:
		if (name) return _("分解音波", "Sound of disintegration");
		if (desc) return _("壁を掘り進む。自分の足元のアイテムは蒸発する。", "Makes you be able to burrow into walls. Objects under your feet evaporate.");

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("粉砕するメロディを奏で始めた．．．", "You weave a violent pattern of sounds to break wall."));
			start_singing(spell, MUSIC_WALL);
		}

		{
			/*
			 * 歌の開始時にも効果発動：
			 * MP不足で効果が発動される前に歌が中断してしまうのを防止。
			 */
			if (cont || cast)
			{
				project(0, 0, p_ptr->y, p_ptr->x,
					0, GF_DISINTEGRATE, PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE, -1);
			}
		}
		break;

	case 17:
		if (name) return _("元素耐性", "Finrod's Resistance");
		if (desc) return _("酸、電撃、炎、冷気、毒に対する耐性を得る。装備による耐性に累積する。", 
			"Gives resistance to fire, cold, electricity, acid and poison. These resistances can be added to which from equipment for more powerful resistances.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("元素の力に対する忍耐の歌を歌った。", "You sing a song of perseverance against powers..."));
			start_singing(spell, MUSIC_RESIST);
		}

		if (stop)
		{
			if (!p_ptr->oppose_acid)
			{
				msg_print(_("酸への耐性が薄れた気がする。", "You feel less resistant to acid."));
			}

			if (!p_ptr->oppose_elec)
			{
				msg_print(_("電撃への耐性が薄れた気がする。", "You feel less resistant to elec."));
			}

			if (!p_ptr->oppose_fire)
			{
				msg_print(_("火への耐性が薄れた気がする。", "You feel less resistant to fire."));
			}

			if (!p_ptr->oppose_cold)
			{
				msg_print(_("冷気への耐性が薄れた気がする。", "You feel less resistant to cold."));
			}

			if (!p_ptr->oppose_pois)
			{
				msg_print(_("毒への耐性が薄れた気がする。", "You feel less resistant to pois."));
			}
		}

		break;

	case 18:
		if (name) return _("ホビットのメロディ", "Hobbit Melodies");
		if (desc) return _("加速する。", "Hastes you.");

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("軽快な歌を口ずさみ始めた．．．", "You start singing joyful pop song..."));
			start_singing(spell, MUSIC_SPEED);
		}

		if (stop)
		{
			if (!p_ptr->fast)
			{
				msg_print(_("動きの素早さがなくなったようだ。", "You feel yourself slow down."));
			}
		}

		break;

	case 19:
		if (name) return _("歪んだ世界", "World Contortion");
		if (desc) return _("近くのモンスターをテレポートさせる。抵抗されると無効。", "Teleports all nearby monsters away unless resisted.");
    
		{
			int rad = plev / 15 + 1;
			int power = plev * 3 + 1;

			if (info) return info_radius(rad);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				msg_print(_("歌が空間を歪めた．．．", "Reality whirls wildly as you sing a dizzying melody..."));
				project(0, rad, p_ptr->y, p_ptr->x, power, GF_AWAY_ALL, PROJECT_KILL, -1);
			}
		}
		break;

	case 20:
		if (name) return _("退散の歌", "Dispelling chant");
		if (desc) return _("視界内の全てのモンスターにダメージを与える。邪悪なモンスターに特に大きなダメージを与える。", 
			"Damages all monsters in sight. Hurts evil monsters greatly.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("耐えられない不協和音が敵を責め立てた．．．", "You cry out in an ear-wracking voice..."));
			start_singing(spell, MUSIC_DISPEL);
		}

		{
			int m_sides = plev * 3;
			int e_sides = plev * 3;

			if (info) return format("%s1d%d+1d%d", s_dam, m_sides, e_sides);

			if (cont)
			{
				dispel_monsters(randint1(m_sides));
				dispel_evil(randint1(e_sides));
			}
		}
		break;

	case 21:
		if (name) return _("サルマンの甘言", "The Voice of Saruman");
		if (desc) return _("視界内の全てのモンスターを減速させ、眠らせようとする。抵抗されると無効。", "Attempts to slow and sleep all monsters in sight.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("優しく、魅力的な歌を口ずさみ始めた．．．", "You start humming a gentle and attractive song..."));
			start_singing(spell, MUSIC_SARUMAN);
		}

		{
			int power = plev;

			if (info) return info_power(power);

			if (cont)
			{
				slow_monsters(plev);
				sleep_monsters(plev);
			}
		}

		break;

	case 22:
		if (name) return _("嵐の音色", "Song of the Tempest");
		if (desc) return _("轟音のビームを放つ。", "Fires a beam of sound.");
    
		{
			int dice = 15 + (plev - 1) / 2;
			int sides = 10;

			if (info) return info_damage(dice, sides, 0);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_SOUND, dir, damroll(dice, sides));
			}
		}
		break;

	case 23:
		if (name) return _("もう一つの世界", "Ambarkanta");
		if (desc) return _("現在の階を再構成する。", "Recreates current dungeon level.");
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				msg_print(_("周囲が変化し始めた．．．", "You sing of the primeval shaping of Middle-earth..."));
				alter_reality();
			}
		}
		break;

	case 24:
		if (name) return _("破壊の旋律", "Wrecking Pattern");
		if (desc) return _("周囲のダンジョンを揺らし、壁と床をランダムに入れ変える。", 
			"Shakes dungeon structure, and results in random swapping of floors and walls.");

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("破壊的な歌が響きわたった．．．", "You weave a pattern of sounds to contort and shatter..."));
			start_singing(spell, MUSIC_QUAKE);
		}

		{
			int rad = 10;

			if (info) return info_radius(rad);

			if (cont)
			{
				earthquake(p_ptr->y, p_ptr->x, 10);
			}
		}

		break;


	case 25:
		if (name) return _("停滞の歌", "Stationary Shriek");
		if (desc) return _("視界内の全てのモンスターを麻痺させようとする。抵抗されると無効。", "Attempts to freeze all monsters in sight.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("ゆっくりとしたメロディを奏で始めた．．．", "You weave a very slow pattern which is almost likely to stop..."));
			start_singing(spell, MUSIC_STASIS);
		}

		{
			int power = plev * 4;

			if (info) return info_power(power);

			if (cont)
			{
				stasis_monsters(power);
			}
		}

		break;

	case 26:
		if (name) return _("守りの歌", "Endurance");
		if (desc) return _("自分のいる床の上に、モンスターが通り抜けたり召喚されたりすることができなくなるルーンを描く。", 
			"Sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break glyph.");
    
		{
			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				msg_print(_("歌が神聖な場を作り出した．．．", "The holy power of the Music is creating sacred field..."));
				warding_glyph();
			}
		}
		break;

	case 27:
		if (name) return _("英雄の詩", "The Hero's Poem");
		if (desc) return _("加速し、ヒーロー気分になり、視界内の全てのモンスターにダメージを与える。", 
			"Hastes you. Gives heroism. Damages all monsters in sight.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("英雄の歌を口ずさんだ．．．", "You chant a powerful, heroic call to arms..."));
			(void)hp_player(10);
			(void)set_afraid(0);

			/* Recalculate hitpoints */
			p_ptr->update |= (PU_HP);

			start_singing(spell, MUSIC_SHERO);
		}

		if (stop)
		{
			if (!p_ptr->hero)
			{
				msg_print(_("ヒーローの気分が消え失せた。", "The heroism wears off."));
				/* Recalculate hitpoints */
				p_ptr->update |= (PU_HP);
			}

			if (!p_ptr->fast)
			{
				msg_print(_("動きの素早さがなくなったようだ。", "You feel yourself slow down."));
			}
		}

		{
			int dice = 1;
			int sides = plev * 3;

			if (info) return info_damage(dice, sides, 0);

			if (cont)
			{
				dispel_monsters(damroll(dice, sides));
			}
		}
		break;

	case 28:
		if (name) return _("ヤヴァンナの助け", "Relief of Yavanna");
		if (desc) return _("強力な回復の歌で、負傷と朦朧状態も全快する。", "Powerful healing song. Also heals cut and stun completely.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("歌を通して体に活気が戻ってきた．．．", "Life flows through you as you sing the song..."));
			start_singing(spell, MUSIC_H_LIFE);
		}

		{
			int dice = 15;
			int sides = 10;

			if (info) return info_heal(dice, sides, 0);

			if (cont)
			{
				hp_player(damroll(dice, sides));
				set_stun(0);
				set_cut(0);
			}
		}

		break;

	case 29:
		if (name) return _("再生の歌", "Goddess' rebirth");
		if (desc) return _("すべてのステータスと経験値を回復する。", "Restores all stats and experience.");
    
		{
			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				msg_print(_("暗黒の中に光と美をふりまいた。体が元の活力を取り戻した。",
							"You strewed light and beauty in the dark as you sing. You feel refreshed."));
				(void)do_res_stat(A_STR);
				(void)do_res_stat(A_INT);
				(void)do_res_stat(A_WIS);
				(void)do_res_stat(A_DEX);
				(void)do_res_stat(A_CON);
				(void)do_res_stat(A_CHR);
				(void)restore_level();
			}
		}
		break;

	case 30:
		if (name) return _("サウロンの魔術", "Wizardry of Sauron");
		if (desc) return _("非常に強力でごく小さい轟音の球を放つ。", "Fires an extremely powerful tiny ball of sound.");
    
		{
			int dice = 50 + plev;
			int sides = 10;
			int rad = 0;

			if (info) return info_damage(dice, sides, 0);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_SOUND, dir, damroll(dice, sides), rad);
			}
		}
		break;

	case 31:
		if (name) return _("フィンゴルフィンの挑戦", "Fingolfin's Challenge");
		if (desc) return _("ダメージを受けなくなるバリアを張る。", 
			"Generates barrier which completely protect you from almost all damages. Takes a few your turns when the barrier breaks.");
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
			msg_print(_("フィンゴルフィンの冥王への挑戦を歌った．．．",
						"You recall the valor of Fingolfin's challenge to the Dark Lord..."));

			/* Redraw map */
			p_ptr->redraw |= (PR_MAP);
		
			/* Update monsters */
			p_ptr->update |= (PU_MONSTERS);
		
			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

			start_singing(spell, MUSIC_INVULN);
		}

		if (stop)
		{
			if (!p_ptr->invuln)
			{
				msg_print(_("無敵ではなくなった。", "The invulnerability wears off."));
				/* Redraw map */
				p_ptr->redraw |= (PR_MAP);

				/* Update monsters */
				p_ptr->update |= (PU_MONSTERS);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
			}
		}

		break;
	}

	return "";
}

/*!
 * @brief 剣術の各処理を行う
 * @param spell 剣術ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_CAST)
 * @return SPELL_NAME / SPELL_DESC 時には文字列ポインタを返す。SPELL_CAST時はNULL文字列を返す。
 */
static cptr do_hissatsu_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("飛飯綱", "Tobi-Izuna");
		if (desc) return _("2マス離れたところにいるモンスターを攻撃する。", "Attacks a two squares distant monster.");
    
		if (cast)
		{
			project_length = 2;
			if (!get_aim_dir(&dir)) return NULL;

			project_hook(GF_ATTACK, dir, HISSATSU_2, PROJECT_STOP | PROJECT_KILL);
		}
		break;

	case 1:
		if (name) return _("五月雨斬り", "3-Way Attack");
		if (desc) return _("3方向に対して攻撃する。", "Attacks in 3 directions in one time.");
    
		if (cast)
		{
			int cdir;
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			for (cdir = 0;cdir < 8; cdir++)
			{
				if (cdd[cdir] == dir) break;
			}

			if (cdir == 8) return NULL;

			y = p_ptr->y + ddy_cdd[cdir];
			x = p_ptr->x + ddx_cdd[cdir];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
				msg_print(_("攻撃は空を切った。", "You attack the empty air."));
			
			y = p_ptr->y + ddy_cdd[(cdir + 7) % 8];
			x = p_ptr->x + ddx_cdd[(cdir + 7) % 8];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
				msg_print(_("攻撃は空を切った。", "You attack the empty air."));
			
			y = p_ptr->y + ddy_cdd[(cdir + 1) % 8];
			x = p_ptr->x + ddx_cdd[(cdir + 1) % 8];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
				msg_print(_("攻撃は空を切った。", "You attack the empty air."));
		}
		break;

	case 2:
		if (name) return _("ブーメラン", "Boomerang");
		if (desc) return _("武器を手元に戻ってくるように投げる。戻ってこないこともある。", 
			"Throws current weapon. And it'll return to your hand unless failed.");
    
		if (cast)
		{
			if (!do_cmd_throw(1, TRUE, -1)) return NULL;
		}
		break;

	case 3:
		if (name) return _("焔霊", "Burning Strike");
		if (desc) return _("火炎耐性のないモンスターに大ダメージを与える。", "Attacks a monster with more damage unless it has resistance to fire.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_FIRE);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 4:
		if (name) return _("殺気感知", "Detect Ferocity");
		if (desc) return _("近くの思考することができるモンスターを感知する。", "Detects all monsters except mindless in your vicinity.");
    
		if (cast)
		{
			detect_monsters_mind(DETECT_RAD_DEFAULT);
		}
		break;

	case 5:
		if (name) return _("みね打ち", "Strike to Stun");
		if (desc) return _("相手にダメージを与えないが、朦朧とさせる。", "Attempts to stun a monster in the adjacent.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_MINEUCHI);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 6:
		if (name) return _("カウンター", "Counter");
		if (desc) return _("相手に攻撃されたときに反撃する。反撃するたびにMPを消費。", 
			"Prepares to counterattack. When attack by a monster, strikes back using SP each time.");
    
		if (cast)
		{
			if (p_ptr->riding)
			{
				msg_print(_("乗馬中には無理だ。", "You cannot do it when riding."));
				return NULL;
			}
			msg_print(_("相手の攻撃に対して身構えた。", "You prepare to counter blow."));
			p_ptr->counter = TRUE;
		}
		break;

	case 7:
		if (name) return _("払い抜け", "Harainuke");
		if (desc) return _("攻撃した後、反対側に抜ける。", 
			"Attacks monster with your weapons normally, then move through counter side of the monster.");
    
		if (cast)
		{
			POSITION y, x;

			if (p_ptr->riding)
			{
				msg_print(_("乗馬中には無理だ。", "You cannot do it when riding."));
				return NULL;
			}
	
			if (!get_rep_dir2(&dir)) return NULL;
	
			if (dir == 5) return NULL;
			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];
	
			if (!cave[y][x].m_idx)
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
	
			py_attack(y, x, 0);
	
			if (!player_can_enter(cave[y][x].feat, 0) || is_trap(cave[y][x].feat))
				break;
	
			y += ddy[dir];
			x += ddx[dir];
	
			if (player_can_enter(cave[y][x].feat, 0) && !is_trap(cave[y][x].feat) && !cave[y][x].m_idx)
			{
				msg_print(NULL);
	
				/* Move the player */
				(void)move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
			}
		}
		break;

	case 8:
		if (name) return _("サーペンツタン", "Serpent's Tongue");
		if (desc) return _("毒耐性のないモンスターに大ダメージを与える。", "Attacks a monster with more damage unless it has resistance to poison.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_POISON);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 9:
		if (name) return _("斬魔剣弐の太刀", "Zammaken");
		if (desc) return _("生命のない邪悪なモンスターに大ダメージを与えるが、他のモンスターには全く効果がない。", 
			"Attacks an evil unliving monster with great damage. No effect to other  monsters.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_ZANMA);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 10:
		if (name) return _("裂風剣", "Wind Blast");
		if (desc) return _("攻撃した相手を後方へ吹き飛ばす。", "Attacks an adjacent monster, and blow it away.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
			if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
			{
				return "";
			}
			if (cave[y][x].m_idx)
			{
				int i;
				POSITION ty = y, tx = x;
				POSITION oy = y, ox = x;
				MONSTER_IDX m_idx = cave[y][x].m_idx;
				monster_type *m_ptr = &m_list[m_idx];
				char m_name[80];
	
				monster_desc(m_name, m_ptr, 0);
	
				for (i = 0; i < 5; i++)
				{
					y += ddy[dir];
					x += ddx[dir];
					if (cave_empty_bold(y, x))
					{
						ty = y;
						tx = x;
					}
					else break;
				}
				if ((ty != oy) || (tx != ox))
				{
					msg_format(_("%sを吹き飛ばした！", "You blow %s away!"), m_name);
					cave[oy][ox].m_idx = 0;
					cave[ty][tx].m_idx = m_idx;
					m_ptr->fy = ty;
					m_ptr->fx = tx;
	
					update_mon(m_idx, TRUE);
					lite_spot(oy, ox);
					lite_spot(ty, tx);
	
					if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
						p_ptr->update |= (PU_MON_LITE);
				}
			}
		}
		break;

	case 11:
		if (name) return _("刀匠の目利き", "Judge");
		if (desc) return _("武器・防具を1つ識別する。レベル45以上で武器・防具の能力を完全に知ることができる。", 
			"Identifies a weapon or armor. Or *identifies* these at level 45.");
    
		if (cast)
		{
			if (plev > 44)
			{
				if (!identify_fully(TRUE)) return NULL;
			}
			else
			{
				if (!ident_spell(TRUE)) return NULL;
			}
		}
		break;

	case 12:
		if (name) return _("破岩斬", "Rock Smash");
		if (desc) return _("岩を壊し、岩石系のモンスターに大ダメージを与える。", "Breaks rock. Or greatly damage a monster made by rocks.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_HAGAN);
	
			if (!cave_have_flag_bold(y, x, FF_HURT_ROCK)) break;
	
			/* Destroy the feature */
			cave_alter_feat(y, x, FF_HURT_ROCK);
	
			/* Update some things */
			p_ptr->update |= (PU_FLOW);
		}
		break;

	case 13:
		if (name) return _("乱れ雪月花", "Midare-Setsugekka");
		if (desc) return _("攻撃回数が増え、冷気耐性のないモンスターに大ダメージを与える。", 
			"Attacks a monster with increased number of attacks and more damage unless it has resistance to cold.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_COLD);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 14:
		if (name) return _("急所突き", "Spot Aiming");
		if (desc) return _("モンスターを一撃で倒す攻撃を繰り出す。失敗すると1点しかダメージを与えられない。", 
			"Attempts to kill a monster instantly. If failed cause only 1HP of damage.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_KYUSHO);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 15:
		if (name) return _("魔神斬り", "Majingiri");
		if (desc) return _("会心の一撃で攻撃する。攻撃がかわされやすい。", 
			"Attempts to attack with critical hit. But this attack is easy to evade for a monster.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_MAJIN);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 16:
		if (name) return _("捨て身", "Desperate Attack");
		if (desc) return _("強力な攻撃を繰り出す。次のターンまでの間、食らうダメージが増える。", 
			"Attacks with all of your power. But all damages you take will be doubled for one turn.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_SUTEMI);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
			p_ptr->sutemi = TRUE;
		}
		break;

	case 17:
		if (name) return _("雷撃鷲爪斬", "Lightning Eagle");
		if (desc) return _("電撃耐性のないモンスターに非常に大きいダメージを与える。", 
			"Attacks a monster with more damage unless it has resistance to electricity.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_ELEC);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 18:
		if (name) return _("入身", "Rush Attack");
		if (desc) return _("素早く相手に近寄り攻撃する。", "Steps close to a monster and attacks at a time.");
    
		if (cast)
		{
			if (!rush_attack(NULL)) return NULL;
		}
		break;

	case 19:
		if (name) return _("赤流渦", "Bloody Maelstrom");
		if (desc) return _("自分自身も傷を作りつつ、その傷が深いほど大きい威力で全方向の敵を攻撃できる。生きていないモンスターには効果がない。", 
			"Attacks all adjacent monsters with power corresponding to your cut status. Then increases your cut status. No effect to unliving monsters.");
    
		if (cast)
		{
			int y = 0, x = 0;

			cave_type       *c_ptr;
			monster_type    *m_ptr;
	
			if (p_ptr->cut < 300)
				set_cut(p_ptr->cut + 300);
			else
				set_cut(p_ptr->cut * 2);
	
			for (dir = 0; dir < 8; dir++)
			{
				y = p_ptr->y + ddy_ddd[dir];
				x = p_ptr->x + ddx_ddd[dir];
				c_ptr = &cave[y][x];
	
				/* Get the monster */
				m_ptr = &m_list[c_ptr->m_idx];
	
				/* Hack -- attack monsters */
				if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
				{
					if (!monster_living(&r_info[m_ptr->r_idx]))
					{
						char m_name[80];
	
						monster_desc(m_name, m_ptr, 0);
						msg_format(_("%sには効果がない！", "%s is unharmed!"), m_name);
					}
					else py_attack(y, x, HISSATSU_SEKIRYUKA);
				}
			}
		}
		break;

	case 20:
		if (name) return _("激震撃", "Earthquake Blow");
		if (desc) return _("地震を起こす。", "Shakes dungeon structure, and results in random swapping of floors and walls.");
    
		if (cast)
		{
			int y,x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_QUAKE);
			else
				earthquake(p_ptr->y, p_ptr->x, 10);
		}
		break;

	case 21:
		if (name) return _("地走り", "Crack");
		if (desc) return _("衝撃波のビームを放つ。", "Fires a beam of shock wave.");
    
		if (cast)
		{
			int total_damage = 0, basedam, i;
			u32b flgs[TR_FLAG_SIZE];
			object_type *o_ptr;
			if (!get_aim_dir(&dir)) return NULL;
			msg_print(_("武器を大きく振り下ろした。", "You swing your weapon downward."));
			for (i = 0; i < 2; i++)
			{
				int damage;
	
				if (!buki_motteruka(INVEN_RARM+i)) break;
				o_ptr = &inventory[INVEN_RARM+i];
				basedam = (o_ptr->dd * (o_ptr->ds + 1)) * 50;
				damage = o_ptr->to_d * 100;
				object_flags(o_ptr, flgs);
				if ((o_ptr->name1 == ART_VORPAL_BLADE) || (o_ptr->name1 == ART_CHAINSWORD))
				{
					/* vorpal blade */
					basedam *= 5;
					basedam /= 3;
				}
				else if (have_flag(flgs, TR_VORPAL))
				{
					/* vorpal flag only */
					basedam *= 11;
					basedam /= 9;
				}
				damage += basedam;
				damage *= p_ptr->num_blow[i];
				total_damage += damage / 200;
				if (i) total_damage = total_damage*7/10;
			}
			fire_beam(GF_FORCE, dir, total_damage);
		}
		break;

	case 22:
		if (name) return _("気迫の雄叫び", "War Cry");
		if (desc) return _("視界内の全モンスターに対して轟音の攻撃を行う。さらに、近くにいるモンスターを怒らせる。", 
			"Damages all monsters in sight with sound. Aggravate nearby monsters.");
    
		if (cast)
		{
			msg_print(_("雄叫びをあげた！", "You roar out!"));
			project_hack(GF_SOUND, randint1(plev * 3));
			aggravate_monsters(0);
		}
		break;

	case 23:
		if (name) return _("無双三段", "Musou-Sandan");
		if (desc) return _("強力な3段攻撃を繰り出す。", "Attacks with powerful 3 strikes.");
    
		if (cast)
		{
			int i;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			for (i = 0; i < 3; i++)
			{
				POSITION y, x;
				POSITION ny, nx;
				MONSTER_IDX m_idx;
				cave_type *c_ptr;
				monster_type *m_ptr;
	
				y = p_ptr->y + ddy[dir];
				x = p_ptr->x + ddx[dir];
				c_ptr = &cave[y][x];
	
				if (c_ptr->m_idx)
					py_attack(y, x, HISSATSU_3DAN);
				else
				{
					msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
					return NULL;
				}
	
				if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
				{
					return "";
				}
	
				/* Monster is dead? */
				if (!c_ptr->m_idx) break;
	
				ny = y + ddy[dir];
				nx = x + ddx[dir];
				m_idx = c_ptr->m_idx;
				m_ptr = &m_list[m_idx];
	
				/* Monster cannot move back? */
				if (!monster_can_enter(ny, nx, &r_info[m_ptr->r_idx], 0))
				{
					/* -more- */
					if (i < 2) msg_print(NULL);
					continue;
				}
	
				c_ptr->m_idx = 0;
				cave[ny][nx].m_idx = m_idx;
				m_ptr->fy = ny;
				m_ptr->fx = nx;
	
				update_mon(m_idx, TRUE);
	
				/* Redraw the old spot */
				lite_spot(y, x);
	
				/* Redraw the new spot */
				lite_spot(ny, nx);
	
				/* Player can move forward? */
				if (player_can_enter(c_ptr->feat, 0))
				{
					/* Move the player */
					if (!move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP)) break;
				}
				else
				{
					break;
				}

				/* -more- */
				if (i < 2) msg_print(NULL);
			}
		}
		break;

	case 24:
		if (name) return _("吸血鬼の牙", "Vampire's Fang");
		if (desc) return _("攻撃した相手の体力を吸いとり、自分の体力を回復させる。生命を持たないモンスターには通じない。", 
			"Attacks with vampiric strikes which absorbs HP from a monster and gives them to you. No effect to unliving monsters.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_DRAIN);
			else
			{
					msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 25:
		if (name) return _("幻惑", "Moon Dazzling");
		if (desc) return _("視界内の起きている全モンスターに朦朧、混乱、眠りを与えようとする。", "Attempts to stun, confuse and sleep all waking monsters.");
    
		if (cast)
		{
			msg_print(_("武器を不規則に揺らした．．．", "You irregularly wave your weapon..."));
			project_hack(GF_ENGETSU, plev * 4);
			project_hack(GF_ENGETSU, plev * 4);
			project_hack(GF_ENGETSU, plev * 4);
		}
		break;

	case 26:
		if (name) return _("百人斬り", "Hundred Slaughter");
		if (desc) return _("連続して入身でモンスターを攻撃する。攻撃するたびにMPを消費。MPがなくなるか、モンスターを倒せなかったら百人斬りは終了する。", 
			"Performs a series of rush attacks. The series continues while killing each monster in a time and SP remains.");
    
		if (cast)
		{
			const int mana_cost_per_monster = 8;
			bool is_new = TRUE;
			bool mdeath;

			do
			{
				if (!rush_attack(&mdeath)) break;
				if (is_new)
				{
					/* Reserve needed mana point */
					p_ptr->csp -= technic_info[REALM_HISSATSU - MIN_TECHNIC][26].smana;
					is_new = FALSE;
				}
				else
					p_ptr->csp -= mana_cost_per_monster;

				if (!mdeath) break;
				command_dir = 0;

				p_ptr->redraw |= PR_MANA;
				handle_stuff();
			}
			while (p_ptr->csp > mana_cost_per_monster);

			if (is_new) return NULL;
	
			/* Restore reserved mana */
			p_ptr->csp += technic_info[REALM_HISSATSU - MIN_TECHNIC][26].smana;
		}
		break;

	case 27:
		if (name) return _("天翔龍閃", "Dragonic Flash");
		if (desc) return _("視界内の場所を指定して、その場所と自分の間にいる全モンスターを攻撃し、その場所に移動する。", 
			"Runs toward given location while attacking all monsters on the path.");
    
		if (cast)
		{
			POSITION y, x;

			if (!tgt_pt(&x, &y)) return NULL;

			if (!cave_player_teleportable_bold(y, x, 0L) ||
			    (distance(y, x, p_ptr->y, p_ptr->x) > MAX_SIGHT / 2) ||
			    !projectable(p_ptr->y, p_ptr->x, y, x))
			{
				msg_print(_("失敗！", "You cannot move to that place!"));
				break;
			}
			if (p_ptr->anti_tele)
			{
				msg_print(_("不思議な力がテレポートを防いだ！", "A mysterious force prevents you from teleporting!"));
				break;
			}
			project(0, 0, y, x, HISSATSU_ISSEN, GF_ATTACK, PROJECT_BEAM | PROJECT_KILL, -1);
			teleport_player_to(y, x, 0L);
		}
		break;

	case 28:
		if (name) return _("二重の剣撃", "Twin Slash");
		if (desc) return _("1ターンで2度攻撃を行う。", "double attacks at a time.");
    
		if (cast)
		{
			int x, y;
	
			if (!get_rep_dir(&dir, FALSE)) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
			{
				py_attack(y, x, 0);
				if (cave[y][x].m_idx)
				{
					handle_stuff();
					py_attack(y, x, 0);
				}
			}
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "You don't see any monster in this direction"));
				return NULL;
			}
		}
		break;

	case 29:
		if (name) return _("虎伏絶刀勢", "Kofuku-Zettousei");
		if (desc) return _("強力な攻撃を行い、近くの場所にも効果が及ぶ。", "Performs a powerful attack which even effect nearby monsters.");
    
		if (cast)
		{
			int total_damage = 0, basedam, i;
			int y, x;
			u32b flgs[TR_FLAG_SIZE];
			object_type *o_ptr;
	
			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
			{
				msg_print(_("なぜか攻撃することができない。", "Something prevent you from attacking."));
				return "";
			}
			msg_print(_("武器を大きく振り下ろした。", "You swing your weapon downward."));
			for (i = 0; i < 2; i++)
			{
				int damage;
				if (!buki_motteruka(INVEN_RARM+i)) break;
				o_ptr = &inventory[INVEN_RARM+i];
				basedam = (o_ptr->dd * (o_ptr->ds + 1)) * 50;
				damage = o_ptr->to_d * 100;
				object_flags(o_ptr, flgs);
				if ((o_ptr->name1 == ART_VORPAL_BLADE) || (o_ptr->name1 == ART_CHAINSWORD))
				{
					/* vorpal blade */
					basedam *= 5;
					basedam /= 3;
				}
				else if (have_flag(flgs, TR_VORPAL))
				{
					/* vorpal flag only */
					basedam *= 11;
					basedam /= 9;
				}
				damage += basedam;
				damage += p_ptr->to_d[i] * 100;
				damage *= p_ptr->num_blow[i];
				total_damage += (damage / 100);
			}
			project(0, (cave_have_flag_bold(y, x, FF_PROJECT) ? 5 : 0), y, x, total_damage * 3 / 2, GF_METEOR, PROJECT_KILL | PROJECT_JUMP | PROJECT_ITEM, -1);
		}
		break;

	case 30:
		if (name) return _("慶雲鬼忍剣", "Keiun-Kininken");
		if (desc) return _("自分もダメージをくらうが、相手に非常に大きなダメージを与える。アンデッドには特に効果がある。", 
			"Attacks a monster with extremely powerful damage. But you also takes some damages. Hurts a undead monster greatly.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_UNDEAD);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
			take_hit(DAMAGE_NOESCAPE, 100 + randint1(100), _("慶雲鬼忍剣を使った衝撃", "exhaustion on using Keiun-Kininken"), -1);
		}
		break;

	case 31:
		if (name) return _("切腹", "Harakiri");
		if (desc) return _("「武士道とは、死ぬことと見つけたり。」", "'Busido is found in death'");

		if (cast)
		{
			int i;
			if (!get_check(_("本当に自殺しますか？", "Do you really want to commit suicide? "))) return NULL;
				/* Special Verification for suicide */
			prt(_("確認のため '@' を押して下さい。", "Please verify SUICIDE by typing the '@' sign: "), 0, 0);
	
			flush();
			i = inkey();
			prt("", 0, 0);
			if (i != '@') return NULL;
			if (p_ptr->total_winner)
			{
				take_hit(DAMAGE_FORCE, 9999, "Seppuku", -1);
				p_ptr->total_winner = TRUE;
			}
			else
			{
				msg_print(_("武士道とは、死ぬことと見つけたり。", "Meaning of Bushi-do is found in the death."));
				take_hit(DAMAGE_FORCE, 9999, "Seppuku", -1);
			}
		}
		break;
	}

	return "";
}

/*!
 * @brief 呪術領域の武器呪縛の対象にできる武器かどうかを返す。 / An "item_tester_hook" for offer
 * @param o_ptr オブジェクト構造体の参照ポインタ
 * @return 呪縛可能な武器ならばTRUEを返す
 */
static bool item_tester_hook_weapon_except_bow(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}

/*!
 * @brief 呪術領域の各処理に使える呪われた装備かどうかを返す。 / An "item_tester_hook" for offer
 * @param o_ptr オブジェクト構造体の参照ポインタ
 * @return 使える装備ならばTRUEを返す
 */
static bool item_tester_hook_cursed(object_type *o_ptr)
{
	return (bool)(object_is_cursed(o_ptr));
}

/*!
 * @brief 呪術領域魔法の各処理を行う
 * @param spell 魔法ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST / SPELL_CONT / SPELL_STOP)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST / SPELL_CONT / SPELL_STOP 時はNULL文字列を返す。
 */
static cptr do_hex_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
	bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
	bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;

	bool add = TRUE;

	PLAYER_LEVEL plev = p_ptr->lev;
	HIT_POINT power;

	switch (spell)
	{
	/*** 1st book (0-7) ***/
	case 0:
		if (name) return _("邪なる祝福", "Evily blessing");
		if (desc) return _("祝福により攻撃精度と防御力が上がる。", "Attempts to increase +to_hit of a weapon and AC");
		if (cast)
		{
			if (!p_ptr->blessed)
			{
				msg_print(_("高潔な気分になった！", "You feel righteous!"));
			}
		}
		if (stop)
		{
			if (!p_ptr->blessed)
			{
				msg_print(_("高潔な気分が消え失せた。", "The prayer has expired."));
			}
		}
		break;

	case 1:
		if (name) return _("軽傷の治癒", "Cure light wounds");
		if (desc) return _("HPや傷を少し回復させる。", "Heals cut and HP a little.");
		if (info) return info_heal(1, 10, 0);
		if (cast)
		{
			msg_print(_("気分が良くなってくる。", "You feel better and better."));
		}
		if (cast || cont)
		{
			hp_player(damroll(1, 10));
			set_cut(p_ptr->cut - 10);
		}
		break;

	case 2:
		if (name) return _("悪魔のオーラ", "Demonic aura");
		if (desc) return _("炎のオーラを身にまとい、回復速度が速くなる。", "Gives fire aura and regeneration.");
		if (cast)
		{
			msg_print(_("体が炎のオーラで覆われた。", "You have enveloped by fiery aura!"));
		}
		if (stop)
		{
			msg_print(_("炎のオーラが消え去った。", "Fiery aura disappeared."));
		}
		break;

	case 3:
		if (name) return _("悪臭霧", "Stinking mist");
		if (desc) return _("視界内のモンスターに微弱量の毒のダメージを与える。", "Deals few damages of poison to all monsters in your sight.");
		power = plev / 2 + 5;
		if (info) return info_damage(1, power, 0);
		if (cast || cont)
		{
			project_hack(GF_POIS, randint1(power));
		}
		break;

	case 4:
		if (name) return _("腕力強化", "Extra might");
		if (desc) return _("術者の腕力を上昇させる。", "Attempts to increase your strength.");
		if (cast)
		{
			msg_print(_("何だか力が湧いて来る。", "You feel you get stronger."));
		}
		break;

	case 5:
		if (name) return _("武器呪縛", "Curse weapon");
		if (desc) return _("装備している武器を呪う。", "Curses your weapon.");
		if (cast)
		{
			OBJECT_IDX item;
			cptr q, s;
			char o_name[MAX_NLEN];
			object_type *o_ptr;
			u32b f[TR_FLAG_SIZE];

			item_tester_hook = item_tester_hook_weapon_except_bow;
			q = _("どれを呪いますか？", "Which weapon do you curse?");
			s = _("武器を装備していない。", "You wield no weapons.");

			if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

			o_ptr = &inventory[item];
			object_desc(o_name, o_ptr, OD_NAME_ONLY);
			object_flags(o_ptr, f);

			if (!get_check(format(_("本当に %s を呪いますか？", "Do you curse %s, really？"), o_name))) return FALSE;

			if (!one_in_(3) &&
				(object_is_artifact(o_ptr) || have_flag(f, TR_BLESSED)))
			{
				msg_format(_("%s は呪いを跳ね返した。", "%s resists the effect."), o_name);
				if (one_in_(3))
				{
					if (o_ptr->to_d > 0)
					{
						o_ptr->to_d -= randint1(3) % 2;
						if (o_ptr->to_d < 0) o_ptr->to_d = 0;
					}
					if (o_ptr->to_h > 0)
					{
						o_ptr->to_h -= randint1(3) % 2;
						if (o_ptr->to_h < 0) o_ptr->to_h = 0;
					}
					if (o_ptr->to_a > 0)
					{
						o_ptr->to_a -= randint1(3) % 2;
						if (o_ptr->to_a < 0) o_ptr->to_a = 0;
					}
					msg_format(_("%s は劣化してしまった。", "Your %s was disenchanted!"), o_name);
				}
			}
			else
			{
				int curse_rank = 0;
				msg_format(_("恐怖の暗黒オーラがあなたの%sを包み込んだ！", "A terrible black aura blasts your %s!"), o_name);
				o_ptr->curse_flags |= (TRC_CURSED);

				if (object_is_artifact(o_ptr) || object_is_ego(o_ptr))
				{

					if (one_in_(3)) o_ptr->curse_flags |= (TRC_HEAVY_CURSE);
					if (one_in_(666))
					{
						o_ptr->curse_flags |= (TRC_TY_CURSE);
						if (one_in_(666)) o_ptr->curse_flags |= (TRC_PERMA_CURSE);

						add_flag(o_ptr->art_flags, TR_AGGRAVATE);
						add_flag(o_ptr->art_flags, TR_VORPAL);
						add_flag(o_ptr->art_flags, TR_VAMPIRIC);
						msg_print(_("血だ！血だ！血だ！", "Blood, Blood, Blood!"));
						curse_rank = 2;
					}
				}

				o_ptr->curse_flags |= get_curse(curse_rank, o_ptr);
			}

			p_ptr->update |= (PU_BONUS);
			add = FALSE;
		}
		break;

	case 6:
		if (name) return _("邪悪感知", "Evil detection");
		if (desc) return _("周囲の邪悪なモンスターを感知する。", "Detects evil monsters.");
		if (info) return info_range(MAX_SIGHT);
		if (cast)
		{
			msg_print(_("邪悪な生物の存在を感じ取ろうとした。", "You attend to the presence of evil creatures."));
		}
		break;

	case 7:
		if (name) return _("我慢", "Patience");
		if (desc) return _("数ターン攻撃を耐えた後、受けたダメージを地獄の業火として周囲に放出する。", 
			"Bursts hell fire strongly after patients any damage while few turns.");
		power = MIN(200, (HEX_REVENGE_POWER(p_ptr) * 2));
		if (info) return info_damage(0, 0, power);
		if (cast)
		{
			int a = 3 - (p_ptr->pspeed - 100) / 10;
			MAGIC_NUM2 r = 3 + randint1(3) + MAX(0, MIN(3, a));

			if (HEX_REVENGE_TURN(p_ptr) > 0)
			{
				msg_print(_("すでに我慢をしている。", "You are already patienting."));
				return NULL;
			}

			HEX_REVENGE_TYPE(p_ptr) = 1;
			HEX_REVENGE_TURN(p_ptr) = r;
			HEX_REVENGE_POWER(p_ptr) = 0;
			msg_print(_("じっと耐えることにした。", "You decide to patient all damages."));
			add = FALSE;
		}
		if (cont)
		{
			int rad = 2 + (power / 50);

			HEX_REVENGE_TURN(p_ptr)--;

			if ((HEX_REVENGE_TURN(p_ptr) <= 0) || (power >= 200))
			{
				msg_print(_("我慢が解かれた！", "Time for end of patioence!"));
				if (power)
				{
					project(0, rad, p_ptr->y, p_ptr->x, power, GF_HELL_FIRE,
						(PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL), -1);
				}
				if (p_ptr->wizard)
				{
					msg_format(_("%d点のダメージを返した。", "You return %d damages."), power);
				}

				/* Reset */
				HEX_REVENGE_TYPE(p_ptr) = 0;
				HEX_REVENGE_TURN(p_ptr) = 0;
				HEX_REVENGE_POWER(p_ptr) = 0;
			}
		}
		break;

	/*** 2nd book (8-15) ***/
	case 8:
		if (name) return _("氷の鎧", "Ice armor");
		if (desc) return _("氷のオーラを身にまとい、防御力が上昇する。", "Gives fire aura and bonus to AC.");
		if (cast)
		{
			msg_print(_("体が氷の鎧で覆われた。", "You have enveloped by ice armor!"));
		}
		if (stop)
		{
			msg_print(_("氷の鎧が消え去った。", "Ice armor disappeared."));
		}
		break;

	case 9:
		if (name) return _("重傷の治癒", "Cure serious wounds");
		if (desc) return _("体力や傷を多少回復させる。", "Heals cut and HP more.");
		if (info) return info_heal(2, 10, 0);
		if (cast)
		{
			msg_print(_("気分が良くなってくる。", "You feel better and better."));
		}
		if (cast || cont)
		{
			hp_player(damroll(2, 10));
			set_cut((p_ptr->cut / 2) - 10);
		}
		break;

	case 10:
		if (name) return _("薬品吸入", "Inhail potion");
		if (desc) return _("呪文詠唱を中止することなく、薬の効果を得ることができる。", "Quaffs a potion without canceling of casting a spell.");
		if (cast)
		{
			CASTING_HEX_FLAGS(p_ptr) |= (1L << HEX_INHAIL);
			do_cmd_quaff_potion();
			CASTING_HEX_FLAGS(p_ptr) &= ~(1L << HEX_INHAIL);
			add = FALSE;
		}
		break;

	case 11:
		if (name) return _("衰弱の霧", "Hypodynamic mist");
		if (desc) return _("視界内のモンスターに微弱量の衰弱属性のダメージを与える。", 
			"Deals few damages of hypodynamia to all monsters in your sight.");
		power = (plev / 2) + 5;
		if (info) return info_damage(1, power, 0);
		if (cast || cont)
		{
			project_hack(GF_HYPODYNAMIA, randint1(power));
		}
		break;

	case 12:
		if (name) return _("魔剣化", "Swords to runeswords");
		if (desc) return _("武器の攻撃力を上げる。切れ味を得、呪いに応じて与えるダメージが上昇し、善良なモンスターに対するダメージが2倍になる。", 
			"Gives vorpal ability to your weapon. Increases damages by your weapon acccording to curse of your weapon.");
		if (cast)
		{
#ifdef JP
			msg_print("あなたの武器が黒く輝いた。");
#else
			if (!empty_hands(FALSE))
				msg_print("Your weapons glow bright black.");
			else
				msg_print("Your weapon glows bright black.");
#endif
		}
		if (stop)
		{
#ifdef JP
			msg_print("武器の輝きが消え去った。");
#else
			msg_format("Brightness of weapon%s disappeared.", (empty_hands(FALSE)) ? "" : "s");
#endif
		}
		break;

	case 13:
		if (name) return _("混乱の手", "Touch of confusion");
		if (desc) return _("攻撃した際モンスターを混乱させる。", "Confuses a monster when you attack.");
		if (cast)
		{
			msg_print(_("あなたの手が赤く輝き始めた。", "Your hands glow bright red."));
		}
		if (stop)
		{
			msg_print(_("手の輝きがなくなった。", "Brightness on your hands disappeard."));
		}
		break;

	case 14:
		if (name) return _("肉体強化", "Building up");
		if (desc) return _("術者の腕力、器用さ、耐久力を上昇させる。攻撃回数の上限を 1 増加させる。", 
			"Attempts to increases your strength, dexterity and constitusion.");
		if (cast)
		{
			msg_print(_("身体が強くなった気がした。", "You feel your body is developed more now."));
		}
		break;

	case 15:
		if (name) return _("反テレポート結界", "Anti teleport barrier");
		if (desc) return _("視界内のモンスターのテレポートを阻害するバリアを張る。", "Obstructs all teleportations by monsters in your sight.");
		power = plev * 3 / 2;
		if (info) return info_power(power);
		if (cast)
		{
			msg_print(_("テレポートを防ぐ呪いをかけた。", "You feel anyone can not teleport except you."));
		}
		break;

	/*** 3rd book (16-23) ***/
	case 16:
		if (name) return _("衝撃のクローク", "Cloak of shock");
		if (desc) return _("電気のオーラを身にまとい、動きが速くなる。", "Gives lightning aura and a bonus to speed.");
		if (cast)
		{
			msg_print(_("体が稲妻のオーラで覆われた。", "You have enveloped by electrical aura!"));
		}
		if (stop)
		{
			msg_print(_("稲妻のオーラが消え去った。", "Electrical aura disappeared."));
		}
		break;

	case 17:
		if (name) return _("致命傷の治癒", "Cure critical wounds");
		if (desc) return _("体力や傷を回復させる。", "Heals cut and HP greatry.");
		if (info) return info_heal(4, 10, 0);
		if (cast)
		{
			msg_print(_("気分が良くなってくる。", "You feel better and better."));
		}
		if (cast || cont)
		{
			hp_player(damroll(4, 10));
			set_stun(0);
			set_cut(0);
			set_poisoned(0);
		}
		break;

	case 18:
		if (name) return _("呪力封入", "Recharging");
		if (desc) return _("魔法の道具に魔力を再充填する。", "Recharges a magic device.");
		power = plev * 2;
		if (info) return info_power(power);
		if (cast)
		{
			if (!recharge(power)) return NULL;
			add = FALSE;
		}
		break;

	case 19:
		if (name) return _("死者復活", "Animate Dead");
		if (desc) return _("死体を蘇らせてペットにする。", "Raises corpses and skeletons from dead.");
		if (cast)
		{
			msg_print(_("死者への呼びかけを始めた。", "You start to call deads.!"));
		}
		if (cast || cont)
		{
			animate_dead(0, p_ptr->y, p_ptr->x);
		}
		break;

	case 20:
		if (name) return _("防具呪縛", "Curse armor");
		if (desc) return _("装備している防具に呪いをかける。", "Curse a piece of armour that you wielding.");
		if (cast)
		{
			OBJECT_IDX item;
			cptr q, s;
			char o_name[MAX_NLEN];
			object_type *o_ptr;
			u32b f[TR_FLAG_SIZE];

			item_tester_hook = object_is_armour;
			q = _("どれを呪いますか？", "Which piece of armour do you curse?");
			s = _("防具を装備していない。", "You wield no piece of armours.");

			if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

			o_ptr = &inventory[item];
			object_desc(o_name, o_ptr, OD_NAME_ONLY);
			object_flags(o_ptr, f);

			if (!get_check(format(_("本当に %s を呪いますか？", "Do you curse %s, really？"), o_name))) return FALSE;

			if (!one_in_(3) &&
				(object_is_artifact(o_ptr) || have_flag(f, TR_BLESSED)))
			{
				msg_format(_("%s は呪いを跳ね返した。", "%s resists the effect."), o_name);
				if (one_in_(3))
				{
					if (o_ptr->to_d > 0)
					{
						o_ptr->to_d -= randint1(3) % 2;
						if (o_ptr->to_d < 0) o_ptr->to_d = 0;
					}
					if (o_ptr->to_h > 0)
					{
						o_ptr->to_h -= randint1(3) % 2;
						if (o_ptr->to_h < 0) o_ptr->to_h = 0;
					}
					if (o_ptr->to_a > 0)
					{
						o_ptr->to_a -= randint1(3) % 2;
						if (o_ptr->to_a < 0) o_ptr->to_a = 0;
					}
					msg_format(_("%s は劣化してしまった。", "Your %s was disenchanted!"), o_name);
				}
			}
			else
			{
				int curse_rank = 0;
				msg_format(_("恐怖の暗黒オーラがあなたの%sを包み込んだ！", "A terrible black aura blasts your %s!"), o_name);
				o_ptr->curse_flags |= (TRC_CURSED);

				if (object_is_artifact(o_ptr) || object_is_ego(o_ptr))
				{

					if (one_in_(3)) o_ptr->curse_flags |= (TRC_HEAVY_CURSE);
					if (one_in_(666))
					{
						o_ptr->curse_flags |= (TRC_TY_CURSE);
						if (one_in_(666)) o_ptr->curse_flags |= (TRC_PERMA_CURSE);

						add_flag(o_ptr->art_flags, TR_AGGRAVATE);
						add_flag(o_ptr->art_flags, TR_RES_POIS);
						add_flag(o_ptr->art_flags, TR_RES_DARK);
						add_flag(o_ptr->art_flags, TR_RES_NETHER);
						msg_print(_("血だ！血だ！血だ！", "Blood, Blood, Blood!"));
						curse_rank = 2;
					}
				}

				o_ptr->curse_flags |= get_curse(curse_rank, o_ptr);
			}

			p_ptr->update |= (PU_BONUS);
			add = FALSE;
		}
		break;

	case 21:
		if (name) return _("影のクローク", "Cloak of shadow");
		if (desc) return _("影のオーラを身にまとい、敵に影のダメージを与える。", "Gives aura of shadow.");
		if (cast)
		{
			object_type *o_ptr = &inventory[INVEN_OUTER];

			if (!o_ptr->k_idx)
			{
				msg_print(_("クロークを身につけていない！", "You don't ware any cloak."));
				return NULL;
			}
			else if (!object_is_cursed(o_ptr))
			{
				msg_print(_("クロークは呪われていない！", "Your cloak is not cursed."));
				return NULL;
			}
			else
			{
				msg_print(_("影のオーラを身にまとった。", "You have enveloped by shadow aura!"));
			}
		}
		if (cont)
		{
			object_type *o_ptr = &inventory[INVEN_OUTER];

			if ((!o_ptr->k_idx) || (!object_is_cursed(o_ptr)))
			{
				do_spell(REALM_HEX, spell, SPELL_STOP);
				CASTING_HEX_FLAGS(p_ptr) &= ~(1L << spell);
				CASTING_HEX_NUM(p_ptr)--;
				if (!SINGING_SONG_ID(p_ptr)) set_action(ACTION_NONE);
			}
		}
		if (stop)
		{
			msg_print(_("影のオーラが消え去った。", "Shadow aura disappeared."));
		}
		break;

	case 22:
		if (name) return _("苦痛を魔力に", "Pains to mana");
		if (desc) return _("視界内のモンスターに精神ダメージ与え、魔力を吸い取る。", "Deals psychic damages to all monsters in sight, and drains some mana.");
		power = plev * 3 / 2;
		if (info) return info_damage(1, power, 0);
		if (cast || cont)
		{
			project_hack(GF_PSI_DRAIN, randint1(power));
		}
		break;

	case 23:
		if (name) return _("目には目を", "Eye for an eye");
		if (desc) return _("打撃や魔法で受けたダメージを、攻撃元のモンスターにも与える。", "Returns same damage which you got to the monster which damaged you.");
		if (cast)
		{
			msg_print(_("復讐したい欲望にかられた。", "You wish strongly you want to revenge anything."));
		}
		break;

	/*** 4th book (24-31) ***/
	case 24:
		if (name) return _("反増殖結界", "Anti multiply barrier");
		if (desc) return _("その階の増殖するモンスターの増殖を阻止する。", "Obstructs all multiplying by monsters in entire floor.");
		if (cast)
		{
			msg_print(_("増殖を阻止する呪いをかけた。", "You feel anyone can not already multiply."));
		}
		break;

	case 25:
		if (name) return _("全復活", "Restoration");
		if (desc) return _("経験値を徐々に復活し、減少した能力値を回復させる。", "Restores experience and status.");
		if (cast)
		{
			msg_print(_("体が元の活力を取り戻し始めた。", "You feel your lost status starting to return."));
		}
		if (cast || cont)
		{
			bool flag = FALSE;
			int d = (p_ptr->max_exp - p_ptr->exp);
			int r = (p_ptr->exp / 20);
			int i;

			if (d > 0)
			{
				if (d < r)
					p_ptr->exp = p_ptr->max_exp;
				else
					p_ptr->exp += r;

				/* Check the experience */
				check_experience();

				flag = TRUE;
			}
			for (i = A_STR; i < 6; i ++)
			{
				if (p_ptr->stat_cur[i] < p_ptr->stat_max[i])
				{
					if (p_ptr->stat_cur[i] < 18)
						p_ptr->stat_cur[i]++;
					else
						p_ptr->stat_cur[i] += 10;

					if (p_ptr->stat_cur[i] > p_ptr->stat_max[i])
						p_ptr->stat_cur[i] = p_ptr->stat_max[i];

					/* Recalculate bonuses */
					p_ptr->update |= (PU_BONUS);

					flag = TRUE;
				}
			}

			if (!flag)
			{
				msg_format(_("%sの呪文の詠唱をやめた。", "Finish casting '%^s'."), do_spell(REALM_HEX, HEX_RESTORE, SPELL_NAME));
				CASTING_HEX_FLAGS(p_ptr) &= ~(1L << HEX_RESTORE);
				if (cont) CASTING_HEX_NUM(p_ptr)--;
				if (CASTING_HEX_NUM(p_ptr)) p_ptr->action = ACTION_NONE;

				/* Redraw status */
				p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
				p_ptr->redraw |= (PR_EXTRA);

				return "";
			}
		}
		break;

	case 26:
		if (name) return _("呪力吸収", "Drain curse power");
		if (desc) return _("呪われた武器の呪いを吸収して魔力を回復する。", "Drains curse on your weapon and heals SP a little.");
		if (cast)
		{
			OBJECT_IDX item;
			cptr s, q;
			u32b f[TR_FLAG_SIZE];
			object_type *o_ptr;

			item_tester_hook = item_tester_hook_cursed;
			q = _("どの装備品から吸収しますか？", "Which cursed equipment do you drain mana from?");
			s = _("呪われたアイテムを装備していない。", "You have no cursed equipment.");

			if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

			o_ptr = &inventory[item];
			object_flags(o_ptr, f);

			p_ptr->csp += (p_ptr->lev / 5) + randint1(p_ptr->lev / 5);
			if (have_flag(f, TR_TY_CURSE) || (o_ptr->curse_flags & TRC_TY_CURSE)) p_ptr->csp += randint1(5);
			if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;

			if (o_ptr->curse_flags & TRC_PERMA_CURSE)
			{
				/* Nothing */
			}
			else if (o_ptr->curse_flags & TRC_HEAVY_CURSE)
			{
				if (one_in_(7))
				{
					msg_print(_("呪いを全て吸い取った。", "Heavy curse vanished away."));
					o_ptr->curse_flags = 0L;
				}
			}
			else if ((o_ptr->curse_flags & (TRC_CURSED)) && one_in_(3))
			{
				msg_print(_("呪いを全て吸い取った。", "Curse vanished away."));
				o_ptr->curse_flags = 0L;
			}

			add = FALSE;
		}
		break;

	case 27:
		if (name) return _("吸血の刃", "Swords to vampires");
		if (desc) return _("吸血属性で攻撃する。", "Gives vampiric ability to your weapon.");
		if (cast)
		{
#ifdef JP
			msg_print("あなたの武器が血を欲している。");
#else
			if (!empty_hands(FALSE))
				msg_print("Your weapons want more blood now.");
			else
				msg_print("Your weapon wants more blood now.");
#endif
		}
		if (stop)
		{
#ifdef JP
			msg_print("武器の渇望が消え去った。");
#else
			msg_format("Thirsty of weapon%s disappeared.", (empty_hands(FALSE)) ? "" : "s");
#endif
		}
		break;

	case 28:
		if (name) return _("朦朧の言葉", "Word of stun");
		if (desc) return _("視界内のモンスターを朦朧とさせる。", "Stuns all monsters in your sight.");
		power = plev * 4;
		if (info) return info_power(power);
		if (cast || cont)
		{
			stun_monsters(power);
		}
		break;

	case 29:
		if (name) return _("影移動", "Moving into shadow");
		if (desc) return _("モンスターの隣のマスに瞬間移動する。", "Teleports you close to a monster.");
		if (cast)
		{
			int i, dir;
			POSITION y, x;
			bool flag;

			for (i = 0; i < 3; i++)
			{
				if (!tgt_pt(&x, &y)) return FALSE;

				flag = FALSE;

				for (dir = 0; dir < 8; dir++)
				{
					int dy = y + ddy_ddd[dir];
					int dx = x + ddx_ddd[dir];
					if (dir == 5) continue;
					if(cave[dy][dx].m_idx) flag = TRUE;
				}

				if (!cave_empty_bold(y, x) || (cave[y][x].info & CAVE_ICKY) ||
					(distance(y, x, p_ptr->y, p_ptr->x) > plev + 2))
				{
					msg_print(_("そこには移動できない。", "Can not teleport to there."));
					continue;
				}
				break;
			}

			if (flag && randint0(plev * plev / 2))
			{
				teleport_player_to(y, x, 0L);
			}
			else
			{
				msg_print(_("おっと！", "Oops!"));
				teleport_player(30, 0L);
			}

			add = FALSE;
		}
		break;

	case 30:
		if (name) return _("反魔法結界", "Anti magic barrier");
		if (desc) return _("視界内のモンスターの魔法を阻害するバリアを張る。", "Obstructs all magic spell of monsters in your sight.");
		power = plev * 3 / 2;
		if (info) return info_power(power);
		if (cast)
		{
			msg_print(_("魔法を防ぐ呪いをかけた。", "You feel anyone can not cast spells except you."));
		}
		break;

	case 31:
		if (name) return _("復讐の宣告", "Revenge sentence");
		if (desc) return _("数ターン後にそれまで受けたダメージに応じた威力の地獄の劫火の弾を放つ。", 
			"Fires  a ball of hell fire to try revenging after few turns.");
		power = HEX_REVENGE_POWER(p_ptr);
		if (info) return info_damage(0, 0, power);
		if (cast)
		{
			MAGIC_NUM2 r;
			int a = 3 - (p_ptr->pspeed - 100) / 10;
			r = 1 + randint1(2) + MAX(0, MIN(3, a));

			if (HEX_REVENGE_TURN(p_ptr) > 0)
			{
				msg_print(_("すでに復讐は宣告済みだ。", "You already pronounced your revenge."));
				return NULL;
			}

			HEX_REVENGE_TYPE(p_ptr) = 2;
			HEX_REVENGE_TURN(p_ptr) = r;
			msg_format(_("あなたは復讐を宣告した。あと %d ターン。", "You pronounce your revenge. %d turns left."), r);
			add = FALSE;
		}
		if (cont)
		{
			HEX_REVENGE_TURN(p_ptr)--;

			if (HEX_REVENGE_TURN(p_ptr) <= 0)
			{
				int dir;

				if (power)
				{
					command_dir = 0;

					do
					{
						msg_print(_("復讐の時だ！", "Time to revenge!"));
					}
					while (!get_aim_dir(&dir));

					fire_ball(GF_HELL_FIRE, dir, power, 1);

					if (p_ptr->wizard)
					{
						msg_format(_("%d点のダメージを返した。", "You return %d damages."), power);
					}
				}
				else
				{
					msg_print(_("復讐する気が失せた。", "You are not a mood to revenge."));
				}
				HEX_REVENGE_POWER(p_ptr) = 0;
			}
		}
		break;
	}

	/* start casting */
	if ((cast) && (add))
	{
		/* add spell */
		CASTING_HEX_FLAGS(p_ptr) |= 1L << (spell);
		CASTING_HEX_NUM(p_ptr)++;

		if (p_ptr->action != ACTION_SPELL) set_action(ACTION_SPELL);
	}

	/* Redraw status */
	if (!info)
	{
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
		p_ptr->redraw |= (PR_EXTRA | PR_HP | PR_MANA);
	}

	return "";
}


/*!
 * @brief 魔法処理のメインルーチン
 * @param realm 魔法領域のID
 * @param spell 各領域の魔法ID
 * @param mode 求める処理
 * @return 各領域魔法に各種テキストを求めた場合は文字列参照ポインタ、そうでない場合はNULLポインタを返す。
 */
cptr do_spell(REALM_IDX realm, SPELL_IDX spell, BIT_FLAGS mode)
{
	switch (realm)
	{
	case REALM_LIFE:     return do_life_spell(spell, mode);
	case REALM_SORCERY:  return do_sorcery_spell(spell, mode);
	case REALM_NATURE:   return do_nature_spell(spell, mode);
	case REALM_CHAOS:    return do_chaos_spell(spell, mode);
	case REALM_DEATH:    return do_death_spell(spell, mode);
	case REALM_TRUMP:    return do_trump_spell(spell, mode);
	case REALM_ARCANE:   return do_arcane_spell(spell, mode);
	case REALM_CRAFT:    return do_craft_spell(spell, mode);
	case REALM_DAEMON:   return do_daemon_spell(spell, mode);
	case REALM_CRUSADE:  return do_crusade_spell(spell, mode);
	case REALM_MUSIC:    return do_music_spell(spell, mode);
	case REALM_HISSATSU: return do_hissatsu_spell(spell, mode);
	case REALM_HEX:      return do_hex_spell(spell, mode);
	}

	return NULL;
}
