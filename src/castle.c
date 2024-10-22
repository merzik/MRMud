/***************************************************************************
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer, *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe. *
* *
* Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael *
* Chastain, Michael Quan, and Mitchell Tse. *
* *
* In order to use any part of this Merc Diku Mud, you must comply with *
* both the original Diku license in 'license.doc' as well the Merc *
* license in 'license.txt'. In particular, you may not remove either of *
* these copyright notices. *
* *
* Much time and thought has gone into this software and you are *
* benefitting. We hope that you share your changes too. What goes *
* around, comes around. *
* *
* MrMud improvements (C) 1993, 1994 by Douglas Michael & David Bills. *
* *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"

/* costs are in thousands of dollars */
#define COST_OF_ENTRANCE 30000
#define COST_OF_ROOM 20000
#define COST_OF_DOOR 4000
#define COST_OF_BACKDOOR 70000
#define COST_OF_STAT 10
#define COST_OF_SET 1000
#define COST_OF_CREATE 15000
#define COST_OF_BANK 90000
#define COST_OF_DISCONNECT 1000

bool castle_needs_saving=FALSE;

/* return name of bits that are set */
char *broken_bits( int , char * , bool );
char *justify( char *);

extern int top_mob_index;
extern int top_reset;

/*
* get_bitvector_value: either sets number to the value represented in name,
* or it searches the bitvector list for a matching name and sets number to
* that bitvector's value--in either case it returns TRUE. Otherwise, it
* returns FALSE. If the string allowed is not NULL, then it limits the
* values searched to those staring with the allowed string. Also doesn't
* accept numbers if allowed is not NULL.
*/
bool get_bitvector_value(char *name,int *number,char *allowed)
{
	int cnt;

	if(allowed==NULL && is_number(name))
	{
		*number=atol(name);
		return TRUE;
	}
	if(is_name_short(allowed,name))
		for( cnt=0; cnt < MAX_BITVECTOR; cnt++)
			if( is_name( name, bitvector_table[cnt].name ))
			{
				*number=bitvector_table[cnt].value;
				return TRUE;
			}
			*number=-1;
			return FALSE;
}

void list_bitvectors(CHAR_DATA *ch,char *prefix)
{
	int cnt;
	char buf[81],bigBuf[MAX_STRING_LENGTH];

	bigBuf[0]='\0';
	for( cnt=0; cnt < MAX_BITVECTOR; cnt++)
		if( is_name_short( prefix, bitvector_table[cnt].name ))
		{
			sprintf(buf," %s\n\r",bitvector_table[cnt].name);
			strcat(bigBuf,buf);
		}
		if(bigBuf[0]=='\0')
		{
			sprintf(buf,"There are no bitvectors starting with '%s'.\n\r",prefix);
			send_to_char(buf,ch);
			return;
		}
		send_to_char(bigBuf,ch);
		return;
}

bool do_castle_mstat( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	sh_int numDice,sizeDice;
	MPROG_DATA *mprg;



	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Mstat whom?\n\r", ch );
		return FALSE;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return FALSE;
	}

	if(!IS_NPC(victim))
	{
		send_to_char( "Don't be silly.\n\r", ch );
		return FALSE;
	}

	if ( victim->pIndexData->creator_pvnum!=ch->pcdata->pvnum )
	{
		send_to_char( "You can't stat them!\n\r", ch );
		return FALSE;
	}

	sprintf( buf, "Name: %s.\n\r", victim->name );
	send_to_char( buf, ch );
	sprintf( buf, "Short description: %s\n\rLong description: %s",
		victim->short_descr,
		victim->long_descr[0] != '\0' ? victim->long_descr : "(none).\n\r" );
	send_to_char( buf, ch );

	sprintf( buf, "Description: %s\n\r", victim->description);
	send_to_char( buf, ch );


	sprintf( buf, "Sex: %s. Room: %u. Race: %s.\n\r",
		victim->sex == SEX_MALE ? "male" :
	victim->sex == SEX_FEMALE ? "female" : "neutral",
		victim->in_room == NULL ? 0 : victim->in_room->vnum,
		race_table[victim->race].race_name);
	send_to_char( buf, ch );

	sprintf( buf, "Str: %d. Int: %d. Wis: %d. Dex: %d. Con: %d.\n\r",
		get_curr_str(victim),
		get_curr_int(victim),
		get_curr_wis(victim),
		get_curr_dex(victim),
		get_curr_con(victim) );
	send_to_char( buf, ch );

	sprintf( buf, "Hp: %d/%d. Mana: %d/%d. Move: %d/%d.\n\r",
		victim->hit, victim->max_hit,
		victim->mana, victim->max_mana,
		victim->move, victim->max_move);
	send_to_char( buf, ch );

	sprintf( buf, "Lv: %d. Class: %d. Align: %d. AC: %d.\n\r",
		victim->level, victim->class, victim->alignment,
		98-mob_armor(victim)+victim->npcdata->armor/3-victim->level*4);
	send_to_char( buf, ch );

	/* dice indicator added by Dug 10/2/93 */
	wield = get_eq_char( victim, WEAR_WIELD );
	if((wield!=NULL)&&wield->item_type!=ITEM_WEAPON)
		wield=NULL;
	numDice=(wield!=NULL)?wield->value[1]:(IS_NPC(victim)?victim->npcdata->damnodice:1);
	sizeDice=(wield!=NULL)?wield->value[2]:(IS_NPC(victim)?victim->npcdata->damsizedice:4+victim->level/4);
	sprintf( buf, "Hitroll: %d. Damroll: (%dd%d)+%d. Position: %d. Wimpy: %d.\n\r", GET_HITROLL(victim), numDice, sizeDice,victim->npcdata->damplus+GET_DAMROLL(victim), victim->position, victim->wimpy);
	send_to_char( buf, ch );

	sprintf( buf, "Fighting: %s.\n\r",
		victim->fighting ? victim->fighting->who->name : "(none)" );
	send_to_char( buf, ch );

	sprintf( buf, "Carry number: %d. Carry weight: %d.\n\r",
		victim->carry_number, victim->carry_weight );
	send_to_char( buf, ch );

	sprintf( buf, "Timer: %d. Act: %s.\n\r", victim->timer,
		broken_bits(victim->act, "CACT_", FALSE));
	send_to_char( buf, ch );

	sprintf( buf, "Master: %s. Leader: %s. Affected by: %s.\n\r",
		victim->master ? victim->master->name : "(none)",
		victim->leader ? victim->leader->name : "(none)",
		affect_bit_name( victim->affected_by ) );
	send_to_char( buf, ch );

	/* added shot_timer and shot_aware entries -dug */
	sprintf(buf,"Basic Armor: %d Current Armor: %d\n\r",
		victim->npcdata->armor, mob_armor(victim) );
	send_to_char(buf,ch);

	for ( paf = victim->first_affect; paf != NULL; paf = paf->next )
	{
		sprintf(buf,"Spell: '%s' modifies %s by %d for %d hours with bits %s.\n\r",
			skill_table[(int) paf->type].name,
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration,
			affect_bit_name( paf->bitvector ));
		send_to_char( buf, ch );
	}

	if(IS_SET( victim->pIndexData->act, ACT_BODY))
	{
		int cnt;
		strcpy(buf, "Body parts: ");
		for( cnt=0; cnt<MAX_BODY; cnt++)
			if( IS_SET( victim->pIndexData->body_parts, 1<<cnt))
			{
				strcat( buf, body_table[cnt].name);
				strcat( buf, " ");
			}
			strcat( buf, "\n\r");
			strcat(buf, "Attack parts: ");
			for( cnt=0; cnt<MAX_BODY; cnt++)
				if( IS_SET( victim->pIndexData->attack_parts, 1<<cnt))
				{
					strcat( buf, body_table[cnt].name);
					strcat( buf, " ");
				}
				strcat( buf, "\n\r");
				send_to_char( buf, ch );
	}

	if ( !( victim->pIndexData->progtypes ) )
	{
		send_to_char( "That Mobile has no Programs set.\n\r", ch);
		return TRUE;
	}

	for ( mprg = victim->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
	{
		sprintf( buf, ">%s %s\n\r%s\n\r",
			mprog_type_to_name( mprg->type ),
			mprg->arglist, mprg->comlist );
		send_to_char( buf, ch );
	}

	return TRUE;
}

