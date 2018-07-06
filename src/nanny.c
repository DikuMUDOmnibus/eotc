/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

/****************************************************************************
 *   This file is just the stock nanny() function ripped from comm.c. It    *
 *   seems to be a popular task for new mud coders, so what the heck?       *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>                /* OLC -- for close read write etc */
#include <stdarg.h>                /* printf_to_char */

#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"

#if    defined(macintosh) || defined(MSDOS)
extern const char echo_off_str[];
extern const char echo_on_str[];
extern const char go_ahead_str[];
#endif

#if    defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
extern const char echo_off_str[];
extern const char echo_on_str[];
extern const char go_ahead_str[];
#endif

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos args ((void));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

#if defined(unix)
void game_loop_unix args ((int control));
int init_socket args ((int port));
void init_descriptor args ((int control));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

/*
 *  * Other local functions (OS-independent).
 *   */
bool check_parse_name args ((char *name));
bool check_reconnect args ((DESCRIPTOR_DATA * d, char *name, bool fConn));
bool check_playing args ((DESCRIPTOR_DATA * d, char *name));

/*
 * Global variables.
 */
extern DESCRIPTOR_DATA *descriptor_list;    /* All open descriptors     */
extern DESCRIPTOR_DATA *d_next;        /* Next descriptor in loop  */
extern FILE *fpReserve;                /* Reserved file handle     */
extern bool god;                        /* All new chars are gods!  */
extern bool merc_down;                    /* Shutdown         */
extern bool wizlock;                    /* Game is wizlocked        */
extern bool newlock;                    /* Game is newlocked        */
extern char str_boot_time[MAX_INPUT_LENGTH];
extern time_t current_time;            /* time of this pulse */


