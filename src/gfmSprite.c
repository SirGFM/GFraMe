/**
 * @file src/gfmSprite.c
 * 
 * Represent a retangular 'object' that can be rendered to the screen; It has
 * its own gfmObject to handle the physics
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

#include <stdlib.h>
#include <string.h>

/** The gfmSprite structure */
struct stGFMSprite {
    /** The sprite's physical object */
    gfmObject *pObject;
    /** The sprite's 'child-class' (e.g., a player struct) */
    void *pChild;
    /** The sprite child's type */
    int childType;
    /** Horizontal offset between the object's top-left corner and the tile's */
    int offsetX;
    /** Vertical offset between the object's top-left corner and the tile's */
    int offsetY;
    // TODO Animation...
};

/** Size of gfmSprite */
const int sizeofGFMSprite = (int)sizeof(gfmSprite);

