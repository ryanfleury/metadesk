// V1
typedef enum
{
    v1_EntryKind_A,
    v1_EntryKind_B,
    v1_EntryKind_C,
    v1_EntryKind_E,
} v1_EntryKind;

typedef enum
{
    v1_EntryFlag_A = (1<<0),
    v1_EntryFlag_B = (1<<1),
    v1_EntryFlag_C = (1<<2),
} v1_EntryFlags;

typedef union v1_Color v1_Color;
union v1_Color
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } components;
    uint8_t raw[4];
};

typedef struct v1_Entry v1_Entry;
struct v1_Entry
{
    uint16_t to_remove;
    v1_EntryKind kind;
    v1_EntryFlags flags;
    v1_Color color;
    struct
    {
        float x;
        float y;
    } p;
};

// V2
typedef enum
{
    EntryKind_A,
    EntryKind_B,
    EntryKind_B2,
    EntryKind_C,
} EntryKind;

typedef enum
{
    EntryFlag_B = (1<<0),
    EntryFlag_C = (1<<1),
    EntryFlag_D = (1<<2),
} EntryFlags;

typedef union Color Color;
union Color
{
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    } components;
    uint8_t raw[4];
};

typedef struct Entry Entry;
struct Entry
{
    EntryKind kind;
    Color color;
    struct
    {
        float x;
        float y;
        float z;
    } p;
    EntryFlags flags;
};

// V1->V2
static EntryKind EntryKindFromV1(v1_EntryKind v)
{
    EntryKind result = 0;
    switch(v)
    {
        case v1_EntryKind_A: result = EntryKind_A; break;
        case v1_EntryKind_B: result = EntryKind_B; break;
        case v1_EntryKind_C: result = EntryKind_C; break;
        case v1_EntryKind_E: assert(!"Enumerand v1_EntryKind_E is no longer allowed\n");
        default: assert(!"Illegal value for enum v1_EntryKind\n"); break;
    }
    return result;
}

static EntryFlags EntryFlagsFromV1(v1_EntryFlags v)
{
    EntryFlags result = 0;
    if(v & v1_EntryFlag_A) assert(!"Flag v1_A is no longer allowed\n");
    if(v & v1_EntryFlag_B) result |= EntryFlag_B;
    if(v & v1_EntryFlag_C) result |= EntryFlag_C;
    return result;
}

static Color ColorFromV1(v1_Color v)
{
    Color result = {0};
    result.components.r = v.components.r;
    result.components.g = v.components.g;
    result.components.b = v.components.b;
    result.components.a = v.components.a;
    return result;
}

static Entry EntryFromV1(v1_Entry v)
{
    Entry result = {0};
    result.kind = EntryKindFromV1(v.kind);
    result.color = ColorFromV1(v.color);
    result.p.x = v.p.x;
    result.p.y = v.p.y;
    result.flags = EntryFlagsFromV1(v.flags);
    return result;
}