/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny (DESCRIPTOR_DATA * d, char *argument)
{
    DESCRIPTOR_DATA *d_old, *d_next;
    ACCOUNT_DATA *account;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
//    char *pwdnew;
    char *pwdnew, *option;
    char *p;
    bool fOld;
    int iClass, race, i, weapon, god;
    extern int mud_telnetga, mud_ansicolor;

    /* Delete leading spaces UNLESS character is writing a note */
	if (d->connected != CON_NOTE_TEXT)
	{
		while ( isspace(*argument) )
			argument++;
	}
    ch = d->character;

    switch (d->connected)
    {

        default:
            bug ("Nanny: bad d->connected %d.", d->connected);
            close_socket (d);
            return;

        case CON_ANSI:
            if (argument[0] == '\0' || UPPER (argument[0]) == 'Y')
            {
                d->ansi = TRUE;
                send_to_desc ("{RAnsi enabled!{x\n\r", d);
//                d->connected = CON_GET_NAME;
                d->connected = CON_ACCOUNT_NAME;
                {
                    extern char *help_greeting;
                    if (help_greeting[0] == '.')
                        send_to_desc (help_greeting + 1, d);
                    else
                        send_to_desc (help_greeting, d);
                }
                break;
            }

            if (UPPER (argument[0]) == 'N')
            {
                d->ansi = FALSE;
                send_to_desc ("Ansi disabled!\n\r", d);
//                d->connected = CON_GET_NAME;
                d->connected = CON_ACCOUNT_NAME;

                {
                    extern char *help_greeting;
                    if (help_greeting[0] == '.')
                        send_to_desc (help_greeting + 1, d);
                    else
                        send_to_desc (help_greeting, d);
                }
                break;
            }
            else
            {
                send_to_desc ("Do you want ANSI? (Y/n) ", d);
                return;
            }


//        case CON_GET_NAME:
        case CON_ACCOUNT_NAME:
            if (argument[0] == '\0')
            {
                close_socket (d);
                return;
            }

//            argument[0] = UPPER (argument[0]);
//            if (!check_parse_name (argument))
            if ( check_ban( d->host, BAN_ALL) )

            {
//                send_to_desc ("Illegal name, try another.\n\rName: ", d);
                write_to_buffer(d, "Your site has been banned from this mud\n\r", 0);
                close_socket(d);
                return;
            }

//            fOld = load_char_obj (d, argument);
//            ch = d->character;
//
//            if (IS_SET (ch->act, PLR_DENY))
//            {
            argument = capitalize(argument);
            if (!check_parse_name(argument))
             {

//                sprintf (log_buf, "Denying access to %s@%s.", argument,
//                         d->host);
//                log_string (log_buf);
//                send_to_desc ("You are denied access.\n\r", d);
//                close_socket (d);
                write_to_buffer( d, "Illegal account name, try another.\n\r"
									"What is your account name? ", 0 );

                return;
            }

//            if (check_ban (d->host, BAN_PERMIT)
//                && !IS_SET (ch->act, PLR_PERMIT))
            if (strlen(argument) < 3
			|| (account = load_account(argument)) == NULL)

            {
//                send_to_desc ("Your site has been banned from this mud.\n\r",
//                              d);
	            /* wizlock */
	            if (wizlock)
	            {
	                write_to_buffer(d, "Currently wizlocked.\n\r", 0);

                close_socket (d);
                return;
            }

//            if (check_reconnect (d, argument, FALSE))
//            {
//                fOld = TRUE;
	            account = alloc_account();
	            account->owner = str_dup(argument);
	            d->account = account;
	            d->connected = CON_CONFIRM_ACCOUNT;
	            sprintf(buf, "Do you wish to create a new account"
			   				 " called '%s' (yes|no)? ", argument);
	            write_to_buffer(d, buf, 0);
	            return;

            }
//            else

            d->account = account;
            if (account->denied > current_time)

            {
//                if (wizlock && !IS_IMMORTAL (ch))
                sprintf(log_buf, "Denying access to %s@%s.",
				        account->owner, d->host);
                log_string(log_buf);
	            write_to_buffer(d, "You are denied access.\n\r", 0);
                close_socket(d);
	            return;
            }

            /* wizlock */
            if (wizlock && account->level < ADMIN_ACCOUNT)

                {
//                    send_to_desc ("The game is wizlocked.\n\r", d);
                write_to_buffer(d, "Currenly wizlocked.\n\r", 0);

                    close_socket (d);
                    return;
                }

            /* ask for password */
            write_to_buffer(d, "Please enter password: ", 0);
            write_to_buffer(d, echo_off_str, 0);
            d->connected = CON_GET_OLD_PASSWORD;
        break;

        case CON_NEW_PLAYER:
            argument[0] = UPPER (argument[0]);
            if (!check_parse_name (argument) || player_exists(argument) )
            {
                send_to_desc ("Illegal name, try another.\n\rName: ", d);
                return;
            }
	        fOld = load_char_obj( d, argument );
            ch = d->character;



            if (fOld)
            {
                /* Old player */
//                send_to_desc ("Password: ", d);
//                write_to_buffer (d, echo_off_str, 0);
//                d->connected = CON_GET_OLD_PASSWORD;

                if (check_playing (d, ch->name))
                    return;

                if (check_reconnect (d, ch->name, TRUE))
                 return;

                sprintf (log_buf, "Account %s, Player %s@%s has connected.",
			         d->account->owner, ch->name, d->host);
                log_string (log_buf);
                wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, get_trust (ch));
		connections_today++;

                if (IS_IMMORTAL (ch))
                {
                    do_function (ch, &do_help, "imotd");
                    d->connected = CON_READ_IMOTD;
                }
                else
                {
                    do_function (ch, &do_help, "motd");
                    d->connected = CON_READ_MOTD;
                }
            }
            else
            {
                /* New player */
                if (newlock)
                {
                    send_to_desc ("The game is newlocked.\n\r", d);
                    close_socket (d);
                    return;
                }

                if (check_ban (d->host, BAN_NEWBIES))
                {
                    send_to_desc
                        ("New players are not allowed from your site.\n\r",
                         0);
                    close_socket (d);
                    return;
                }

                sprintf (buf, "Did I get that right, %s (Y/N)? ", argument);
                send_to_desc (buf, d);
//                d->connected = CON_CONFIRM_NEW_NAME;

// This is where it crashes
      /* save this new player */
      account_new_player(d->account, d->character);
      save_char_obj(d->character);


                load_char_obj(d, argument);
                d->connected = CON_CONFIRM_PLAYER;

                return;
            }
            break;

        case CON_GET_OLD_PASSWORD:
#if defined(unix)
            write_to_buffer (d, "\n\r", 2);
#endif

//            if (strcmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd))
            if ( str_cmp( crypt( argument, d->account->password ),
			    d->account->password ) )

            {
                send_to_desc ("Wrong password.\n\r", d);
                close_socket (d);
                return;
            }

            write_to_buffer (d, echo_on_str, 0);
            show_options(d);
            d->connected = CON_PICK_PLAYER;
        break;


//            if (check_playing (d, ch->name))
//                return;

//            if (check_reconnect (d, ch->name, TRUE))
//                return;

//            sprintf (log_buf, "%s@%s has connected.", ch->name, d->host);
//            log_string (log_buf);
//            wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, get_trust (ch));

//            if (ch->desc->ansi)
//                SET_BIT (ch->act, PLR_COLOUR);
//            else
//                REMOVE_BIT (ch->act, PLR_COLOUR);

//            if (IS_IMMORTAL (ch))
//            {
//                do_function (ch, &do_help, "imotd");
//                d->connected = CON_READ_IMOTD;
//            }
//            else
        case CON_CONFIRM_ACCOUNT:
            switch ( *argument )
            {
//                do_function (ch, &do_help, "motd");
//                d->connected = CON_READ_MOTD;
                case 'y': case 'Y':
                    sprintf(buf, "New account created.\n\r"
				             "Give me a password for %s: %s",
                    d->account->owner, echo_off_str );
                    write_to_buffer(d, buf, 0);
	                d->connected = CON_GET_NEW_PASSWORD;
	            break;
	            case 'n': case 'N':
	                write_to_buffer(d, "Ok, what IS it, then? ", 0);
	                close_account(d->account);
	                d->account = NULL;
	                d->connected = CON_ACCOUNT_NAME;
	            break;
	            default:
                    write_to_buffer(d, "Please type Yes or No? ", 0);
	            break;
            }
            break;

/* RT code for breaking link */

        case CON_BREAK_CONNECT:
            switch (*argument)
            {
                case 'y':
                case 'Y':
                    for (d_old = descriptor_list; d_old != NULL;
                         d_old = d_next)
                    {
                        d_next = d_old->next;
                        if (d_old == d || d_old->character == NULL)
                            continue;

                        if (str_cmp (ch->name, d_old->original ?
                                     d_old->original->name : d_old->
                                     character->name))
                            continue;

                        close_socket (d_old);
                    }
                    if (check_reconnect (d, ch->name, TRUE))
                        return;
                    send_to_desc ("Reconnect attempt failed.\n\rName: ", d);
                    if (d->character != NULL)
                    {
                        free_char (d->character);
                        d->character = NULL;
                    }
//                    d->connected = CON_GET_NAME;
                    d->connected = CON_ACCOUNT_NAME;
                    break;

                case 'n':
                case 'N':
                    send_to_desc ("Name: ", d);
                    if (d->character != NULL)
                    {
                        free_char (d->character);
                        d->character = NULL;
                    }
//                    d->connected = CON_GET_NAME;
                    d->connected = CON_ACCOUNT_NAME;

                    break;

                default:
                    send_to_desc ("Please type Y or N? ", d);
                    break;
            }
            break;

//        case CON_CONFIRM_NEW_NAME:
        case CON_CONFIRM_PLAYER:
            switch (*argument)
            {
                case 'y':
//                case 'Y':
//                    sprintf (buf,
//                             "New character.\n\rGive me a password for %s: %s",
//                             ch->name, echo_off_str);
//                    send_to_desc (buf, d);
//                    d->connected = CON_GET_NEW_PASSWORD;
					ch->desc->account->p_count++;
                    send_to_desc ("The following races are available:\n\r  ", d);
/*                    for (race = 1; race_table[race].name != NULL; race++)
                    {
                        if (!race_table[race].pc_race)
                        break;

                        write_to_buffer (d, race_table[race].name, 0);
                        write_to_buffer (d, " ", 1);
                    }
                    write_to_buffer (d, "\n\r", 0);
*/

            send_to_desc ("{GHuman        Elf          Dwarf       Giant   \n\r  ", d);
            send_to_desc ("{GDraconian    Gnome        Hobbit      Kender  \n\r  ", d);
            send_to_desc ("{GTroll        Pixie        Half-Elf    Half-Giant\n\r  ", d);
            send_to_desc ("{GHalf-Orc     Duergar      Minotaur    Centaur\n\r  ", d);
            send_to_desc ("{GDrow         StormGiant   CloudGiant  FireGiant\n\r  ", d);
            send_to_desc ("{GFrostGiant   Cyclops      Hydra       Rockseer\n\r  ", d);
            send_to_desc ("{GSvirfnebli   Arial        Felar       Githyanki\n\r  ", d);
            send_to_desc ("{GSatyr 								{x\n\r", d);


                    send_to_desc ("What is your race (help for more information)? ",d);
                    d->connected = CON_GET_NEW_RACE;

                    if (ch->desc->ansi)
                        SET_BIT (ch->act, PLR_COLOUR);
                    break;

                case 'n':
                case 'N':
                    send_to_desc ("Ok, what IS it, then? ", d);
                    free_char (d->character);
                    d->character = NULL;
//                    d->connected = CON_GET_NAME;
                    d->connected = CON_NEW_PLAYER;
                    break;

                default:
                    send_to_desc ("Please type Yes or No? ", d);
                    break;
            }
            break;

        case CON_GET_NEW_PASSWORD:
#if defined(unix)
            write_to_buffer (d, "\n\r", 2);
#endif

            if (strlen (argument) < 5)
            {
                send_to_desc
                    ("Password must be at least five characters long.\n\rPassword: ",
                     d);
                return;
            }

//            pwdnew = crypt (argument, ch->name);
            pwdnew = crypt (argument, d->account->owner);
            for (p = pwdnew; *p != '\0'; p++)
            {
                if (*p == '~')
                {
                    send_to_desc
                        ("New password not acceptable, try again.\n\rPassword: ",
                         d);
                    return;
                }
            }

//            free_string (ch->pcdata->pwd);
//            ch->pcdata->pwd = str_dup (pwdnew);
//            send_to_desc ("Please retype password: ", d);
            free_string(d->account->new_password);
            d->account->new_password = str_dup(pwdnew);
            write_to_buffer( d, "Please retype password: ", 0 );
            d->connected = CON_CONFIRM_NEW_PASSWORD;
            break;

		/*
            free_string (ch->pcdata->pwd);
            ch->pcdata->pwd = str_dup (pwdnew);
*/


        case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
            write_to_buffer (d, "\n\r", 2);
#endif

//            if (strcmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd))
            if ( strcmp( crypt( argument, d->account->owner ),
			     d->account->new_password ) )
            //if (strcmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd))

            {
                send_to_desc ("Passwords don't match.\n\rRetype password: ",
                              d);
                d->connected = CON_GET_NEW_PASSWORD;
                return;
            }

            free_string(d->account->password);
            d->account->password = str_dup(d->account->new_password);

            write_to_buffer (d, echo_on_str, 0);
//            send_to_desc ("The following races are available:\n\r  ", d);
//            for (race = 1; race_table[race].name != NULL; race++)
            show_options(d);
            create_new_account(d->account);
            d->connected = CON_PICK_PLAYER;
        break;

        case CON_DELETE_PLAYER:
            if (!load_char_obj(d, argument))
            {
//                if (!race_table[race].pc_race)
	            write_to_buffer(d, "There is no character with that name.\n\r", 0);
	            show_options(d);
	            d->connected = CON_PICK_PLAYER;
	            return;
            }

            ch = d->character;
            if (str_cmp(ch->pcdata->account, d->account->owner))
            {
	            free_char(ch);
	            d->character = NULL;
	            write_to_buffer(d, "That is not your character.\n\r", 0);
	            show_options(d);
	            d->connected = CON_PICK_PLAYER;
	            return;
            }

            /* delete this character */
           sprintf(buf, "%s %d", ch->name, ch->played/3600);
           free_string(d->account->new_password);
           d->account->new_password = str_dup(buf);
           sprintf(buf, "Are you sure you wish to delete %s [Y/N]? ", ch->name);
           write_to_buffer(d, buf, 0);
           free_char(ch);
           d->character = NULL;
           d->connected = CON_CONFIRM_DEL_PLAYER;

                    break;
//                write_to_buffer (d, race_table[race].name, 0);
//                write_to_buffer (d, " ", 1);

       case CON_CONFIRM_DEL_PLAYER:
           switch(argument[0])
           {
               default:
	               write_to_buffer(d, "Please answer Yes or No.\n\rAre you sure? ", 0);
	           break;
               case 'y': case'Y':
               {
	               char strsave[MAX_STRING_LENGTH];
	               char *ptr;
	               int len = strlen(d->account->new_password), i;

	               /* rip out the substring (string parsing) */
	               strcpy(buf, d->account->players);
	               d->account->p_count--;
	               if ((ptr = strstr(buf, d->account->new_password)) == NULL)
	               {
	                   sprintf(buf, "Unable to delete '%s' from account '%s'",
                       d->account->new_password, d->account->owner);
	                   bug(buf, 0);
                       write_to_buffer(d, "\n\rDeletion FAILED!\n\r", 0);
                       show_options(d);
                       d->connected = CON_PICK_PLAYER;
  	                   return;
	               }

	               if (((i = strlen(buf)) - len) == 0)
	               {
	                   free_string(d->account->players);
	                   d->account->players = str_dup("");
	               }
                   else
	               {
  	                   i -= strlen(ptr);
	                   if (i > 0 && buf[i-1] == ' ')
	                       buf[i-1] = '\0';
                       else
	                       buf[i++] = '\0';
	                   strcat(buf, &d->account->players[i + len]);
	                   free_string(d->account->players);
	                   d->account->players = str_dup(buf);
	               }

                   /* unlink all relevant files and save account */
	               one_argument(d->account->new_password, buf);
	               buf[0] = toupper(buf[0]);
	               for (i = 1; buf[i] != '\0'; i++)
	                   buf[i] = tolower(buf[i]);

               sprintf(strsave, "../accounts/%s/%s", d->account->owner, buf);
	               unlink(strsave);

                   save_account(d->account);

                   write_to_buffer(d, "\n\rCharacter deleted.\n\r", 0);
                  show_options(d);
                   d->connected = CON_PICK_PLAYER;
	               break;
	           }

                   case 'n': case 'N':
	                   show_options(d);
	                   d->connected = CON_PICK_PLAYER;
	               break;
            }
        break;
        case CON_PICK_PLAYER:
            if (toupper(argument[0]) == 'C')
            {
	            write_to_buffer(d, "What shall you be known as? ", 0);
	            d->connected = CON_NEW_PLAYER;
            }
            else if (toupper(argument[0]) == 'D')
            {
	            write_to_buffer(d, "Which player do you wish to delete? ", 0);
                d->connected = CON_DELETE_PLAYER;
            }
            else if (toupper(argument[0]) == 'E')
           {
	            write_to_buffer(d, "What is your name?? ", 0);
	            d->connected = CON_NEW_PLAYER;
            }
            else if (toupper(argument[0]) == 'P')
            {
	            write_to_buffer(d, echo_off_str, 0);
	            write_to_buffer(d, "Please pick a new password: ", 0);
	            d->connected = CON_GET_NEW_PASSWORD;
            }
            else if (toupper(argument[0]) == 'Q')
            {
	            write_to_buffer(d, "Come back soon.\n\r", 0);
	            close_socket(d);
            }
            else
            {
                if ( ( option = get_option_login( d->account, d->account->players,
			       atoi( argument ) ) ) == NULL )
	            {
	                write_to_buffer(d, "That is not an option!\n\r"
				                   "What will it be? ", 0);
	                return;
	            }


/*                else if ( already_logged( d->account->owner ) )
                {
	                write_to_buffer(d, "You already have a different character"
			                           " in the game.\n\r", 0);
	                close_socket(d);
	                return;
                }
*/
                else if (!load_char_obj(d, option))
                {
                    write_to_buffer(d, "ERROR: Your pfile is missing!\n\r", 0);
                    close_socket(d);
                    return;
                }
                else
                {   ch = d->character;
                    if (d->ansi)
                        SET_BIT (ch->act, PLR_COLOUR);
                    else
                        REMOVE_BIT (ch->act, PLR_COLOUR);

                    if (IS_IMMORTAL (ch))
                    {
                        do_function (ch, &do_help, "imotd");
                        d->connected = CON_READ_IMOTD;
                    }
                    else
                    {
                        do_function (ch, &do_help, "motd");
                        d->connected = CON_READ_MOTD;
                    }
			    }
            }
                if ( check_reconnect( d, option, TRUE ) )
		return;
//            write_to_buffer (d, "\n\r", 0);


/* Commented out during account system intall
            send_to_desc ("{gHuman        Elf          Dwarf       Giant   \n\r  ", d);
            send_to_desc ("{gDraconian    Gnome        Hobbit      Kender  \n\r  ", d);
            send_to_desc ("{gTroll        Pixie        Half-Elf    Half-Giant\n\r  ", d);
            send_to_desc ("{gHalf-Orc     Duergar      Minotaur    Centaur\n\r  ", d);
            send_to_desc ("{gDrow         StormGiant   CloudGiant  FireGiant\n\r  ", d);
            send_to_desc ("{gFrostGiant   Cyclops      Hydra       Rockseer\n\r  ", d);
            send_to_desc ("{gSvirfnebli   Arial        Felar       Githyanki\n\r  ", d);
            send_to_desc ("{gSatyr 								\n\r", d);
       BlackDragon  BlueDragon  GreenDragon\n\r  ", d);
            send_to_desc ("{gRedDragon    WhiteDragon  BrassDragon GoldDragon\n\r  ", d);
            send_to_desc ("{gSilverDragon BronzeDragon CopperDragon{x\n\r  ", d);

*/


//            send_to_desc ("What is your race (help for more information)? ",
//                          d);
//            d->connected = CON_GET_NEW_RACE;
            break;

        case CON_GET_NEW_RACE:
            one_argument (argument, arg);

            if (!strcmp (arg, "help"))
            {
                argument = one_argument (argument, arg);
                if (argument[0] == '\0')
                    do_function (ch, &do_help, "race help");
                else
                    do_function (ch, &do_help, argument);
                send_to_desc
                    ("What is your race (help for more information)? ", d);
                break;
            }
            ch = d->character;
            race = race_lookup (argument);


           if (race == 0 || !race_table[race].pc_race)
            {
                send_to_desc ("That is not a valid race.\n\r", d);
                send_to_desc ("The following races are available:\n\r  ", d);
/*
                for (race = 1; race_table[race].name != NULL; race++)
                {
                    if (!race_table[race].pc_race)
                        break;
                    write_to_buffer (d, race_table[race].name, 0);
                    write_to_buffer (d, " ", 1);
                }
*/

            send_to_desc ("{GHuman        Elf          Dwarf       Giant   \n\r  ", d);
            send_to_desc ("{GDraconian    Gnome        Hobbit      Kender  \n\r  ", d);
            send_to_desc ("{GTroll        Pixie        Half-Elf    Half-Giant\n\r  ", d);
            send_to_desc ("{GHalf-Orc     Duergar      Minotaur    Centaur\n\r  ", d);
            send_to_desc ("{GDrow         StormGiant   CloudGiant  FireGiant\n\r  ", d);
            send_to_desc ("{GFrostGiant   Cyclops      Hydra       Rockseer\n\r  ", d);
            send_to_desc ("{GSvirfnebli   Arial        Felar       Githyanki\n\r  ", d);
            send_to_desc ("{GSatyr 								{x\n\r", d);

//                write_to_buffer (d, "\n\r", 0);
                send_to_desc
                    ("What is your race? (help for more information) ", d);
                break;
            }

            ch->race = race;
            /* initialize stats */
            for (i = 0; i < MAX_STATS; i++)
                ch->perm_stat[i] = pc_race_table[race].stats[i];
            ch->affected_by = ch->affected_by | race_table[race].aff;
            ch->imm_flags = ch->imm_flags | race_table[race].imm;
            ch->res_flags = ch->res_flags | race_table[race].res;
            ch->vuln_flags = ch->vuln_flags | race_table[race].vuln;
            ch->form = race_table[race].form;
            ch->parts = race_table[race].parts;
	    ch->weight = pc_race_table[race].weight;
            ch->height = pc_race_table[race].height;

	    
            /* add skills */
            for (i = 0; i < 5; i++)
            {
                if (pc_race_table[race].skills[i] == NULL)
                    break;
                group_add (ch, pc_race_table[race].skills[i], FALSE);
            }
            /* add cost */
            ch->pcdata->points = pc_race_table[race].points;
            ch->size = pc_race_table[race].size;

            send_to_desc ("What is your sex (M/F)? ", d);
            d->connected = CON_GET_NEW_SEX;
            break;


        case CON_GET_NEW_SEX:
            switch (argument[0])
            {
                case 'm':
                case 'M':
                    ch->sex = SEX_MALE;
                    ch->pcdata->true_sex = SEX_MALE;
                    break;
                case 'f':
                case 'F':
                    ch->sex = SEX_FEMALE;
                    ch->pcdata->true_sex = SEX_FEMALE;
                    break;
                default:
                    send_to_desc ("That's not a sex.\n\rWhat IS your sex? ",
                                  d);
                    return;
            }

//            strcpy (buf, "{rSelect a class{x ");
/*            for (iClass = 0; iClass < MAX_CLASS; iClass++)
            {
                if (iClass > 0)
                    strcat (buf, " ");
                strcat (buf, class_table[iClass].name);
            }
*/
            send_to_desc ("{G   Select a Class{x\n\r  ", d);
            send_to_desc ("{G   Sorcerer{x\n\r  ", d);
	    send_to_desc ("{G   Bishop{x\n\r  ", d);
            send_to_desc ("{G   Ninja{x\n\r  ", d);
            send_to_desc ("{G   Hoplite{x\n\r  ", d);
            send_to_desc ("{G   Templar{x\n\r  ", d);
            send_to_desc ("{G   Avenger{x\n\r  ", d);
            send_to_desc ("{G   Lich{x\n\r  ", d);
            send_to_desc ("{G   Shaman{x\n\r  ", d);
            send_to_desc ("{G   Druid{x\n\r  ", d);
            send_to_desc ("{G   Assassin{x\n\r  ", d);
//            strcat (buf, "]: ");
//            write_to_buffer (d, buf, 0);
            d->connected = CON_GET_NEW_CLASS;
            break;

        case CON_GET_NEW_CLASS:
            iClass = class_lookup (argument);

            if (iClass == -1)
            {
                send_to_desc ("That's not a class.\n\rWhat IS your class? ",
                              d);
                return;
            }

            ch->class = iClass;

            sprintf (log_buf, "%s@%s new player.", ch->name, d->host);
            log_string (log_buf);



            wiznet ("Newbie alert!  $N sighted.", ch, NULL, WIZ_NEWBIE, 0, 0);
            wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, get_trust (ch));


            write_to_buffer (d, echo_on_str, 0);


            send_to_desc ("The following gods are available:\n\r  ", d);

        send_to_desc ("{R Good          Neutral         Evil            {x\n\r", d);
        send_to_desc ("{G+--------------+---------------+----------------+     {x\n\r", d);
        send_to_desc ("{R Branchala     Chislev         Chemosh   {x\n\r", d);
        send_to_desc ("{R Habbakuk      Gilean          Hiddukel    {x\n\r", d);
        send_to_desc ("{R KiriJolith    Lunitari        Morgion  {x\n\r", d);
        send_to_desc ("{R Majere        Reorx           Nuitari{x\n\r", d);
        send_to_desc ("{R Mishakal      Shinare         Sargonnas{x\n\r", d);
        send_to_desc ("{R Paladine      Sirrion         Takhisis{x\n\r", d);
        send_to_desc ("{R Solinari      Zivilyn         Zeboim{x\n\r", d);
        send_to_desc ("{G+--------------+---------------+----------------+     {x\n\r", d);

