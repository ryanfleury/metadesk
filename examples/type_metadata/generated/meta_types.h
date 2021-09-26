#if !defined(META_TYEPS_H)
#define META_TYEPS_H
typedef struct Circle Circle;
struct Circle
{
F32 r;
V2F32 pos;
};
typedef struct RoundedSegment RoundedSegment;
struct RoundedSegment
{
F32 r;
V2F32 p1;
V2F32 p2;
};
typedef struct RoundedPolygon RoundedPolygon;
struct RoundedPolygon
{
F32 r;
U32 count;
V2F32 *p;
};
typedef enum Shape
{
Shape_Circle = 1,
Shape_Segment = 2,
Shape_Polygon = 3,
} Shape;
TypeInfo* type_info_from_shape(Shape v);
U32 max_slot_from_shape(Shape v);
#endif // META_TYEPS_H
