#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include <unistd.h>



// CH_CMD ( do_knee )
void do_knee (CHAR_DATA * ch, char *argument)
{
CHAR_DATA *victim;
int dam;

if ( !IS_NPC(ch)
&& ch->level < skill_table[gsn_knee].skill_level[ch->class]
&& ch->level2 < skill_table[gsn_knee].skill_level[ch->class2]
&& ch->level3 < skill_table[gsn_knee].skill_level[ch->class3] )
{
send_to_char(
"You better leave the martial arts to fighters.\n\r", ch );
return;
}

if ( ( victim = ch->fighting ) == NULL )
{
send_to_char( "You aren't fighting anyone.\n\r", ch );
return;
}


if ( !can_see ( ch, victim ) )
{
send_to_char ( "You can't seem to find their head.\n\r", ch );
return;
}

WAIT_STATE( ch, skill_table[gsn_knee].beats );
if ( get_skill(ch,gsn_knee) > number_percent())
{
check_improve(ch,gsn_knee,TRUE,1);
act( "{i$n grabs your head and slams their {Rk{rn{Re{re{i into your face.{x", ch, NULL, victim, TO_VICT);
act( "{hYou grab $N {hand slam your {Rk{rn{Re{re{h into their face.{x", ch, NULL, victim, TO_CHAR);
act( "{k$n grabs $N {kand slams $s {Rk{rn{Re{re{k into their face.{x", ch, NULL, victim, TO_NOTVICT);

dam = number_range ( (ch->level * 90 ), ( 5 ));
damage( ch, victim, dam, gsn_knee,DAM_BASH,TRUE);
WAIT_STATE ( ch, skill_table[gsn_knee].beats );
}
else
{
check_improve(ch,gsn_knee,FALSE,1);
act( "{i$n tries to {Rk{rn{Re{re{i you but misses.{x", ch, NULL, victim, TO_VICT);
act( "{hYou try to {Rk{rn{Re{re{h $N but miss.{x", ch, NULL, victim, TO_CHAR);
act( "{k$n tries to {Rk{rn{Re{re{k $N but misses.{x", ch, NULL, victim, TO_NOTVICT);
damage( ch, victim, 0, gsn_knee,DAM_BASH,TRUE);
}
return;
}

void do_shand (CHAR_DATA * ch, char *argument)
{
//    OBJ_DATA *obj;
    CHAR_DATA *victim;
    int dam;
    int chance;


    chance = number_percent();

    if ( !IS_NPC ( ch ) &&
         ch->level < skill_table[gsn_shand].skill_level[ch->class]
	 && ch->level2 < skill_table[gsn_shand].skill_level[ch->class2]
	 && ch->level3 < skill_table[gsn_shand].skill_level[ch->class3] )
    {
        send_to_char
            ( "You better leave that skill to Liches.\n\r", ch );
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char
            ( "You must be fighting to do this.\n\r",
              ch );
        return;
    }
    if ( !can_see ( ch, victim ) )
    {
        send_to_char ( "You cant find them.\n\r", ch );
        return;
    }


    WAIT_STATE ( ch, skill_table[gsn_shand].beats / 2 );

    if ( get_skill ( ch, gsn_shand ) < number_percent (  ) )
    {
        printf_to_char ( ch, "You miss them.\n\r" );
        check_improve ( ch, gsn_shand, FALSE, 1 );
        return;
    }

    dam = ch->level * 40;

    damage ( ch, victim, number_range ( dam, ch->level * 45 ), gsn_shand, DAM_NEGATIVE, TRUE);
    check_improve ( ch, gsn_shand, TRUE, 3 );

    dam = ch->level * 40;

    damage ( ch, victim, number_range ( dam, ch->level * 45 ), gsn_shand, DAM_NEGATIVE, TRUE);

    dam = ch->level * 40;

    damage ( ch, victim, number_range ( dam, ch->level * 35 ), gsn_shand, DAM_NEGATIVE, TRUE);

    if (chance < 50 )
        return;

    dam = ch->level * 45;

    damage ( ch, victim, number_range ( dam, ch->level * 28 ), gsn_shand, DAM_NEGATIVE, TRUE);
    check_improve ( ch, gsn_shand, TRUE, 2 );

    if (chance < 30)
        return;

    dam = ch->level * 35;

    damage ( ch, victim, number_range ( dam, ch->level * 22 ), gsn_shand, DAM_NEGATIVE, TRUE);

    if (chance < 25)
        return;

    dam = ch->level * 30;

    damage ( ch, victim, number_range ( dam, ch->level * 16 ), gsn_shand, DAM_NEGATIVE, TRUE);

    if (chance < 20)
        return;

    dam = ch->level * 25;

    damage ( ch, victim, number_range ( dam, ch->level * 16 ), gsn_shand, DAM_NEGATIVE, TRUE);

    return;
}


