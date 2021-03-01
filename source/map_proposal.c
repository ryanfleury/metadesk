/////////////////////////////////////////////
//~ NOTE(mal): MD_Map functions. Same as MD_NodeTable except they ask for hash values to abstract the hash function away
MD_PRIVATE_FUNCTION_IMPL void
_MD_Map_Initialize(MD_Map *map)
{
    if(map->table_size == 0)
    {
        map->table_size = 4096;
        map->table = _MD_PushArray(_MD_GetCtx(), MD_MapSlot *, map->table_size);
    }
}

MD_FUNCTION_IMPL MD_MapSlot *
_MD_Map_Lookup(MD_Map *map, MD_u64 hash)
{
    _MD_Map_Initialize(map);
    
    MD_MapSlot *slot = 0;
    MD_u64 index = hash % map->table_size;
    for(MD_MapSlot *candidate = map->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash)
        {
            slot = candidate;
            break;
        }
    }
    return slot;
}

MD_FUNCTION_IMPL MD_b32
_MD_Map_Insert(MD_StringMap *map, MD_MapCollisionRule collision_rule, MD_u64 hash, void *value)
{
    _MD_Map_Initialize(map);
    
    MD_MapSlot *slot = 0;
    MD_u64 index = hash % map->table_size;
    
    for(MD_MapSlot *candidate = map->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash)
        {
            slot = candidate;
            break;
        }
    }
    
    if(slot == 0 || (slot != 0 && collision_rule == MD_MapCollisionRule_Chain))
    {
        slot = _MD_PushArray(_MD_GetCtx(), MD_MapSlot, 1);
        if(slot)
        {
            slot->next = 0;
            if(map->table[index])
            {
                for(MD_MapSlot *old_slot = map->table[index]; old_slot; old_slot = old_slot->next)
                {
                    if(old_slot->next == 0)
                    {
                        old_slot->next = slot;
                        break;
                    }
                }
            }
            else
            {
                map->table[index] = slot;
            }
        }
    }
    
    if(slot)
    {
        slot->value = value;
        slot->hash = hash;
    }
    
    return !!slot;
}

/////////////////////////////////////////////
//~ NOTE(mal): MD_StringMap
MD_FUNCTION_IMPL MD_MapSlot *
MD_StringMap_Lookup(MD_StringMap *map, MD_String8 string)       // NOTE(mal): Or MD_PtrFromString
{
    MD_MapSlot *slot = _MD_Map_Lookup(map, MD_HashString(string));
    return slot;
}

MD_FUNCTION_IMPL MD_b32
MD_StringMap_Insert(MD_StringMap *map, MD_MapCollisionRule collision_rule, MD_String8 string, void *value)
{
    MD_b32 result = _MD_Map_Insert(map, collision_rule, MD_HashString(string), value);
    return result;
}

// NOTE(mal): Original MD_NodeTable interface
#define MD_NodeTable_Lookup(map, string) MD_StringMap_Lookup(map, string)
#define MD_NodeTable_Insert(map, collision_rule, string, node) MD_StringMap_Insert(map, collision_rule, string, (void *) node)

/////////////////////////////////////////////
//~ NOTE(mal): MD_PtrMap

// NOTE(mal): Generic 64-bit hash function (https://nullprogram.com/blog/2018/07/31/)
//            Assumes all bits of the pointer matter and that there's a b
MD_FUNCTION_IMPL MD_u64 
MD_HashPointer(void *p)
{
    MD_u64 h = (MD_u64)p;
    h = (h ^ (h >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    h = (h ^ (h >> 27)) * UINT64_C(0x94d049bb133111eb);
    h = h ^ (h >> 31);
    return h;
}

MD_FUNCTION_IMPL MD_MapSlot *
MD_PtrMap_Lookup(MD_PtrMap *map, void *key)                     // NOTE(mal): Or MD_PtrFromPtr
{
    MD_MapSlot *slot = _MD_Map_Lookup(map, MD_HashPointer(key));
    return slot;
}

// TODO(mal): Remove collision_rule parameter and assume MD_MapCollisionRule_Overwrite?
MD_FUNCTION_IMPL MD_b32
MD_PtrMap_Insert(MD_PtrMap *map, MD_MapCollisionRule collision_rule, void *key, void *value)
{
    MD_b32 result = _MD_Map_Insert(map, collision_rule, MD_HashPointer(key), value);
    return result;
}

