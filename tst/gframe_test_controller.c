/**
 * @file tst/gframe_test_controller.c
 */

#include <GFraMe/GFraMe.h>
#include <GFraMe/GFraMe_controller.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_screen.h>

int main(int argc, char *argv[]) {
    GFraMe_ret rv;
    int run;
    
    // Initialize the system and create a dummy window
    rv = GFraMe_init(320,                            // Backbuffer width
                     240,                            // Backbuffer height
                     320,                            // Window's width
                     240,                            // Window's height
                     "com.github.sirgfm.gframe.tst", // Pkg name
                     "GFraMeControllerTest",         // App name
                     GFraMe_window_none,             // No flag
                     NULL,                           // No extentions
                     30,                             // App's fps
                     0,                              // Don't log to file
                     0                               // Append or overwrite log
                    );
    GFraMe_assertRV(rv == GFraMe_ret_ok, // check if did initialize
                    "Failed to init",    // Message on error
                    rv = rv,             // I'll fix this someday...
                    __ret                // Label where error is treated
                   );
    
    // Initialize the controller subsystem
    GFraMe_controller_init(0 // Don't connect controllers automatically
                          );
    
    // loop until user wants to exit
    run = 1;
    while (run) {
        // Wait until any event is received
        GFraMe_event_begin();
            // Update the controller(s) state(s)
            GFraMe_event_on_controller();
                GFraMe_controller_printStates();
            // If the application received Ctrl+C or was close by the mouse
            GFraMe_event_on_quit();
                run = 0;
        // Finish updating events
        GFraMe_event_end();
    }
    
__ret:
    GFraMe_controller_close();
    GFraMe_quit();
    return rv;
}

