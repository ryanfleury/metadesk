/////////////////////////////////////////////
//~ NOTE(mal): MD_Map. Same as MD_NodeTable except it stores void * values
typedef enum MD_MapCollisionRule
{
    MD_MapCollisionRule_Chain,
    MD_MapCollisionRule_Overwrite,
}
MD_MapCollisionRule;

typedef struct MD_MapSlot MD_MapSlot;
struct MD_MapSlot
{
    MD_MapSlot *next;
    MD_u64 hash;
    void *value;
};

typedef struct MD_Map MD_Map;
struct MD_Map
{
    MD_u64 table_size;
    MD_MapSlot **table;
};

/////////////////////////////////////////////
//~ NOTE(mal): Helpers
typedef MD_Map MD_StringMap;
typedef MD_Map MD_PtrMap;

// NOTE(mal): MD_NodeTable interface
typedef MD_Map MD_NodeTable;
typedef MD_MapSlot MD_NodeTableSlot;
typedef MD_MapCollisionRule MD_NodeTableCollisionRule;
#define MD_NodeTableCollisionRule_Chain MD_MapCollisionRule_Chain 
#define MD_NodeTableCollisionRule_Overwrite MD_MapCollisionRule_Overwrite 