bool do_castle_ostat( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Ostat what?\n\r", ch );
		return FALSE;
	}

	if(is_number(arg))
	{
		int anum;

		anum=atol(arg);
		for(obj=first_object;obj!=NULL;obj=obj->next)
			if(obj->pIndexData!=NULL&&obj->pIndexData->vnum==anum)
				break;
		if(obj==NULL)
		{
			send_to_char( "No object loaded has that vnum.\n\r", ch );
			return FALSE;
		}
	}
	else
	{
		if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
		{
			send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
			return FALSE;
		}
	}

	if ( obj->pIndexData->creator_pvnum!=ch->pcdata->pvnum )
	{
		send_to_char( "You can't stat that!\n\r", ch );
		return FALSE;
	}

	sprintf( buf, "Name: %s.", obj->name );
	send_to_char( buf, ch );

	sprintf( buf, "Type: %s. Level: %d.\n\r", item_type_name( obj ),
		obj->level );
	send_to_char( buf, ch );

	sprintf( buf, "Short description: %s.\n\rLong description: %s\n\r",
		obj->short_descr, obj->description );
	send_to_char( buf, ch );

	sprintf( buf, "Wear bits: %d. Extra bits: %s.\n\r",
		obj->wear_flags, extra_bit_name( obj->extra_flags ) );
	send_to_char( buf, ch );

	sprintf( buf, "Weight: %d/%d.\n\r", obj->weight, get_obj_weight( obj ) );
	send_to_char( buf, ch );

	sprintf( buf, "Values: %d %d %d %d.\n\r",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );

	if ( obj->first_extradesc != NULL || obj->pIndexData->first_extradesc != NULL )
	{
		EXTRA_DESCR_DATA *ed;

		strcpy( buf, "Extra description keywords: '" );

		for ( ed = obj->first_extradesc; ed != NULL; ed = ed->next )
		{
			strcat( buf, ed->keyword );
			if ( ed->next != NULL )
				strcat( buf, " " );
		}

		for ( ed = obj->pIndexData->first_extradesc; ed != NULL; ed = ed->next )
		{
			strcat( buf, ed->keyword );
			if ( ed->next != NULL )
				strcat( buf, " " );
		}
		strcat( buf, "'.\n\r" );
		send_to_char( buf, ch);
	}

	for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
	{
		sprintf( buf, "Affects %s by %d.\n\r",
			affect_loc_name( paf->location ), paf->modifier );
		send_to_char( buf, ch );
	}

	for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
	{
		sprintf( buf, "Affects %s by %d.\n\r",
			affect_loc_name( paf->location ), paf->modifier );
		send_to_char( buf, ch );
	}

	if(obj->pIndexData->obj_prog!=NULL)
		send_to_char( "Object has 'Object program' (TM).\n\r",ch);

	return TRUE;
}

bool do_castle_rstat( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	int door;

	one_argument( argument, arg );
	location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
	if ( location == NULL )
	{
		send_to_char( "No such location.\n\r", ch );
		return FALSE;
	}

	if ( location->creator_pvnum!=ch->pcdata->pvnum )
	{
		send_to_char( "You can't stat this room!\n\r", ch );
		return FALSE;
	}

	sprintf( buf, "NAME: '%s'\n\rArea: '%s' level %d.\n\r",
		location->name,
		location->area->name,
		location->area->average_level );
	send_to_char( buf, ch );

	sprintf( buf, "SECTOR: %s.\n\r",
		broken_bits(location->sector_type, "SECT_", TRUE));
	send_to_char( buf, ch );

	sprintf( buf, "room FLAGs: %s.\n\rDESCription:\n\r%s",
		broken_bits(location->room_flags, "CROOM_", FALSE),
		location->description );
	send_to_char( buf, ch );

	if ( location->first_extradesc != NULL )
	{
		EXTRA_DESCR_DATA *ed;

		strcpy( buf, "EXTRA description KEYWORDs: '" );
		for ( ed = location->first_extradesc; ed; ed = ed->next )
		{
			strcat( buf, ed->keyword );
			if ( ed->next != NULL )
				strcat( buf, " " );
		}
		strcat( buf, "'.\n\r");
		send_to_char( buf, ch );
	}

	for ( door = 0; door <= 5; door++ )
	{
		EXIT_DATA *pexit;

		if ( ( pexit = location->exit[door] ) != NULL )
		{
			sprintf( buf, "DOOR: %s. To: %u. ",
				broken_bits(door, "DIR_", TRUE),
				pexit->to_room != NULL ? pexit->to_room->vnum : 0);
			send_to_char( buf, ch );
			sprintf( buf, " KEY: %d. exit FLAGs: %s.\n\r KEYWORD: '%s'. DESCription: %s",
				pexit->key, broken_bits(pexit->exit_info, "CEX_", FALSE),
				pexit->keyword,
				pexit->description[0] != '\0'
				? pexit->description : "(none).\n\r" );
			send_to_char( buf, ch );
		}
	}

	return TRUE;
}