/*            for (god = 0; god_table[god].name != NULL; god++)
            {
                if (!god_table[god].name)
                    break;
                write_to_buffer (d, god_table[god].name, 0);
                write_to_buffer (d, " ", 1);
            }

            write_to_buffer (d, "\n\r", 0);
*/

            send_to_desc ("who is your god (help for more information)? ", d);
            d->connected = CON_GET_NEW_GOD;
            break;

        case CON_GET_NEW_GOD:

//            one_argument (argument, arg);
            god = god_lookup (argument);
            if (!strcmp (arg, "help"))
            {
                argument = one_argument (argument, arg);
                if (argument[0] == '\0')
                    do_function (ch, &do_help, "race help");
                else
                    do_function (ch, &do_help, argument);
                send_to_desc
                    ("What is your race (help for more information)? ", d);
                break;
            }

            if (god == -1)
            {
                send_to_desc ("That is not a valid god.\n\r", d);
                send_to_desc ("The following gods are available:\n\r  ", d);

        send_to_desc ("{R Good          Neutral         Evil            {x\n\r", d);
        send_to_desc ("{G+--------------+---------------+----------------+     {x\n\r", d);
        send_to_desc ("{R Branchala     Chislev         Chemosh   {x\n\r", d);
        send_to_desc ("{R Habbakuk      Gilean          Hiddukel    {x\n\r", d);
        send_to_desc ("{R KiriJolith    Lunitari        Morgion  {x\n\r", d);
        send_to_desc ("{R Majere        Reorx           Nuitari{x\n\r", d);
        send_to_desc ("{R Mishakal      Shinare         Sargonnas{x\n\r", d);
        send_to_desc ("{R Paladine      Sirrion         Takhisis{x\n\r", d);
        send_to_desc ("{R Solinari      Zivilyn         Zeboim{x\n\r", d);
        send_to_desc ("{G+--------------+---------------+----------------+     {x\n\r", d);


/*                for (god = 0; god_table[god].name != NULL; god++)
                {
                    if (!god_table[god].name)
                        break;
                    write_to_buffer (d, god_table[god].name, 0);
                    write_to_buffer (d, " ", 1);
                }
                write_to_buffer (d, "\n\r", 0);
*/


                send_to_desc
                    ("What is your god? (help for more information) ", d);
                break;
            }

        ch->god = god;

            write_to_buffer (d, "\n\r", 2);

            send_to_desc ("You may be good, neutral, or evil.\n\r", d);
            send_to_desc ("Which alignment (G/N/E)? ", d);
            d->connected = CON_GET_ALIGNMENT;
            break;

        case CON_GET_ALIGNMENT:
            switch (argument[0])
            {
                case 'g':
                case 'G':
                    ch->alignment = 750;
                    break;
                case 'n':
                case 'N':
                    ch->alignment = 0;
                    break;
                case 'e':
                case 'E':
                    ch->alignment = -750;
                    break;
                default:
                    send_to_desc ("That's not a valid alignment.\n\r", d);
                    send_to_desc ("Which alignment (G/N/E)? ", d);
                    return;
            }

            write_to_buffer (d, "\n\r", 0);

            group_add (ch, "rom basics", FALSE);
            group_add (ch, class_table[ch->class].base_group, FALSE);
            ch->pcdata->learned[gsn_recall] = 50;
            send_to_desc ("Do you wish to customize this character?\n\r", d);
            send_to_desc
                ("Customization takes time, but allows a wider range of skills and abilities.\n\r",
                 d);
            send_to_desc ("Customize (Y/N)? ", d);
            d->connected = CON_DEFAULT_CHOICE;
            break;

        case CON_DEFAULT_CHOICE:
            write_to_buffer (d, "\n\r", 2);
            switch (argument[0])
            {
                case 'y':
                case 'Y':
                    ch->gen_data = new_gen_data ();
                    ch->gen_data->points_chosen = ch->pcdata->points;
                    do_function (ch, &do_help, "group header");
                    list_group_costs (ch);
                    write_to_buffer (d,
                                     "You already have the following skills:\n\r",
                                     0);
                    do_function (ch, &do_skills, "");
                    do_function (ch, &do_help, "menu choice");
                    d->connected = CON_GEN_GROUPS;
                    break;
                case 'n':
                case 'N':
                    group_add (ch, class_table[ch->class].default_group,
                               TRUE);
                    write_to_buffer (d, "\n\r", 2);
                    write_to_buffer (d,
                                     "Please pick a weapon from the following choices:\n\r",
                                     0);
                    buf[0] = '\0';
                    for (i = 0; weapon_table[i].name != NULL; i++)
                        if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                        {
                            strcat (buf, weapon_table[i].name);
                            strcat (buf, " ");
                        }
                    strcat (buf, "\n\rYour choice? ");
                    write_to_buffer (d, buf, 0);
                    d->connected = CON_PICK_WEAPON;
                    break;
                default:
                    write_to_buffer (d, "Please answer (Y/N)? ", 0);
                    return;
            }
            break;

        case CON_PICK_WEAPON:
            write_to_buffer (d, "\n\r", 2);
            weapon = weapon_lookup (argument);
            if (weapon == -1
                || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
            {
                write_to_buffer (d,
                                 "That's not a valid selection. Choices are:\n\r",
                                 0);
                buf[0] = '\0';
                for (i = 0; weapon_table[i].name != NULL; i++)
                    if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                    {
                        strcat (buf, weapon_table[i].name);
                        strcat (buf, " ");
                    }
                strcat (buf, "\n\rYour choice? ");
                write_to_buffer (d, buf, 0);
                return;
            }

            ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
            write_to_buffer (d, "\n\r", 2);


            do_function (ch, &do_help, "motd");
            d->connected = CON_READ_MOTD;
            break;

        case CON_GEN_GROUPS:
            send_to_char ("\n\r", ch);

            if (!str_cmp (argument, "done"))
            {
                if (ch->pcdata->points == pc_race_table[ch->race].points)
                {
                    send_to_char ("You didn't pick anything.\n\r", ch);
                    break;
                }

                if (ch->pcdata->points < 40 + pc_race_table[ch->race].points)
                {
                    sprintf (buf,
                             "You must take at least %d points of skills and groups",
                             40 + pc_race_table[ch->race].points);
                    send_to_char (buf, ch);
                    break;
                }

                sprintf (buf, "Creation points: %d\n\r", ch->pcdata->points);
                send_to_char (buf, ch);
                sprintf (buf, "Experience per level: %d\n\r",
                         exp_per_level (ch, ch->gen_data->points_chosen));
                if (ch->pcdata->points < 40)
                    ch->train = (40 - ch->pcdata->points + 1) / 2;
                free_gen_data (ch->gen_data);
                ch->gen_data = NULL;
                send_to_char (buf, ch);
                write_to_buffer (d, "\n\r", 2);
                write_to_buffer (d,
                                 "Please pick a weapon from the following choices:\n\r",
                                 0);
                buf[0] = '\0';
                for (i = 0; weapon_table[i].name != NULL; i++)
                    if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                    {
                        strcat (buf, weapon_table[i].name);
                        strcat (buf, " ");
                    }
                strcat (buf, "\n\rYour choice? ");
                write_to_buffer (d, buf, 0);
                d->connected = CON_PICK_WEAPON;
                break;
            }

            if (!parse_gen_groups (ch, argument))
                send_to_char
                    ("Choices are: list,learned,premise,add,drop,info,help, and done.\n\r",
                     ch);

            do_function (ch, &do_help, "menu choice");
            break;

        case CON_READ_IMOTD:
            write_to_buffer (d, "\n\r", 2);
            do_function (ch, &do_help, "motd");
            d->connected = CON_READ_MOTD;
            break;

		/* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
		/* ch MUST be PC here; have nwrite check for PC status! */

		case CON_NOTE_TO:
			handle_con_note_to (d, argument);
			break;

		case CON_NOTE_SUBJECT:
			handle_con_note_subject (d, argument);
			break;

		case CON_NOTE_EXPIRE:
			handle_con_note_expire (d, argument);
			break;

		case CON_NOTE_TEXT:
			handle_con_note_text (d, argument);
			break;

		case CON_NOTE_FINISH:
			handle_con_note_finish (d, argument);
			break;

        case CON_READ_MOTD:
            if (ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
            {
                write_to_buffer (d, "Warning! Null password!\n\r", 0);
                write_to_buffer (d,
                                 "Please report old password with bug.\n\r",
                                 0);
                write_to_buffer (d,
                                 "Type 'password null <new password>' to fix.\n\r",
                                 0);
            }

            write_to_buffer (d,
                             "\n\rWelcome to ROM 2.4.  Please don't feed the mobiles!\n\r",
                             0);
            ch->next = char_list;
            char_list = ch;
            d->connected = CON_PLAYING;
            reset_char (ch);

            if (ch->level == 0)
            {
		if(mud_ansicolor)
                	SET_BIT (ch->act, PLR_COLOUR);
		if(mud_telnetga)
			SET_BIT (ch->comm, COMM_TELNET_GA);

                ch->perm_stat[class_table[ch->class].attr_prime] += 3;

                ch->level = 1;
//                ch->exp = exp_per_level (ch, ch->pcdata->points);
		ch->exp = 0;
                ch->hit = ch->max_hit;
                ch->mana = ch->max_mana;
                ch->move = ch->max_move;
                ch->train = 3;
                ch->practice = 5;
                // sprintf (buf, "the %s", title_table[ch->class][ch->level]
                //         [ch->sex == SEX_FEMALE ? 1 : 0]);
                set_title (ch, "The Newbie");
                do_prompt ( ch, "all" );

                creations_today++;

                do_function (ch, &do_outfit, "");
                obj_to_char (create_object (get_obj_index (OBJ_VNUM_MAP), 0),
                             ch);

                char_to_room (ch, get_room_index (ROOM_VNUM_SCHOOL));
                send_to_char ("\n\r", ch);
                do_function (ch, &do_help, "newbie info");
                send_to_char ("\n\r", ch);
            }
            else if (ch->in_room != NULL)
            {
                char_to_room (ch, ch->in_room);
            }
            else if (IS_IMMORTAL (ch))
            {
                char_to_room (ch, get_room_index (ROOM_VNUM_CHAT));
            }
            else
            {
                char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
            }

            sprintf(buf, "{r%s {whas entered {YEye of the Cyclops.{x", ch->name);
            enter_info(buf);


            act ("$n has entered the game.", ch, NULL, NULL, TO_ROOM);
            do_function (ch, &do_look, "auto");

            ++logins_today;


            wiznet ("$N has left real life behind.", ch, NULL,
                    WIZ_LOGINS, WIZ_SITES, get_trust (ch));

            if (ch->pet != NULL)
            {
                char_to_room (ch->pet, ch->in_room);
                act ("$n has entered the game.", ch->pet, NULL, NULL,
                     TO_ROOM);
            }

			send_to_char("\n", ch);
            do_function (ch, &do_board, "");
          
            break;
    }

    return;
}

