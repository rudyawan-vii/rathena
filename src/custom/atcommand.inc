// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

static int count_mob(struct block_list *bl, va_list ap) // [FE]
{
    struct mob_data *md = (struct mob_data*)bl;
    // va_arg promotes types to int, so we catch it as int
    int id = va_arg(ap, int); 
    
    // class_ was renamed to mob_id in modern rAthena
    if (md->mob_id == id) 
        return 1;
    return 0;
}

/*==========================================
 * @mapmoblist 
 * added by Pudica, updated for modern rA
 *------------------------------------------*/
ACMD_FUNC(mapmoblist) // [FE]
{
    char temp[100];
    
    // Using std::set instead of fixed arrays (removes the need for MAX_MOB_DB)
    std::set<int> mob_searched;
    std::set<int> mob_mvp;
    
    struct s_mapiterator* it;
    unsigned short count = 0, mapindex = 0;
    int m = 0;

    if (message && *message) {
        // Player input map name, search mob list for that map
        mapindex = mapindex_name2id(message);
        if (!mapindex) {
            clif_displaymessage(fd, "Map not found");
            return -1;
        }
        m = map_mapindex2mapid(mapindex);
    } else {
        // Player doesn't input map name, search mob list in player current map
        mapindex = sd->mapindex;
        m = sd->m; // sd->bl.m is now sd->m (C++ Inheritance)
    }

    clif_displaymessage(fd, "--------Monster List--------");
    sprintf(temp, "Mapname: %s", mapindex_id2name(mapindex));
    clif_displaymessage(fd, temp);

    clif_displaymessage(fd, "Monsters: ");

    // Looping and searching for mobs
    it = mapit_geteachmob();
    while (true) {
        TBL_MOB* md = (TBL_MOB*)mapit_next(it);
        if (md == NULL)
            break;

        if (md->m != m || md->status.hp <= 0) // md->bl.m is now md->m
            continue;
            
        int mob_id = md->mob_id;

        // Check if we've already logged this mob
        if (mob_searched.find(mob_id) != mob_searched.end())
            continue; 

        // mob_db() macro is now a class method mob_db.find()
        auto db_info = mob_db.find(mob_id); 
        if (!db_info)
            continue;

        if (db_info->mexp > 0) {
            mob_searched.insert(mob_id);
            mob_mvp.insert(mob_id); // Save id for the MVP loop later
            continue; 
        }

        mob_searched.insert(mob_id);
        count = map_foreachinmap(count_mob, m, BL_MOB, mob_id);

        // Note: db_info->jname is a std::string in modern rA, so we use .c_str()
        sprintf(temp, " %s[%d] : %d", db_info->jname.c_str(), mob_id, count);
        clif_displaymessage(fd, temp);
    }
    mapit_free(it);

    clif_displaymessage(fd, "MVP: ");

    // Loop through the dynamically saved MVPs instead of looping up to MAX_MOB_DB
    for (int mvp_id : mob_mvp) {
        auto db_info = mob_db.find(mvp_id);
        if (db_info) {
            count = map_foreachinmap(count_mob, m, BL_MOB, mvp_id);
            sprintf(temp, " %s[%d] : %d", db_info->jname.c_str(), mvp_id, count);
            clif_displaymessage(fd, temp);
        }
    }

    return 0;
}
