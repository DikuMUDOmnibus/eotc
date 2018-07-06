
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

/* energy spells */
void spell_etheral_fist( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 12 );
    if ( saves_spell( level, victim, DAM_ENERGY ) )
	dam /= 2;
    act("A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!"
		,ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE);
    return;
}

void spell_spectral_furor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 8 );
    if ( saves_spell( level, victim, DAM_ENERGY ) )
	dam /= 2;
    act("The fabric of the cosmos strains in fury about $N!",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE);
    return;
}

void spell_disruption( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 9 );
    if ( saves_spell( level, victim, DAM_ENERGY ) )
	dam /= 2;
    act("A weird energy encompasses $N, causing you to question $S continued existence.",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE);
    return;
}


void spell_sonic_resonance( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 7 );
    if ( saves_spell( level, victim, DAM_ENERGY ) )
	dam /= 2;
    act("A cylinder of kinetic energy enshrouds $N causing $S to resonate.",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE);
    WAIT_STATE( victim, skill_table[sn].beats );
    return;
}
/* mental */
void spell_mind_wrack( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 7 );
    if ( saves_spell( level, victim, DAM_MENTAL ) )
	dam /= 2;
    act("$n stares intently at $N, causing $N to seem very lethargic.",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_MENTAL,TRUE);
    return;
}

void spell_mind_wrench( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 9 );
    if ( saves_spell( level, victim, DAM_MENTAL ) )
	dam /= 2;
    act("$n stares intently at $N, causing $N to seem very hyperactive.",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_MENTAL,TRUE);
    return;
}
/* acid */
void spell_sulfurus_spray( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 7 );
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    act("A stinking spray of sulfurous liquid rains down on $N." ,
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}

void spell_caustic_font( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 9 );
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    act("A fountain of caustic liquid forms below $N.  The smell of $S degenerating tissues is revolting! ",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}


/*  Electrical  */

void spell_galvanic_whip( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 7 );
    if ( saves_spell( level, victim, DAM_LIGHTNING ) )
	dam /= 2;
    act("$n conjures a whip of ionized particles, which lashes ferociously at $N.",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_LIGHTNING,TRUE);
    return;
}


void spell_magnetic_trust( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 8 );
    if ( saves_spell( level, victim, DAM_LIGHTNING ) )
	dam /= 2;
    act("An unseen energy moves nearby, causing your hair to stand on end!",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_LIGHTNING,TRUE);
    return;
}

void spell_quantum_spike( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 9 );
    if ( saves_spell( level, victim, DAM_LIGHTNING ) )
	dam /= 2;
    act("$N seems to dissolve into tiny unconnected particles, then is painfully reassembled.",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_LIGHTNING,TRUE);
    return;
}

/* negative */
void spell_hand_of_undead( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);
	return;
    }

    if ( (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
	{
	 send_to_char("Your victim is unaffected by hand of undead.\n\r",ch);
	 return;
	}
    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
     dam = dice( level, 10 );
     victim->mana	/= 2;
     victim->move	/= 2;
     ch->hit		+= dam / 2;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    act("$N is grasped by an incomprehensible hand of undead!",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_NEGATIVE,TRUE);
    return;
}

void spell_precision( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED ( victim, AFF_PRECISION ) )
	{
	if ( victim == ch )
	    send_to_char("You are already affected.\n\r", ch);
	else
	    act("$N is already affected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level*4;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = victim->hitroll;
    af.bitvector = AFF_PRECISION;
    affect_to_char( victim, &af );
    send_to_char("You feel more lethal!\n\r", victim);
    if ( ch != victim )
	act("$N looks more lethal.",ch,NULL,victim,TO_CHAR);
    return;
}


/* holy */
void spell_wrath_of_god( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range(4500,6500);

    send_to_char("You feel your life slipping away!\n\r",victim);
    act("$N is grasped by an incomprehensible hand of undead!",
		ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_HOLY,TRUE);
    return;
}

// Templar
void spell_knightshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *item;
    char buf     [MAX_STRING_LENGTH];
    int hitroll = 0;
    int damroll = 0;
    int armor   = 0;
            
    item = create_object( get_obj_index( OBJ_VNUM_KNIGHTSHIELD ), 1 );
   
     
    free_string(item->name);
    sprintf(buf,"Templar Shield of %s",god_table[ch->god].name);
    item->name = str_dup(buf);
    free_string(item->short_descr);
    sprintf(buf,"Templar Shield of %s",god_table[ch->god].name);
    item->short_descr = str_dup(buf);
    free_string(item->description);
    sprintf(buf,"Templar Shield of %s",god_table[ch->god].name);
    item->description = str_dup(buf);

//    SET_BIT( item->item_type, ITEM_ARMOR );
    item->item_type = ITEM_ARMOR;
    SET_BIT( item->wear_flags, ITEM_TAKE );
    SET_BIT( item->wear_flags, ITEM_WEAR_SHIELD );
    SET_BIT( item->extra_flags, ITEM_MAGIC);
    SET_BIT( item->extra_flags, ITEM_GLOW);       
    item->value[0] = 200;  // pierce
    item->value[1] = 200;  // bash 
    item->value[2] = 200;  // slash
    item->value[3] = 200;  // magic
    item->level = ch->level;
    item->condition = 100;
    item->weight = 10;
    if (ch->level <= 50) hitroll = 50; 
    else hitroll = ch->level;
    if (ch->level <= 50) damroll = 50; 
    else damroll = ch->level;
    armor = number_range(-200,-300);
    add_affect( item, APPLY_HITROLL, hitroll );
    add_affect( item, APPLY_DAMROLL, damroll );

    obj_to_char(item,ch);
    act("$p fades into existance in your hand.", ch, item, NULL, TO_CHAR);
    act("$p fades into existance in $n's hand.", ch, item, NULL, TO_ROOM);
    return;
}
