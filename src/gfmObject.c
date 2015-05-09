/**
 * @file include/GFraMe/gfmObject.h
 * 
 * Basic physical object; It has an AABB, position, velocity, acceleration, drag
 * and current (and previous frame) collision info;
 * Since this is the base type to be passed to the quadtree for
 * overlaping/collision, it also has info about it's "child type" (e.g., a
 * gfmSprite pointer and the type T_GFMSPRITE)
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>

/** The gfmObject structure */
struct stGFMObject {
    int x;
    int y;
    double dx;
    double dy;
    double ldx;
    double ldy;
    double vx;
    double vy;
    double ax;
    double ay;
    double dragX;
    double dragY;
    double halfWidth;
    double halfHeight;
    int type;
    void *pChild;
};
/** Size of gfmObject */
const int sizeofGFMObject = (int)sizeof(gfmObject);