// CH_CMD ( do_rampage )
void do_rampage (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    int dam;
    int chance;


    chance = number_percent();

    if ( !IS_NPC ( ch ) &&
         ch->level < skill_table[gsn_rampage].skill_level[ch->class]
	 && ch->level2 < skill_table[gsn_rampage].skill_level[ch->class2]
	 && ch->level3 < skill_table[gsn_rampage].skill_level[ch->class3] )
    {
        send_to_char
            ( "You better leave that skill to fighters.\n\r", ch );
        return;
    }

    if ( ( obj = get_eq_char ( ch, WEAR_WIELD ) ) == NULL )
    {
        send_to_char
            ( "You need to wield a primary weapon to go on a rampage.\n\r",
              ch );
        return;
    }
    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char
            ( "How do you expect to lose your temper when your not in battle.\n\r",
              ch );
        return;
    }
    if ( !can_see ( ch, victim ) )
    {
        send_to_char ( "You stumble blindly into a wall.\n\r", ch );
        return;
    }


    WAIT_STATE ( ch, skill_table[gsn_rampage].beats / 2 );

    if ( get_skill ( ch, gsn_rampage ) < number_percent (  ) )
    {
        printf_to_char ( ch,
                         "You prepare for a rampage but your opponent is too quick!\n\r" );
        check_improve ( ch, gsn_rampage, FALSE, 1 );
        return;
    }

    dam = ch->level * 30;

    damage ( ch, victim, number_range ( dam, ch->level * 45 ), gsn_rampage, DAM_BASH, TRUE);
    check_improve ( ch, gsn_rampage, TRUE, 3 );

    dam = ch->level * 30;

    damage ( ch, victim, number_range ( dam, ch->level * 45 ), gsn_rampage,
             DAM_BASH, TRUE);

    dam = ch->level * 30;

    damage ( ch, victim, number_range ( dam, ch->level * 35 ), gsn_rampage,
             DAM_BASH, TRUE);

    if (chance < 50 )
        return;

    dam = ch->level * 35;

    damage ( ch, victim, number_range ( dam, ch->level * 28 ), gsn_rampage,
             DAM_BASH, TRUE);
    check_improve ( ch, gsn_rampage, TRUE, 2 );

    if (chance < 30)
        return;

    dam = ch->level * 25;

    damage ( ch, victim, number_range ( dam, ch->level * 22 ), gsn_rampage,
             DAM_BASH, TRUE);
/*
    if ( chance ( 35 ) )
    {
        printf_to_char ( ch,
                         "{YYour opponent is stunned by your {Rf{ru{Rr{ri{Ro{ru{Rs{Y{x rampage!{x\n\r" ); */
/*    if ( chance ( 35 ) )
    {
     if ( ch->level > skill_table[gsn_stun].skill_level[ch->class] )
     {
        act("You {Ystun{x $N with your rampage.",
            ch,NULL,victim,TO_CHAR);
        act("You are {Ystunned{x by $n's rampage.",
            ch,NULL,victim,TO_VICT);
        act("$N is {Ystunned{x.",
            ch,NULL,victim,TO_NOTVICT);
        victim->stunned = 2;
        check_improve ( ch, gsn_stun, TRUE, 1 ); 
     }
    }
*/
    if (chance < 25)
        return;

    dam = ch->level * 20;

    damage ( ch, victim, number_range ( dam, ch->level * 16 ), gsn_rampage,
             DAM_BASH, TRUE);

    if (chance < 20)
        return;

    dam = ch->level * 15;

    damage ( ch, victim, number_range ( dam, ch->level * 16 ), gsn_rampage,
             DAM_BASH, TRUE);

    return;
}


