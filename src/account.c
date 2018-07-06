/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
5B *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

/*
 * Most account stuff wrote by Jobo from Dystopia MUD.
 * Changed and ported by Daurven
 */

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

ACCOUNT_DATA  *  account_free     = NULL;

ACCOUNT_DATA *load_account(char *name)
{
    ACCOUNT_DATA *account = NULL;
    FILE *fp;
    char pfile[256];
    char *word;
    bool done = FALSE, found;

    /* open the account so we can read from it */
    sprintf(pfile, "../accounts/%s/account.dat", capitalize(name));
    if ((fp = fopen(pfile, "r")) == NULL)
        return NULL;

    /* create new account data */
    account = alloc_account();

    /* load data */
    word = fread_word(fp);
    while (!done)
    {
        found = FALSE;
        switch (word[0])
        {
           case 'D':
	            IREAD( "Denied",       account->denied        );
            break;
            case 'E':
                if (!str_cmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
            break;
            case 'L':
	            IREAD( "Level",        account->level         );
	        break;
            case 'O':
                SREAD( "Owner",        account->owner         );
            break;
            case 'P':
                SREAD( "Password",     account->password      );
		if(!strcasecmp(word, "Player" ) )
		{	ACNT_PLAYER *pPlr = calloc(1, sizeof(*pPlr));
			pPlr->name = str_dup(fread_word(fp));
			pPlr->class = fread_number(fp);
			pPlr->race = fread_number(fp);
			pPlr->played = fread_number(fp);
			if(!account->first_player)
				account->first_player = pPlr;
				
			if(account->last_player)
				account->last_player->next = pPlr;
			account->last_player = pPlr;
			found = TRUE;
			break;
		}
			

            break;
        }

        if (!found)
        {
            char buf[MAX_STRING_LENGTH];

            sprintf(buf, "Load_account: unexpected '%s' in /%s/account.dat.", word, name);
            bug(buf, 0);
            close_account(account);
            return NULL;
        }

        /* read one more */
        if (!done) word = fread_word(fp);
    }
    fclose(fp);
    return account;
}

void save_account(ACCOUNT_DATA *account)
{
    FILE *fp;
    char pfile[256];

    /* open the account file so we can write to it */
    sprintf(pfile, "../accounts/%s/account.dat", account->owner);
    if ((fp = fopen(pfile, "w")) == NULL)
    {
        char buf[MAX_STRING_LENGTH];

        sprintf(buf, "Unable to write to %s's account file", account->owner);
       bug(buf, 0);
        return;
    }

    /* save the account data */
    fprintf(fp, "Denied            %ld\n",   account->denied       );
    fprintf(fp, "Level             %d\n",    account->level        );
    fprintf(fp, "Owner             %s~\n",   account->owner        );
    fprintf(fp, "Password          %s~\n",   account->password     );

    {	ACNT_PLAYER *pPlr;

	for(pPlr = account->first_player ; pPlr ; pPlr = pPlr->next )
		fprintf(fp, "Player %s %d %d %d\n", pPlr->name, pPlr->class, pPlr->race, pPlr->played);
    }

    /* terminate the file */
    fprintf(fp, "EOF\n");
    fclose(fp);
}

ACCOUNT_DATA *alloc_account()
{
    static ACCOUNT_DATA clear_account;
    ACCOUNT_DATA *account;

    if (account_free)
    {
        account      = account_free;
        account_free = account_free->next;
    }
    else
    {
        if ((account = alloc_perm(sizeof(*account))) == NULL)
        {
           bug("Alloc_account: Cannot allocate memory.", 0);
            abort();
        }
    }
    *account               =  clear_account;
    account->owner         =  str_dup("");
    account->password      =  str_dup("");
    account->new_password  =  str_dup("");
    account->players       =  str_dup("");
    account->level         =  PLAYER_ACCOUNT;
    account->denied        =  (time_t) 0;
    account->first_player = 0;
    account->last_player = 0;	
    return account;
}

void close_account(ACCOUNT_DATA *account)
{	ACNT_PLAYER *pPlr, *p_next;
    /* free the memory */
    free_string(account->password);
    free_string(account->owner);
    free_string(account->new_password);
	for(pPlr = account->first_player ; pPlr ; pPlr = p_next )
	{	p_next = pPlr->next;
		free_string(pPlr->name);
		free(pPlr);
	}
    /* attach to free list */
    account->next = account_free;
    account_free = account;
}
/*

                     _                    _
                  ,/                        \,
        _________{(                          })_________
       /.-------./\\                        //\.-------.\
      //@@@@@@@//@@\\  )                (  //@@\\@@@@@@@\\
     //@@@@@@@//@@@@>>/                  \<<@@@@\\@@@@@@@\\
    //O@O@O@O//@O@O//                      \\O@O@\\O@O@O@O\\
  //OOOOOOOO//OOOO||          \  /          ||OOOO\\OOOOOOOO\\
 //O%O%O%O%//O%O%O%\\         ))((         //%O%O%O\\%O%O%O%O\\
||%%%%%%%%//'  `%%%%\\       //  \\       //%%%%'   `\\%%%%%%%||
((%%%%%%%((      %%%%%\\    ((    ))    //%%%%%       ))%%%%%%))
 \:::' `::\\      `:::::\\   \)~~(/    //:::::'      //::' `:::/
  )'     `;)'      (`  ` \\ `<@  @>' / / '  ')      `(;'     `(
          (               \`\ )^^( /  /               )
        _                  ) \\oo/   (
       (@)                  \  `'   /                      _
       |-|\__________________\__^__<________oOo__________ (@)
       | |                                  VVV          \|-|
       |-|                                                |-|
       |_|\_____________________________________________  | |
       (@)                 / ,/ \_____/ \\ ~\/~         `\|-|
        ~             ___//^~      \____/\\               (@)
                     <<<  \     __  <____/||               ~
                               <   \ <___/||
                                  || <___//
                                  \ \/__//
                                   ~----~


   ,           \  -.__...--.__ _   /                   /
  _|\_          .'               )              ,~~   /
_  ("}       - :                  '.        _  <=)  _/_
Y_.-@-._ _     \                   .' _    /I\.="==.{>
B--,  .-`~     (                   )       \I/-\T/-'
I  /==\         (_                /            /_\
I  |   \           (___,    _..__) _          // \\_
I  /____\       -'      '--'                 _I    /



                   _    _                ______________________
 _____________ __ _ \../ _   __  ____ -=(_)____________________)=- _
|             // \_=>OO<=_  / \\        /                   _/      |
| ______      /X\ \ \oo/\  / /\\       /~                   /       |
||      |     \\\\ \ `'\^\/ /\ \\     (~                    (       |
||       ==='  \\\\ \  /^/ /\ \ \\     \ ------------------~\       |
||      |       \XX\_\/^.\/\/\/\/\\     \                    \      |
||      |        _-~^^^^/)\v\__          \_     ========      \     |
||      |       /^^^^^^_// \__ \          \                    \    |
| ~~~~~~        \^^___/ \    ///           \_     ========     ~\   |
|               |^^/\/\ (//                 )                    )  |
|               |^|  ^ \ (                 /~                  _/   |
|                \^\_   ^ __/            _/____________________/    |
|________________ \^^-----_/ ________ -=(_)____________________)=- _|
                   ~----~~

o--====---------------------------------------------------------===--o
      ___    ___        __________________        ___    ___
  ____/ _ \__/ _ \_____ (------------------) _____/ _ \__/ _ \____
 (  _| / \/  \/ \ |_   ) \                / (   _| / \/  \/ \ |_  )
  \(  \|  )  (  |/  ) (___)  __________  (___) (  \|  )  (  |/  )/
   `   `  \`'/  '  (_________)        (_________)  `  \`'/  '   '
           ||                                          ||
           `'                                          `'



 Does not tell you how many Door = 0 files to modify in each file. Not detailed enough.
 Forgot to add rev_dir and its based on your door #'s. If your set up north, east, south,
 west, up, down, northeast, southeast, southwest, northwest then your rev_dir should look
 like this:
2, 3, 0, 1, 5, 4, 6, 7, 8, 9 if it is not and you create a room your exit names will be
messed up.
There is some other information not included.

*/
/* void show_options(DESCRIPTOR_DATA *dsock)
{
    char buf[MAX_STRING_LENGTH];
    extern char *help_menu_1;

    send_to_desc("\x1B[H\x1B[J",dsock);
    if (help_menu_1[0] == '.')
        send_to_desc (help_menu_1 + 1, dsock);
    else
        send_to_desc (help_menu_1, dsock);
    sprintf(buf, "\n\rWelcome %s, whats your choice today.", dsock->account->owner);
    send_to_desc(buf, dsock);
}
*/

void show_options(DESCRIPTOR_DATA *dsock)
{
  char buf[MAX_STRING_LENGTH];
  ACNT_PLAYER *pPlr;


  if (dsock->connected != CON_PLAYING)
  {
    sprintf(buf, "\n\n\rWelcome %s. What's your game today ?\n\n\r", dsock->account->owner);
    write_to_buffer(dsock, buf, 0);
  }

	if(dsock->account->first_player)
	{	int i = 1;
		sprintf(buf, "[##] %-20s%-20s%-20s%-20s\r\n", "Name", "Class", "Race", "Hours");
		write_to_buffer(dsock, buf,0);

		for(pPlr = dsock->account->first_player ; pPlr ; pPlr = pPlr->next )
		{	sprintf(buf, "[%2d] %-20s%-20s%-20s%-20d\r\n",i++, pPlr->name, class_table[pPlr->class].name, race_table[pPlr->race].name, pPlr->played );
			write_to_buffer(dsock,buf,0);
		}
	}

  if (dsock->connected != CON_PLAYING)
  {
    sprintf(buf, "\r\n\r\n [C]  Create new player\n\r"
                 " [D]  Delete player\n\r"
                 " [P]  Change Password\n\r"
                 " [Q]  Quit\n\n\r");
    write_to_buffer(dsock, buf, 0);
    write_to_buffer(dsock, "What will it be? ", 0);
  }

}

char *get_option_login(  ACCOUNT_DATA *account, char *list, int option)
{
  int current = 1;
	ACNT_PLAYER *pPlr;

	for(pPlr = account->first_player ; pPlr ; pPlr = pPlr->next )
		if( option == current++ )
			return pPlr->name;
    return NULL;
}

/*
* check reconnect is called before this procedure,
 * so we don't have to worry about this being the
 * same char as the one trying to logon.
 */
bool already_logged(char *name)
{
  CHAR_DATA *dMob;

  for (dMob = char_list; dMob; dMob = dMob->next)
 {
    if (IS_NPC(dMob)) continue;
    if (!str_cmp(dMob->pcdata->account, name))
      return TRUE;
  }
  return FALSE;
}

void account_update(ACCOUNT_DATA *account, CHAR_DATA *dMob)
{	ACNT_PLAYER *pPlr;
	bool found = FALSE;
	char buf[MSL];
	for(pPlr = account->first_player ; pPlr ; pPlr = pPlr->next )
	{	if(!str_cmp(pPlr->name, dMob->name ) )
		{	dMob->played = (dMob->played + (int) (current_time - dMob->logon)) /3600;
			found = TRUE;
		}
	}
	if(!found)
 	{	sprintf(buf, "Account_update: %s not in %s's playerlist", dMob->name, account->owner);
		bug(buf, 0);
  	}
	save_account(account);
}

void account_new_player(ACCOUNT_DATA *account, CHAR_DATA *dMob)
{       ACNT_PLAYER *nPlr;

        nPlr = calloc(1, sizeof(*nPlr) );
	if(!account->first_player )
		account->first_player = nPlr;
        if(account->last_player )
                account->last_player->next = nPlr;
        account->last_player = nPlr;

        nPlr->name = str_dup(dMob->name);
        nPlr->race = dMob->race;
        nPlr->played = dMob->played / 3600;
	nPlr->class = dMob->class;

        save_account(account);
}

void create_new_account(ACCOUNT_DATA *account)
{
  char buf[MAX_STRING_LENGTH];

  /* create the directory */
  sprintf(buf, "mkdir ../accounts/%s", account->owner);
  system(buf);

  /* save the account */
  save_account(account);
}

void account_del_player(ACCOUNT_DATA *pAcnt, CHAR_DATA *dMob )
{	ACNT_PLAYER *p, *prev = 0;

	for( p = pAcnt->first_player ; p ; p = p->next )
	{	if(!strcasecmp(p->name, dMob->name ) )
		{	if(prev)
			    prev->next = p->next;
			break;
		}
		prev = p;
	}

	if(p && pAcnt->first_player == p)
		pAcnt->first_player = p->next;
	if(p && pAcnt->last_player == p)
		pAcnt->last_player = prev;

	free(p);
}

