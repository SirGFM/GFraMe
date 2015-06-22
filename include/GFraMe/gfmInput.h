/**
 * @file src/include/GFraMe_int/gfmInput.h
 * 
 * Generic input interface; Instead of giving access to each possible input
 * device, this maps inputs to actions, easing customizable controls;
 * A drawback to this implementation is that it forces the user to map each key
 * before using them (but that also forces 'em to use it correctly!)
 * 
 * Each action may be either 'pressed' or 'released', but there is a flag to
 * signal the frame when the action switched; Also, actions may be assigned to
 * a controller's button; Be aware that as soon as a controller is
 * disconected, its bindings will be released
 * 
 * Eventually, there'll be a special type of action to get the axis value of a
 * controller; For now, if an axis is used as input it will have a deadzone of
 * 30% of it's full motion
 */
#ifndef __GFMINPUT_STRUCT__
#define __GFMINPUT_STRUCT__

typedef struct enGFMInput gfmInput;

enum enGFMInputStatus {
    gfmInput_released     = 0x01,
    gfmInput_pressed      = 0x02,
    gfmInput_justAction   = 0x04,
    gfmInput_justPressed  = gfmInput_released | gfmInput_justAction,
    gfmInput_justReleased = gfmInput_pressed | gfmInput_justAction,
};
typedef enum enGFMInputStatus gfmInputStatus;

#endif /* __GFMINPUT_STRUCT__ */

#ifndef __GFMINPUT_H__
#define __GFMINPUT_H__

#include <GFraMe/gfmError.h>

gfmRV gfmInput_getNew(gfmInput **ppCtx);
gfmRV gfmInput_free(gfmInput **ppCtx);

gfmRV gfmInput_addAction(int *pHandle, gfmInput *pCtx);
gfmRV gfmInput_remAction(gfmInput *pCtx, int handle);
gfmRV gfmInput_getAction(gfmInputStatus *pStatus, gfmInput *pCtx, int handle);

gfmRV gfmInput_bindKey(gfmInput *pCtx, int handle, gfmInputKey key);
gfmRV gfmInput_unbindKey(gfmInput *pCtx, int handle, gfmInputKey key);

gfmRV gfmInput_bindMultiKey(gfmInput *pCtx, int handle, gfmInputKey key);
gfmRV gfmInput_unbindMultiKey(gfmInput *pCtx, int handle, gfmInputKey key);

gfmRV gfmInput_bindPointer(gfmInput *pCtx, int handle);
gfmRV gfmInput_unbindPointer(gfmInput *pCtx, int handle);

gfmRV gfmInput_bindMultiPointer(gfmInput *pCtx, int handle, int num);
gfmRV gfmInput_unbindMultiPointer(gfmInput *pCtx, int handle, int num);

gfmRV gfmInput_setPointerPosition(gfmInput *pCtx, int x, int y);
gfmRV gfmInput_getPointerPosition(int *pX, int *pY, gfmInput *pCtx);

gfmRV gfmInput_getNumControllers(int *pNum, gfmInput *pCtx);
gfmRV gfmInput_updateControllers(gfmInput *pCtx);

gfmRV gfmInput_bindController(gfmInput *pCtx, int handle, gfmInputController bt,
        int controller);
gfmRV gfmInput_unbindController(gfmInput *pCtx, int handle,
        gfmInputController bt, int controller);

gfmRV gfmInput_bindMultiController(gfmInput *pCtx, int handle,
        gfmInputController bt, int controller);
gfmRV gfmInput_unbindMultiController(gfmInput *pCtx, int handle,
        gfmInputController bt, int controller);

#endif /* __GFMINPUT_H__ */