void do_relinquish (CHAR_DATA * ch, char *argument)
{

  char arg[MSL];
  CHAR_DATA *victim;
  one_argument (argument, arg);

  if (IS_NPC (ch))
    return;

    if ( !IS_NPC(ch)
    && ch->level < skill_table[gsn_relinquish].skill_level[ch->class]
    && ch->level < skill_table[gsn_relinquish].skill_level[ch->class2]
    && ch->level < skill_table[gsn_relinquish].skill_level[ch->class3] )

    {
        send_to_char("You don't know how to relinquish.\n\r", ch );
        return;
    }



  if (arg[0] == '\0')
    {
      stc ("Desanc whom?\n\r", ch);
      return;
    }
  if (ch->fighting != NULL)
    {
      stc ("You can only use this power to start combat.\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      stc ("They are not here.\n\r", ch);
      return;
    }
  if(victim == ch) 
    {
     stc("not on yourself",ch);
     return;
    }

  if (is_safe (ch, victim))
    return;

  if (number_range (1,3) == 1)
    {
      stc ("You failed to Desanct!\n\r", ch);
      WAIT_STATE (ch, 6);
      return;
    }

  act
    ("$n cackles with glee as he destroys the hold of god from your body.\n\r",
     ch, NULL, victim, TO_VICT);
  act
    ("$n cackles with glee as he destroys the hold of god from $N's body.\n\r",
     ch, NULL, victim, TO_ROOM);
  act
    ("You cackles with glee as you destroy the hold of god from $N's body.\n\r",
     ch, NULL, victim, TO_CHAR);
//  sn = skill_lookup ("desanct");
//  level = 25;
//  (*skill_table[sn].spell_fun) (sn, level, ch, victim);

	REMOVE_BIT (victim->affected_by, AFF_SANCTUARY);

  set_fighting (ch, victim);
  set_fighting (victim, ch);
  if (!IS_NPC (victim))

  WAIT_STATE (ch, 6);

}


/*  
 * critical strike
*/
int critical_strike( CHAR_DATA *ch, CHAR_DATA *victim,   int dam )
{
  int diceroll;
  AFFECT_DATA baf;

	
  if ( get_eq_char(ch,WEAR_WIELD) != NULL 
	&& get_eq_char(ch,WEAR_SECONDARY ) != NULL
	&& number_percent() > ( (ch->hit * 100) / ch->max_hit ) ) 
      return 0;

  if ( !IS_NPC(ch) && /* IS_SET( ch->off_flags, OFF_CRIT ) ) ||*/ 
	 get_skill( ch, gsn_critical ) > 0 ) 
  {  
    diceroll = number_range( 0, 100 );
    if( victim -> level > ch -> level )
      diceroll += ( victim->level - ch->level ) * 2;
    if( victim -> level < ch -> level )
      diceroll -= ( ch->level - victim->level );
 
    if ( (!IS_NPC(ch)) && (diceroll <= (get_skill( ch, gsn_critical )/2)) )  
    {  
      check_improve( ch, gsn_critical, TRUE, 2 );
      dam += dam * diceroll/200;
    }  

    if ( (IS_NPC(ch) && (diceroll <= (ch->level / 13))) || 
         (!IS_NPC(ch) && (diceroll <= (get_skill(ch, gsn_critical)/13)) ) )
    {  
      diceroll = number_percent( );
      if( diceroll < 75 )
      {  
        act( "$n takes you down with a weird judo move!", 
	     ch, NULL, victim, TO_VICT);
	act( "You take $N down with a weird judo move!", 
	     ch, NULL, victim, TO_CHAR);
        if (!IS_NPC(ch))
          check_improve( ch, gsn_critical, TRUE, 3 );
	WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
	dam += (dam * number_range( 2, 5 )) / 5;			
	return(dam);
      }   
      else if( diceroll > 75 && diceroll < 95 )
      {   
        act( "You are blinded by $n's attack!", ch, NULL, victim, 
			TO_VICT);
        act( "You blind $N with your attack!", ch, NULL, victim, 
			TO_CHAR);
	if (!IS_NPC(ch)) 
	  check_improve( ch, gsn_critical, TRUE, 4 );
	if (!IS_AFFECTED(victim,AFF_BLIND)) 
	{
          baf.type = gsn_dirt;
	  baf.level = ch->level; 
	  baf.location = APPLY_HITROLL; 
	  baf.modifier = -4;
	  baf.duration = number_range(1,5); 
	  baf.bitvector = AFF_BLIND;
	  affect_to_char( victim, &baf );
        }  
	dam += dam * number_range( 1, 2 );			
	return(dam);
      }  
      else if( diceroll > 95 ) 
      {   
        act( "$n cuts out your heart! OUCH!!",  
             ch, NULL, victim, TO_VICT); 
        act( "You cut out $N's heart!  I bet that hurt!",  
             ch, NULL, victim, TO_CHAR);
	if (!IS_NPC(ch)) 
	  check_improve( ch, gsn_critical, TRUE, 5 );
	dam += dam * number_range( 2, 5 );			
	return(dam);
      }  
    }   
  }   

  return dam;
}  

void do_jugular_slice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );



    if ( !IS_NPC(ch)
    && ch->level < skill_table[gsn_jugular_slice].skill_level[ch->class]
    && ch->level2 < skill_table[gsn_jugular_slice].skill_level[ch->class2]
    && ch->level3 < skill_table[gsn_jugular_slice].skill_level[ch->class3] )
      {
        send_to_char("You don't know how to jugular slice.\n\r",ch);
        return;
      }




    if ( arg[0] == '\0' )
    {
	send_to_char( "Jugular Slice who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if ( (IS_NPC(ch)) && (!(IS_NPC(victim))) )
	return;
	

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to jugular slice.\n\r", ch );
	return;
    }


    if ( victim->fighting != NULL )
    {
	send_to_char( "You can't jugular slice a fighting person.\n\r", ch );
	return;
    }


    WAIT_STATE( ch, skill_table[gsn_jugular_slice].beats );

    if ( victim->hit < (0.7 * victim->max_hit) &&
	 (IS_AWAKE(victim) ) )
    {
	act( "$N is hurt and suspicious ... you couldn't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }


    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < get_skill(ch,gsn_jugular_slice) )
    {
	check_improve(ch,gsn_jugular_slice,TRUE,1);
        multi_hit( ch, victim, gsn_jugular_slice );
    }
    else
    {
	check_improve(ch,gsn_jugular_slice,FALSE,1);
	damage( ch, victim, 0, gsn_jugular_slice,DAM_NONE, TRUE );
    }
    return;
}

void do_assassinate( CHAR_DATA 
*ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    one_argument( argument, arg );

	if (ch->master != NULL && IS_NPC(ch))
	return;


    if ( !IS_NPC ( ch ) &&
         ch->level < skill_table[gsn_assassinate].skill_level[ch->class]
         && ch->level2 < skill_table[gsn_assassinate].skill_level[ch->class2]
         && ch->level3 < skill_table[gsn_assassinate].skill_level[ch->class3] )
      {
	send_to_char("You don't know how to assassinate.\n\r",ch);
	return;
      }
    
    if ( IS_AFFECTED( ch, AFF_CHARM ) )  
    {
	send_to_char( "You don't want to kill your beloved master.\n\r",ch);
	return;
    } 

    if ( arg[0] == '\0' )
    {
	send_to_char( "Assassinate whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is against your way.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( IS_IMMORTAL( victim ) && !IS_NPC(victim) )
    {
	send_to_char( "Your hands pass through.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
    {
	send_to_char( "You can't assassinate a fighting person.\n\r", ch );
	return;
    }
 
    if ( (get_eq_char( ch, WEAR_WIELD ) != NULL) ||
	 (get_eq_char( ch, WEAR_HOLD  ) != NULL) )  {
	send_to_char( 
	"You need both hands free to assassinate somebody.\n\r", ch );
	return;
    }
 
    if ( (victim->hit < victim->max_hit) && 
	 (can_see(victim, ch)) &&
	 (IS_AWAKE(victim) ) )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }
    
/*
    if (IS_SET(victim->imm_flags, IMM_WEAPON))
      {
	act("$N seems immune to your assassination attempt.", ch, NULL,
		 victim, TO_CHAR);
	act("$N seems immune to $n's assassination attempt.", ch, NULL,
		victim, TO_ROOM);
	return;
      }
*/
    WAIT_STATE( ch, skill_table[gsn_assassinate].beats );
    if ( IS_NPC(ch) ||
	!IS_AWAKE(victim) 
	||   number_percent( ) < get_skill(ch,gsn_assassinate))
      multi_hit(ch,victim,gsn_assassinate);
    else
      {
	check_improve(ch,gsn_assassinate,FALSE,1);
	damage( ch, victim, 0, gsn_assassinate,DAM_NONE, TRUE );
      }

    return;
  }


void do_unnerve(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if ( !IS_NPC ( ch ) &&
         ch->level < skill_table[gsn_unnerve].skill_level[ch->class]
         && ch->level2 < skill_table[gsn_unnerve].skill_level[ch->class2]
         && ch->level3 < skill_table[gsn_unnerve].skill_level[ch->class3] )
    {
      send_to_char("Unnerve is unavailable to you.\n\r",ch);
      return;
    }

    if (argument[0] == '\0')
    {
	stc("Who do you wish to unnerve?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 7);
    do_say(ch,"{RXeus Dominus Mortai!{x");
    do_stance(victim, "");
}
