/* Xrakisis's Lazy mans Cotn Quicker Quickmud */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "disease.h"


void do_diseaseself(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char      arg1 [MAX_INPUT_LENGTH];
  argument = one_argument( argument, arg1 );


  if (IS_NPC(ch))return;

  if (!IS_NPC(ch) && ch->disease != -1)
  {   
    send_to_char("You already have a disease.\n\r",ch);
    return;
  }

if ( arg1[0] == '\0' )
{
stc("Type diseaseself <vampire or werewolf>\n\r",ch);
stc("{G************************************************{x\n\r",ch);
stc("{Y--{CVampire       {Y--{CWerewolf\n\r",ch);
stc("{G************************************************{x\n\r",ch);
return;
}

if (!str_cmp(arg1,"werewolf"))
{
ch->disease = DISEASE_WEREWOLF;
SET_BIT(ch->vuln_flags, VULN_SILVER);
send_to_char( "You have Become a {GWerewolf{x.\n\r", ch);
sprintf(buf,"%s Has become a Werewolf.", ch->name);
do_info(ch,buf);
return;
}
if (!str_cmp(arg1,"vampire"))
{
ch->disease = DISEASE_VAMPIRE;
SET_BIT(ch->vuln_flags, VULN_HOLY);
send_to_char( "You have Become a {GVampire{x.\n\r", ch);
sprintf(buf,"%s Has become a Vampire.", ch->name);
do_info(ch,buf);
return;
}
  else do_diseaseself(ch,"");   
  return;
}