void do_castle( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH],buf[81],bigBuf[MAX_STRING_LENGTH],buf2[81];
	CASTLE_DATA *castle;
	int argn,bargain,reconnectEntrance;
	bool clanbank;
	/*ROOM_INDEX_DATA *location;*/

	if ( IS_NPC(ch) )
		return;

	reconnectEntrance=FALSE;

	if(argument[0]=='\0')
	{
		if(ch->pcdata->castle!=NULL)
		{
			castle=ch->pcdata->castle;
			send_to_char("CASTLE: ",ch);
			if(room_index[castle->entrance]==NULL)
				sprintf(bigBuf,"Talk to Order.\n\r");
			else
				sprintf(bigBuf,"Cost so far: %d000\n\r #Rooms: %d #Creatures: %d #Objects: %d\n\rConnect_Room: %s, Connect_Dir: %s\n\r",
				castle->cost,
				castle->num_rooms, castle->num_mobiles, castle->num_objects,
				(castle->door_room>0)?room_index[castle->door_room]->name:"none",
				(castle->door_room>0)?
				broken_bits(castle->door_dir,"DIR_",TRUE):"none"
				);
			send_to_char(bigBuf,ch);
		}
		else
		{
			switch(ch->in_room->sector_type)
			{
			case SECT_FIELD:
			case SECT_FOREST:
			case SECT_HILLS:
			case SECT_MOUNTAIN:
				if ( !IS_SET(ch->in_room->room_flags, ROOM_NO_CASTLE) &&
					!IS_SET(ch->in_room->area->flags, AFLAG_NOCASTLE ) )
					send_to_char("CASTLE: You could start a castle here.\n\r",ch);
				else
					send_to_char("CASTLE: You can't start a castle here.\n\r",ch);
				break;
			default:
				send_to_char("CASTLE: You can't start a castle here.\n\r",ch);
				break;
			}
		}
		return;
	}

	argument = one_argument( argument, arg );

	/* set up bargain skill */
	bargain=IS_NPC(ch)?100:100-(25*(ch->pcdata->learned[gsn_bargain]))/100;

	castle=ch->pcdata->castle;
	if ( !strcasecmp( arg, "entrance" ) || !strcasecmp( arg, "room" ) )
	{
		EXIT_DATA *pexit,*pexit_rev;
		ROOM_INDEX_DATA *pRoomIndex;
		int vnum,iHash,door,room_cost;

		if(!IS_SET(ch->in_room->room_flags,ROOM_IS_CASTLE))
		{
			switch(ch->in_room->sector_type)
			{
			case SECT_FIELD:
			case SECT_FOREST:
			case SECT_HILLS:
			case SECT_MOUNTAIN:
				if ( IS_SET(ch->in_room->room_flags, ROOM_NO_CASTLE) ||
					IS_SET( ch->in_room->area->flags, AFLAG_NOCASTLE) )
				{
					send_to_char("You can't build a castle here!\n\r",ch);
					return;
				}
				break;
			default:
				send_to_char("You can't build a castle here!\n\r",ch);
				return;
				break;
			}
		}
		else if(castle==NULL)
		{
			send_to_char("You can't build a castle in a castle!\n\r",ch);
			return;
		}
		if(castle!=NULL && ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
		{
			/* OK, check for case of lost entrance, allow them to put one if
			they aren't currently in someone elses castle */
			if(IS_SET(ch->in_room->room_flags,ROOM_IS_CASTLE))
			{
				send_to_char("You can't put the entrance to your castle in a castle!\n\r",ch);
				return;
			}
			pRoomIndex=room_index[castle->entrance];
			/* make sure their castle->entrance hasn't been screwed up */
			if(pRoomIndex==NULL || ch->pcdata->pvnum!=pRoomIndex->creator_pvnum ||
				!(IS_SET(pRoomIndex->room_flags,ROOM_IS_ENTRANCE)&&
				IS_SET(pRoomIndex->room_flags,ROOM_IS_CASTLE)))
			{/* drat, entrance messed up...fix it */
				for(vnum=20000;vnum<32000;vnum++)
				{
					if((pRoomIndex=room_index[vnum])==NULL)
						continue;
					if(ch->pcdata->pvnum==pRoomIndex->creator_pvnum &&
						IS_SET(pRoomIndex->room_flags,ROOM_IS_ENTRANCE)&&
						IS_SET(pRoomIndex->room_flags,ROOM_IS_CASTLE))
						break;
				}
				if(vnum>=32000||room_index[vnum]==NULL)
				{
					send_to_char("I'm sorry, you don't seem to have a castle, tell the Implementors.\n\r",ch);
					return;
				}
				/* update user's castle structure */
				ch->pcdata->castle->entrance=vnum;

			}
			for ( door = 0; door <= 5; door++ )
			{
				if((pexit=pRoomIndex->exit[door])== NULL)
					continue;
				else
				{
					if(pexit->to_room==NULL || pexit->to_room->vnum==0)
					{
						reconnectEntrance=TRUE;
						argn=rev_dir[door];
						/* connect entrance to this room (code in db.c) */
						if(ch->in_room->exit[argn] != NULL)
						{
							sprintf(buf,"You can't connect your castle here.\n\r The necessary direction(%s) is blocked.\n\r",dir_name[argn]);
							send_to_char(buf,ch);
							return;
						}
						CREATE( pexit_rev, EXIT_DATA, 1);
						top_exit++;
						pexit_rev->description = STRALLOC(pexit->description);
						pexit_rev->keyword = STRALLOC(pexit->keyword);
						pexit_rev->exit_info = pexit->exit_info;
						pexit_rev->pvnum = pexit->pvnum;
						pexit_rev->key = pexit->key;
						pexit_rev->vnum = pRoomIndex->vnum;
						pexit_rev->to_room = pRoomIndex;
						ch->in_room->exit[argn] = pexit_rev;
						pexit->to_room=ch->in_room;
						castle_needs_saving=TRUE;

						/* show the happenings */
						act("Builders come, work, take a break, and leave.",ch,NULL,NULL,TO_ROOM);
						switch(argn)
						{
						case DIR_UP:
							strcpy(buf2,"above you");
							break;
						case DIR_DOWN:
							strcpy(buf2,"below you");
							break;
						default:
							sprintf(buf2,"to the %s",dir_name[argn]);
							break;
						}
						sprintf(buf,"Builders come and build a room %s!",buf2);
						act(buf, ch, NULL, NULL, TO_CHAR );

						return;
					}
				}
			}
			if(!reconnectEntrance)
			{
				send_to_char("You already have a castle entrance!\n\r",ch);
				return;
			}
		}

		if(!get_bitvector_value(argument,&argn,"DIR_"))
		{
			strcpy(bigBuf,"You must specify a direction for the room!\n\r");
			strcat(bigBuf,"Use: castle room <dir>\n\r");
			strcat(bigBuf,"WHERE: <dir> can be one of:\n\r");
			send_to_char(bigBuf,ch);
			list_bitvectors(ch,"DIR_");
			return;
		}

		if(ch->in_room->exit[argn]!=NULL)
		{
			send_to_char("You cannot add onto this room in that direction!\n\r",ch);
			return;
		}

		/* lets see if you can afford it */
		room_cost=(castle==NULL)?COST_OF_ENTRANCE:COST_OF_ROOM;
		room_cost=(bargain*room_cost)/100*1000;
		if(ch->gold<room_cost)
		{
			sprintf(buf,"It costs $%d to build a room.\n\r",room_cost);
			send_to_char(buf,ch);
			return;
		}
		if(castle!=NULL && castle->num_rooms >= ch->level/4+2 && ch->level <= 96)
		{
			send_to_char("You cannot make any more rooms at your level!\n\r",ch);
			return;
		}
		if(castle==NULL)
		{
			CREATE(ch->pcdata->castle, CASTLE_DATA, 1);
			ch->pcdata->castle->entrance=0;
			ch->pcdata->castle->door_room=-1;
			ch->pcdata->castle->door_dir=-1;
			ch->pcdata->castle->has_backdoor=FALSE;
			ch->pcdata->castle->cost=0;
			ch->pcdata->castle->num_rooms=0;
			ch->pcdata->castle->num_mobiles=0;
			ch->pcdata->castle->num_objects=0;
			ch->pcdata->castle->reign_room=0;
			castle=ch->pcdata->castle;
		}
		sprintf(buf,"A contractor comes and takes $%d.\n\r",room_cost);
		send_to_char(buf,ch);
		/* create room */
		sprintf(buf,"The Castle of %s.",ch->name);
		CREATE( pRoomIndex, ROOM_INDEX_DATA, 1);
		pRoomIndex->name = STRALLOC(buf);
		pRoomIndex->first_person = NULL;
		pRoomIndex->last_person = NULL;
		pRoomIndex->first_content = NULL;
		pRoomIndex->last_content = NULL;
		pRoomIndex->first_extradesc = NULL;
		pRoomIndex->last_extradesc = NULL;
		pRoomIndex->area = room_index[20000]->area;
		/* find an empty vnum */
		for(vnum=20000;room_index[vnum]!=NULL && vnum<27000;vnum++);
		if(vnum>=27000)
		{
			send_to_char("I'm sorry, you can't create a room, tell the Implementors.\n\r",ch);
			return;
		}

		pRoomIndex->vnum = vnum;
		pRoomIndex->owned_by = ch->pcdata->pvnum;
		pRoomIndex->creator_pvnum = ch->pcdata->pvnum;
		pRoomIndex->description = STRALLOC( " This room is bare of all ornamentation and appears to be relatively new." );
		pRoomIndex->room_flags =ROOM_IS_CASTLE|ROOM_NO_MOB|ROOM_INDOORS;
		pRoomIndex->sector_type = SECT_INSIDE;
		pRoomIndex->light = 0;

		for ( door = 0; door <= 5; door++ )
			pRoomIndex->exit[door] = NULL;
		for ( door = 0; door< MAX_LAST_LEFT; door++)
			strcpy( pRoomIndex->last_left[door], "");
		/* new room exit points to old room */
		CREATE(pexit, EXIT_DATA, 1);
		pexit->description = STRALLOC("");
		pexit->keyword = STRALLOC("door");
		pexit->exit_info = 0;
		pexit->pvnum = -1;
		pexit->key = -1;
		pexit->vnum = ch->in_room->vnum;
		pexit->to_room = ch->in_room;
		pRoomIndex->exit[rev_dir[argn]]= pexit;
		top_exit++;
		/* old rooms exit points to new room */
		CREATE(pexit, EXIT_DATA, 1);
		pexit->description = STRALLOC(buf );
		pexit->keyword = STRALLOC("door");
		pexit->exit_info = 0;
		pexit->pvnum = ch->pcdata->pvnum;
		pexit->key = -1;
		pexit->vnum = pRoomIndex->vnum;
		pexit->to_room = pRoomIndex;
		ch->in_room->exit[argn] = pexit;
		top_exit++;
		/* put new room in hash table */
		iHash = vnum % MAX_KEY_HASH;
		pRoomIndex->next = room_index_hash[iHash];
		room_index_hash[iHash]= pRoomIndex;
		top_room++;
		room_index[vnum]=pRoomIndex;

		if (vnum > pRoomIndex->area->hi_r_vnum)
			pRoomIndex->area->hi_r_vnum = vnum;
		if (vnum < pRoomIndex->area->low_r_vnum)
			pRoomIndex->area->low_r_vnum = vnum;
		/* show the happenings */
		act("Builders come, work, take a break, and leave.",ch,NULL,NULL,TO_ROOM);
		switch(argn)
		{
		case DIR_UP:
			strcpy(buf2,"above you");
			break;
		case DIR_DOWN:
			strcpy(buf2,"below you");
			break;
		default:
			sprintf(buf2,"to the %s",dir_name[argn]);
			break;
		}
		sprintf(buf,"Builders come and build a room %s!",buf2);
		act(buf, ch, NULL, NULL, TO_CHAR );

		/* modify the appropriate character and castle data */
		ch->gold-=room_cost;
		castle->cost+=room_cost/1000;
		castle_needs_saving=TRUE;
		if(castle->entrance==0)
		{
			SET_BIT(pRoomIndex->room_flags,ROOM_SAFE); /* entrance is safe */
			SET_BIT(pRoomIndex->room_flags,ROOM_IS_ENTRANCE);
			castle->entrance=vnum;
		}
		castle->num_rooms+=1;

		return;
	}

	if (castle==NULL)
	{
		send_to_char("You have no castle established yet.\n\r",ch);
		return;
	}

	if ( !strcasecmp( arg, "door" ) )
	{
		send_to_char("The 'castle door...' command has changed to 'castle connect...'\n\r",ch);
		return;
	}
	if ( !strcasecmp( arg, "connect" ) )
	{
		EXIT_DATA *pexit,*castleexit;
		int door_cost;
		bool clanfounder=FALSE;

		if(!strcasecmp(argument,"clear"))
		{
			castle->door_room=-1;
			castle->door_dir=-1;
			return;
		}
		if (ch->pcdata->clan !=NULL && ch->pcdata->clan &&
			!strcasecmp( ch->name, ch->pcdata->clan->leader ) )
			clanfounder=TRUE;

		if (ch->in_room->creator_pvnum!=ch->pcdata->pvnum &&
			castle->has_backdoor && !clanfounder)
		{
			send_to_char("You can only have ONE back door!\n\r",ch);
			return;
		}

		/* lets see if you can afford it */
		if(ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
			door_cost=UMAX(COST_OF_BACKDOOR,ch->level*900);
		else
			door_cost=COST_OF_DOOR;
		door_cost=(bargain*door_cost)/100*1000;
		if(ch->gold<door_cost)
		{
			if(ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
			{
				sprintf(buf,"It costs $%d to build a BACK door.\n\r",door_cost);
				send_to_char(buf,ch);
				return;
			}
			sprintf(buf,"It costs $%d to build a connection.\n\r",door_cost);
			send_to_char(buf,ch);
			return;
		}
		if(!get_bitvector_value(argument,&argn,"DIR_")&&(castle->door_room==-1))
		{
			strcpy(bigBuf,"You must specify a direction for the first side of a connection!\n\r");
			strcat(bigBuf,"Use: castle connect <dir>\n\r");
			strcat(bigBuf,"WHERE: <dir> can be one of:\n\r");
			send_to_char(bigBuf,ch);
			list_bitvectors(ch,"DIR_");
			return;
		}
		/* default direction is opposite of first one */
		if(castle->door_room!=-1 && argn==-1)
			argn=rev_dir[castle->door_dir];
		if(ch->in_room->exit[argn]!=NULL)
		{
			send_to_char("You cannot put a connection in that direction!\n\r",ch);
			return;
		}
		if(!IS_SET(ch->in_room->room_flags,ROOM_IS_CASTLE))
		{
			switch(ch->in_room->sector_type)
			{
			case SECT_FIELD:
			case SECT_FOREST:
			case SECT_HILLS:
			case SECT_MOUNTAIN:
				if ( IS_SET(ch->in_room->room_flags, ROOM_NO_CASTLE) ||
					IS_SET( ch->in_room->area->flags, AFLAG_NOCASTLE) )
				{
					send_to_char("You cannot put a connection in this room!\n\r",ch);
					return;
				}
				break;
			default:
				send_to_char("You cannot put a connection in this room!\n\r",ch);
				return;
				break;
			}
		}
		/*
		if(IS_SET(ch->in_room->room_flags,ROOM_NO_RECALL) ||
		IS_SET( ch->in_room->area->flags, AFLAG_NOTELEPORT ) ||
		IS_SET( ch->in_room->area->flags, AFLAG_NORECALL ) ||
		IS_SET( ch->in_room->area->flags, AFLAG_NOCASTLE ) )
		{
		send_to_char("You cannot put a connection in this room!\n\r",ch);
		return;
		}
		*/
		if(castle->door_room==-1)
		{
			castle->door_room=ch->in_room->vnum;
			castle->door_dir=argn;
			send_to_char("Now you have to go define the other side of the connection.\n\r",ch);
			return;
		}
		if(room_index[castle->door_room]->exit[castle->door_dir]!=NULL)
		{
			send_to_char("Your first connection is in an invalid direction! Clearing first connection.\n\r",ch);
			castle->door_room=-1;
			castle->door_dir=-1;
			return;
		}
		if(((ch->in_room->creator_pvnum!=ch->pcdata->pvnum)||
			(room_index[castle->door_room]->creator_pvnum!=ch->pcdata->pvnum)) &&
			(IS_SET(ch->in_room->room_flags,ROOM_IS_ENTRANCE) ||
			IS_SET(room_index[castle->door_room]->room_flags,ROOM_IS_ENTRANCE)))
		{
			send_to_char("You can't have a backdoor from your entrance!\n\r",ch);
			return;
		}
		/* make sure they aren't creating links from outside to outside */
		if((ch->in_room->creator_pvnum!=ch->pcdata->pvnum)&&
			(room_index[castle->door_room]->creator_pvnum!=ch->pcdata->pvnum))
		{
			send_to_char("You can't make a connection between two outside areas.\n\r",ch);
			return;
		}
		/* check for back door creation in reverse */
		if(room_index[castle->door_room]->creator_pvnum!=ch->pcdata->pvnum)
		{
			door_cost=UMAX(COST_OF_BACKDOOR,ch->level*900);
			door_cost=(bargain*door_cost)/100*1000;
			/* darn, changed cost so check funds again */
			if(ch->gold<door_cost)
			{
				sprintf(buf,"It costs $%d to build a BACK door.\n\r",door_cost);
				send_to_char(buf,ch);
			}
		}
		/* other rooms exit points to room */
		castleexit=NULL;
		if(room_index[castle->door_room]->creator_pvnum==ch->pcdata->pvnum)
		{
			CREATE(pexit, EXIT_DATA, 1);
			pexit->description = STRALLOC("");
			pexit->keyword = STRALLOC("door");
			pexit->exit_info = 0;
			pexit->pvnum = ch->pcdata->pvnum;
			pexit->key = -1;
			pexit->vnum = ch->in_room->vnum;
			pexit->to_room = ch->in_room;
			room_index[castle->door_room]->exit[castle->door_dir]= pexit;
			top_exit++;
			castleexit=pexit;
		}
		/* room exit points to other room */
		if(room_index[ch->in_room->vnum]->creator_pvnum==ch->pcdata->pvnum)
		{
			CREATE(pexit, EXIT_DATA, 1);
			pexit->description = STRALLOC("");
			pexit->keyword = STRALLOC("door");
			pexit->exit_info = 0;
			pexit->pvnum = ch->pcdata->pvnum;
			pexit->key = -1;
			pexit->vnum = castle->door_room;
			pexit->to_room = room_index[castle->door_room];
			ch->in_room->exit[argn] = pexit;
			top_exit++;
			castleexit=pexit;
		}

		act("Builders come, take a break, and leave.",ch,NULL,NULL,TO_ROOM);
		send_to_char("The door has been made.\n\r",ch);

		/* modify the appropriate character and castle data */
		ch->gold-=door_cost;
		castle->cost+=door_cost/1000;
		castle_needs_saving=TRUE;
		castle->door_room=-1;
		castle->door_dir=-1;
		if((ch->in_room->creator_pvnum!=ch->pcdata->pvnum &&
			!IS_SET(ch->in_room->room_flags,ROOM_RIP))||
			(castleexit->to_room->creator_pvnum!=ch->pcdata->pvnum &&
			!IS_SET(castleexit->to_room->room_flags,ROOM_RIP)))
		{
			castle->has_backdoor=TRUE;
			if (!clanfounder)
				SET_BIT(castleexit->exit_info,EX_BACKDOOR);
			else
			{
				SET_BIT(castleexit->exit_info,EX_CLAN_BACKDOOR);
				ch->pcdata->clan->num_backdoors++;
				ch_printf(ch, "You just made a clan-wide backdoor for %s.\n\r",
					ch->pcdata->clan->name);
			}
		}

		return;
	}

	if (ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
	{
		send_to_char("You are NOT in your castle!\n\r",ch);
		return;
	}

	/* Castle disconnect added by Merzik 11-28-00 */

	if ( !strcasecmp( arg, "disconnect" ) )
	{
		ROOM_INDEX_DATA *connected_room;
		int disc_cost, door;
		bool clanfounder=FALSE, backdoor=FALSE;

		/* Make sure player is in their own castle */

		if( ch->in_room->creator_pvnum != ch->pcdata->pvnum )
		{
			send_to_char( "You are not in your castle!\n\r", ch );
			return;
		}

		/* Make sure player can afford it */

		disc_cost=( bargain * COST_OF_DISCONNECT )/100*1000;
		if( ch->gold<disc_cost )
		{
			sprintf( buf,"It costs $%d to disconnect a room.\n\r", disc_cost );
			send_to_char( buf, ch );
			return;
		}

		/* Verifying direction argument */

		if( !get_bitvector_value( argument, &door, "DIR_" ) )
		{
			send_to_char( "You must specify a direction to disconnect!\n\r", ch );
			send_to_char( "Use: castle disconnect <direction>\n\r", ch );
			send_to_char( "Where <direction> can be one of:\n\r", ch );
			list_bitvectors( ch,"DIR_" );
			return;
		}

		/* Verifying that there is an exit in specified direction */

		if ( ch->in_room->exit[door] == NULL )
		{
			send_to_char( "There is no exit in that direction!\n\r", ch );
			return;
		}

		/* Determine connection type */

		if ( ch->pcdata->clan != NULL && ch->pcdata->clan &&
			!strcasecmp( ch->name, ch->pcdata->clan->leader ) )
			clanfounder=TRUE;
		connected_room = ch->in_room->exit[door]->to_room;
		if ( connected_room->creator_pvnum != ch->pcdata->pvnum )
			backdoor=TRUE;

		/* Delete exit... */

		ch->in_room->exit[door] = NULL;

		/* Spam to player */

		act( "Builders come, take a break, and leave.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "Disconnection completed.\n\r", ch );

		/* Adjustment for backdoor removal */

		if ( backdoor )
		{
			if ( clanfounder )
			{
				if ( ch->pcdata->clan->num_backdoors > 0 )
					ch->pcdata->clan->num_backdoors--;
			}
			else
				castle->has_backdoor=FALSE;
		}

		/* Modify castle and character data */

		ch->gold-=disc_cost;
		castle->cost+=disc_cost/1000;
		castle_needs_saving=TRUE;

		return;
	}


	if ( !strcasecmp( arg, "rstat" ) )
	{
		int stat_cost;
		/* make sure they can afford it */

		stat_cost=(bargain*COST_OF_STAT)/100*1000;
		if(ch->gold<stat_cost)
		{
			sprintf(buf,"It will cost you $%d to do an rstat.\n\r",stat_cost);
			send_to_char(buf,ch);
			return;
		}
		if(do_castle_rstat(ch,argument))
		{
			/* adjust cash situation */
			ch->gold-=stat_cost;
			castle->cost+=stat_cost/1000;
		}
		return;
	}
	else if ( !strcasecmp( arg, "mstat" ) )
	{
		send_to_char("Sorry! That feature is not yet available.\n\r",ch);
		return;

		int stat_cost;
		/* make sure they can afford it */

		stat_cost=(bargain*COST_OF_STAT)/100*1000;
		if(ch->gold<stat_cost)
		{
			sprintf(buf,"It will cost you $%d to do an mstat.\n\r",stat_cost);
			send_to_char(buf,ch);
			return;
		}
		if(do_castle_mstat(ch,argument))
		{
			/* adjust cash situation */
			ch->gold-=stat_cost;
			castle->cost+=stat_cost/1000;
		}
		return;
	}
	else if ( !strcasecmp( arg, "ostat" ) )
	{
		send_to_char("Sorry! That feature is not yet available.\n\r",ch);
		return;

		int stat_cost;
		/* make sure they can afford it */

		stat_cost=(bargain*COST_OF_STAT)/100*1000;
		if(ch->gold<stat_cost)
		{
			sprintf(buf,"It will cost you $%d to do an ostat.\n\r",stat_cost);
			send_to_char(buf,ch);
			return;
		}
		if(do_castle_ostat(ch,argument))
		{
			/* adjust cash situation */
			ch->gold-=stat_cost;
			castle->cost+=stat_cost/1000;
		}
		return;
	}
	else if ( !strcasecmp( arg, "ocreate" ) )
	{
		send_to_char("Sorry! That feature is not yet available.\n\r",ch);
		return;
	}
	else if ( !strcasecmp( arg, "mset" ) )
	{
		send_to_char("Sorry! That feature is not yet available.\n\r",ch);
		return;

		char arg1 [MAX_INPUT_LENGTH];
		char arg2 [MAX_INPUT_LENGTH];
		char arg3 [MAX_INPUT_LENGTH];
		CHAR_DATA *victim;
		int value,mset_cost;

		if ( !strcasecmp( argument, "tag" ) )
		{
			/* taging doesn't require a victim name, tags all mobiles w/ "mob"
			put in to allow refering to mobiles that you forgot the name of */

			for(victim=ch->in_room->first_person;victim;victim=victim->next_in_room)

				if(IS_NPC(victim)&&victim->pIndexData->creator_pvnum==ch->pcdata->pvnum)
				{
					strcpy(arg2,victim->pIndexData->player_name);
					strcat(arg2," mob ");
					if(victim->name!=victim->pIndexData->player_name)
						STRFREE( victim->name );
					STRFREE( victim->pIndexData->player_name );
					victim->pIndexData->player_name=str_dup(arg2);
					victim->name = str_dup(arg2);
				}
				send_to_char("You can now refer to your mobiles in this room as 'mob', '2.mob', etc.\n\r",ch);
				return;
		}

		smash_tilde( argument );
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );

		if ( arg1[0] == '\0' || arg2[0] == '\0' )
		{
			send_to_char( "Syntax: castle mset <victim> <field> <value>\n\r",ch );
			send_to_char( " or: castle mset <victim> <string> <value>\n\r",ch );
			send_to_char( "\n\r", ch );
			send_to_char( "Field being one of:\n\r", ch );
			send_to_char( " sex level race act tag\n\r", ch );
			send_to_char( "\n\r", ch );
			send_to_char( "String being one of:\n\r", ch );
			send_to_char( " name short long desc\n\r", ch );
			return;
		}

		/* determine cost */
		mset_cost=COST_OF_SET;
		mset_cost=(bargain*mset_cost)/100*1000;

		/* determine if they can afford it */
		if(ch->gold<mset_cost)
		{
			sprintf(buf,"It will cost you $%d to do an mset.\n\r",mset_cost);
			send_to_char(buf,ch);
			return;
		}

		if ( ( victim = get_char_room( ch, arg1 ) ) == NULL ||
			!IS_NPC(victim) ||
			victim->pIndexData->creator_pvnum!=ch->pcdata->pvnum)
		{
			sprintf(arg2, "You can't work on '%s' here!\n\r", arg1);
			send_to_char(arg2, ch );
			return;
		}

		/*
		* Set something.
		*/
		if ( !strcasecmp( arg2, "sex" ) )
		{
			if(!get_bitvector_value(arg3,&value,"SEX_"))
			{
				send_to_char("You must specify a sex for it!\n\r",ch);
				list_bitvectors(ch,"SEX_");
				return;
			}
			victim->sex = value;
		}
		else if(!strcasecmp(arg2,"act"))
		{
			if(!get_bitvector_value(arg3,&value,"CACT_"))
			{
				send_to_char("You must specify an act flag to toggle!\n\r",ch);
				list_bitvectors(ch,"CACT_");
				return;
			}
			if(IS_SET(victim->act,value))
				REMOVE_BIT(victim->act,value);
			else
				SET_BIT(victim->act,value);
			victim->pIndexData->act=victim->act;
		}
		else if ( !strcasecmp( arg2, "race" ) )
		{
			if(!get_bitvector_value(arg3,&value,"RACE_"))
			{
				send_to_char("You must specify a race for it!\n\r",ch);
				list_bitvectors(ch,"RACE_");
				return;
			}
			victim->race = value;
			victim->language = victim->race;
			SET_BIT(victim->act,ACT_RACE);
			victim->pIndexData->race=victim->race;
			victim->pIndexData->act=victim->act;
		}
		else if ( !strcasecmp( arg2, "level" ) )
		{
			value = is_number( arg3 ) ? atol( arg3 ) : -1;
			if ( value < 0 || value > (ch->level/2) )
			{
				send_to_char( "Level range is 0 to LEVEL/2.\n\r", ch );
				return;
			}
			victim->level = value;
			victim->max_hit = victim->level*victim->level+victim->level+10;
			victim->hit=victim->max_hit;
			victim->pIndexData->damplus = victim->level+5;
			victim->pIndexData->level=victim->level;
			victim->pIndexData->hitplus=victim->max_hit;
		}
		else if ( !strcasecmp( arg2, "name" ) )
		{
			if(victim->name!=victim->pIndexData->player_name)
				STRFREE( victim->name );
			STRFREE( victim->pIndexData->player_name );
			victim->pIndexData->player_name=str_dup(arg3);
			victim->name = str_dup(arg3);
		}
		else if ( !strcasecmp( arg2, "short" ) )
		{
			if(victim->short_descr!=victim->pIndexData->short_descr)
				STRFREE( victim->short_descr );
			STRFREE( victim->pIndexData->short_descr );
			victim->pIndexData->short_descr=str_dup(arg3);
			victim->short_descr = str_dup(arg3);
		}
		else if ( !strcasecmp( arg2, "long" ) )
		{
			if(victim->long_descr!=victim->pIndexData->long_descr)
				STRFREE( victim->long_descr );
			STRFREE( victim->pIndexData->long_descr );
			victim->pIndexData->long_descr=str_dup(justify(arg3));
			victim->long_descr = str_dup(justify(arg3));
		}
		else if ( !strcasecmp( arg2, "desc" ) )
		{
			if(victim->description!=victim->pIndexData->description)
				STRFREE( victim->description );
			STRFREE( victim->pIndexData->description );
			victim->pIndexData->description = str_dup( justify(arg3) );
			victim->description = str_dup( justify(arg3) );
		}
		else if ( !strcasecmp( arg2, "tag" ) )
		{
			if(victim->name!=victim->pIndexData->player_name)
				STRFREE( victim->name );
			STRFREE( victim->pIndexData->player_name );
			victim->pIndexData->player_name=str_dup(arg3);
			victim->name = str_dup(arg3);
		}
		else
		{
			send_to_char("mset what? Use help CASTLE for a list of mset options.\n\r", ch);
			return;
		}

		/* report the modification */
		act("Your eyes go fuzzy for a moment.",ch,NULL,NULL,TO_ROOM);
		act("The contractor comes, takes your money, looks around, pinches the mobile, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);

		/* affect changes due to modifications */
		ch->gold-=mset_cost;
		castle->cost+=mset_cost/1000;
		castle_needs_saving=TRUE;
	}
	else if ( !strcasecmp( arg, "oset" ) )
	{
		send_to_char("Sorry! That feature is not yet available.\n\r",ch);
		return;
	}
	else if ( !strcasecmp( arg, "rset" ) )
	{
		int rset_cost;
		char arg1[MAX_INPUT_LENGTH];
		char arg2[MAX_INPUT_LENGTH];

		smash_tilde( argument );
		argument = one_argument( argument, arg1 );
		if(arg1==NULL || arg1[0]=='\0')
		{
			send_to_char("rset what? Use help CASTLE for a list of rset options.\n\r", ch);
			return;
		}

		/* determine cost */
		/* Made banks more expensive than other rsets and added
		custom error message for bank creation-- Merzik 8-25-00 */

		if ( !strcasecmp(argument,"croom_bank") )
		{
			rset_cost=COST_OF_BANK;
			rset_cost=(bargain*rset_cost)/100*1000;

			/* determine if they can afford it */
			if(ch->gold<rset_cost)
			{
				sprintf(buf,"It will cost you $%d to create a bank.\n\r",rset_cost);
				send_to_char(buf,ch);
				return;
			}
		}
		else
		{
			rset_cost=COST_OF_SET;
			rset_cost=(bargain*rset_cost)/100*1000;

			/* determine if they can afford it */
			if(ch->gold<rset_cost)
			{
				sprintf(buf,"It will cost you $%d to do an rset.\n\r",rset_cost);
				send_to_char(buf,ch);
				return;
			}
		}

		/* make the modification */
		if(!strcasecmp(arg1,"name"))
		{
			STRFREE( ch->in_room->name );
			ch->in_room->name = str_dup( argument );
		}
		else if(!strcasecmp(arg1,"desc"))
		{
			STRFREE( ch->in_room->description );
			if(argument[0]=='\'' || argument[0]=='"')
			{
				argument=one_argument_nolower(argument,arg1);
				ch->in_room->description = str_dup(justify(arg1));
			}
			else
				ch->in_room->description = str_dup(justify( argument ));
		}
		else if(!strcasecmp(arg1,"desc+"))
		{
			strcpy(bigBuf,ch->in_room->description);
			strcat(bigBuf,"\n\r");
			if(argument[0]=='\'' || argument[0]=='"')
			{
				argument=one_argument_nolower(argument,arg1);
				strcat(bigBuf,arg1);
			}
			else
				strcat(bigBuf,argument);
			STRFREE( ch->in_room->description );
			ch->in_room->description = str_dup(bigBuf);
		}
		else if(!strcasecmp(arg1,"flag"))
		{
			if(!get_bitvector_value(argument,&argn,"CROOM_"))
			{
				send_to_char("You must specify a flag to toggle!\n\r",ch);
				list_bitvectors(ch,"CROOM_");
				return;
			}

			/* Banks can only be created by clan founder -- Merzik 8-25-00 */

			clanbank = FALSE;

			if ( !strcasecmp(argument,"croom_bank") )
			{
				CLAN_DATA *clan;
				clan = ch->pcdata->clan;
				if ( IS_NPC( ch ) || !ch->pcdata->clan )
				{
					send_to_char( "You are not in a clan!\n\r", ch );
					return;
				}
				if ( strcasecmp( ch->name, clan->leader ) )
				{
					send_to_char( "You are not a clan founder.\n\r", ch );
					return;
				}
				clanbank = TRUE;
			}

			if(IS_SET(ch->in_room->room_flags,argn))
			{
				REMOVE_BIT(ch->in_room->room_flags,argn);
				if ( clanbank )
				{
					CLAN_DATA *clan;
					clan = ch->pcdata->clan;
					clan->bank -= 1;
				}
			}
			else
			{
				SET_BIT(ch->in_room->room_flags,argn);
				if ( clanbank )
				{
					CLAN_DATA *clan;
					clan = ch->pcdata->clan;
					clan->bank += 1;
				}
			}

		}
		else if(!strcasecmp(arg1,"sector"))
		{
			if(!get_bitvector_value(argument,&argn,"SECT_"))
			{
				send_to_char("You must specify a new sector type!\n\r",ch);
				list_bitvectors(ch,"SECT_");
				return;
			}
			ch->in_room->sector_type=argn;
		}
		else if(!strcasecmp(arg1,"door"))
		{
			EXIT_DATA *pexit;
			sh_int door_dir;

			argument = one_argument( argument, arg1 );
			if(!get_bitvector_value(arg1,&argn,"DIR_"))
			{
				send_to_char("You must specify the direction of the door to modify:\n\rUSE: castle rset door DIR doorField newData\n\r",ch);
				list_bitvectors(ch,"DIR_");
				return;
			}
			door_dir=argn;

			/* make sure the door is valid, if not, create one */

			if ( (pexit = ch->in_room->exit[door_dir] ) == NULL )
			{
				pexit = alloc_mem( sizeof(*pexit) );
				pexit->description = str_dup("");
				pexit->keyword = str_dup("");
				pexit->exit_info = 0;
				pexit->pvnum = ch->pcdata->pvnum;
				pexit->key = -1;
				pexit->vnum = ch->in_room->vnum;
				pexit->to_room = NULL;
				ch->in_room->exit[door_dir] = pexit;
				top_exit++;
			}

			/* parse the options */
			argument = one_argument( argument, arg1 );
			if(!strcasecmp(arg1,"keyword"))
			{
				STRFREE( pexit->keyword );
				pexit->keyword = str_dup( argument );

				if((pexit->to_room!=NULL)&&
					(pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
				{
					/* set the opposite side of the door the same way if it's outside */
					EXIT_DATA *pexit2;

					pexit2=pexit->to_room->exit[rev_dir[door_dir]];
					if(pexit2!=NULL)
					{
						if(pexit2->keyword!=NULL)
							STRFREE(pexit2->keyword);
						pexit2->keyword = str_dup( pexit->keyword );
					}
				}
			}
			else if(!strcasecmp(arg1,"del"))
			{
				if((pexit->to_room!=NULL)&&
					(pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
				{
					send_to_char("That's your entrance! You can't delete that!\n\r",ch);
					return;
				}
				ch->in_room->exit[door_dir]=NULL;
				STRFREE(pexit->description);
				STRFREE(pexit->keyword);
				free_mem(&pexit,sizeof(*pexit));
				top_exit--;
				rset_cost/=100;
			}
			else if(!strcasecmp(arg1,"desc"))
			{
				STRFREE( pexit->description);
				if(argument[0]=='\'' || argument[0]=='"')
				{
					argument=one_argument_nolower(argument,arg1);
					pexit->description = str_dup(justify(arg1));
				}
				else
					pexit->description = str_dup(justify(argument));
				if((pexit->to_room!=NULL)&&
					(pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
				{
					/* set the opposite side of the door the same way if it's outside */
					EXIT_DATA *pexit2;

					pexit2=pexit->to_room->exit[rev_dir[door_dir]];
					if(pexit2!=NULL)
					{
						if(pexit2->description!=NULL)
							STRFREE(pexit2->description);
						pexit2->description = str_dup( pexit->description );
					}
				}
			}
			else if(!strcasecmp(arg1,"flag"))
			{
				if(!get_bitvector_value(argument,&argn,"CEX_"))
				{
					send_to_char("You must specify a flag to toggle!\n\r",ch);
					list_bitvectors(ch,"CEX_");
					return;
				}
				if(IS_SET(pexit->exit_info,argn))
					REMOVE_BIT(pexit->exit_info,argn);
				else
					SET_BIT(pexit->exit_info,argn);
				if((pexit->to_room!=NULL)&&
					(pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
				{
					/* set the opposite side of the door the same way if it's outside */
					EXIT_DATA *pexit2;

					pexit2=pexit->to_room->exit[rev_dir[door_dir]];
					if(pexit2!=NULL)
					{
						if(!IS_SET(pexit->exit_info,argn))
							REMOVE_BIT(pexit2->exit_info,argn);
						else
							SET_BIT(pexit2->exit_info,argn);
					}
				}
			}
			else if(!strcasecmp(arg1,"key"))
			{
				OBJ_DATA *obj;

				if((obj=get_obj_carry(ch,argument))==NULL)
				{
					send_to_char("You must be holding the object you wish to use as a key!\n\r",ch);
					return;
				}
				pexit->key=obj->pIndexData->vnum;
				if((pexit->to_room!=NULL)&&
					(pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
				{
					/* set the opposite side of the door the same way if it's outside */
					EXIT_DATA *pexit2;

					pexit2=pexit->to_room->exit[rev_dir[door_dir]];
					if(pexit2!=NULL)
						pexit2->key = pexit->key;
				}
			}
			else
			{
				send_to_char("Castle rset: change what part of that door?\n\rUse help CASTLE for a list of rset options.\n\r",ch);
				return;
			}
		}
		else if(!strcasecmp(arg1,"extra"))
		{
			EXTRA_DESCR_DATA *ed, *prev;
			char *desc;

			argument = one_argument( argument, arg1 );

			if(!strcasecmp(arg1,"keyword"))
			{
				desc=str_dup("");
				strcpy(arg2,argument);
				argument = one_argument( argument, arg1 );
			}
			else if(!strcasecmp(arg1,"desc"))
			{
				argument = one_argument( argument, arg1 );
				strcpy(arg2,arg1);
				desc = str_dup(justify(argument));
			}
			else if(!strcasecmp(arg1,"del"))
			{
				argument = one_argument( argument, arg1 );

				/* find the one to delete, keep track of previous */
				for ( prev=NULL, ed = ch->in_room->first_extradesc;
					ed; prev=ed,ed = ed->next )
					if(is_name(arg1,ed->keyword))
						break;

				if(ed!=NULL)
				{
					sprintf(arg2,"Deleted the following extras: '%s'\n\r",ed->keyword);
					if(prev==NULL)
						ch->in_room->first_extradesc=ed->next;
					else
						prev->next=ed->next;
					STRFREE(ed->description);
					STRFREE(ed->keyword);
					free_mem(&ed,sizeof(*ed));

					send_to_char(arg2,ch);

					rset_cost/=100;
					ch->gold-=rset_cost;
					castle->cost+=rset_cost/1000;
					castle_needs_saving=TRUE;
					return;
				}
				else
				{
					sprintf(arg2,"Couldn't find an extra named '%s' to delete.\n\r",arg1);
					send_to_char(arg2,ch);
					return;
				}
			}
			else
			{
				send_to_char("Castle rset: which extra? Use help CASTLE for a list of rset options.\n\r",ch);
				return;
			}

			for ( ed = ch->in_room->first_extradesc; ed; ed = ed->next )
				if(is_name(arg1,ed->keyword))
					break;

			if ( ed == NULL )
			{
				ed=alloc_mem(sizeof(*ed));
				ed->description=str_dup("");

				ed->next=ch->in_room->first_extradesc;
				ch->in_room->first_extradesc=ed;

				ed->keyword=str_dup(arg2);
			}
			if(ed->description!=NULL)
				STRFREE(ed->description);
			ed->description=desc;
		}
		else
		{
			send_to_char("rset what? Use help CASTLE for a list of rset options.\n\r",ch);
			return;
		}

		/* report the modification */

		act("The room appears to flicker for an instant.",ch,NULL,NULL,TO_ROOM);
		act("The contractor comes, takes your money, looks around, hammers on a post, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);

		/* affect changes due to modifications */

		ch->gold-=rset_cost;
		castle->cost+=rset_cost/1000;
		castle_needs_saving=TRUE;
	}
	else
	{
		send_to_char("What Castle command were you wanting to do? (see: HELP CASTLE)\n\r",ch);
		return;
	}
	return;
}

void del_castle( CHAR_DATA *victim)
{
	int vnum;
	ROOM_INDEX_DATA *pRoomIndex;
	MOB_INDEX_DATA *pMobIndex;

	if(victim->pcdata->castle==NULL)
		return;

	for(vnum=20000;vnum<32000;vnum++)
	{
		if( ( (pRoomIndex=room_index[vnum]) !=NULL)
			&& victim->pcdata->pvnum==pRoomIndex->creator_pvnum
			&& IS_SET(pRoomIndex->room_flags,ROOM_IS_CASTLE))
		{
			delete_room(pRoomIndex);
		}
		if( ( (pMobIndex=mob_index[vnum]) !=NULL)
			&& victim->pcdata->pvnum==pMobIndex->creator_pvnum )
		{
			delete_mob(pMobIndex);
		}

	}
	/* update players castle structure */
	victim->pcdata->castle->entrance=0;
	victim->pcdata->castle->door_room=-1;
	victim->pcdata->castle->door_dir=-1;
	castle_needs_saving=TRUE;
	victim->pcdata->castle->has_backdoor=FALSE;
	victim->pcdata->castle->cost=0;
	victim->pcdata->castle->num_rooms=0;
	victim->pcdata->castle->num_mobiles=0;
	victim->pcdata->castle->num_objects=0;
	victim->pcdata->castle->reign_room=0;
	victim->pcdata->castle=NULL;
}

void clear_castles(void)
{
	return;
}
