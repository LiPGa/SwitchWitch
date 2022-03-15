//
//  main.cpp
//  SwitchWitch
//
//  Based on: Geometry Lab
//  Author: Walker White
//  Version: 1/20/22

#include "SWApp.h"

using namespace cugl;

/**
 * The main entry point of any CUGL application.
 *
 * This class creates the application and runs it until done.  You may
 * need to tailor this to your application, particularly the application
 * settings.  However, never modify anything below the line marked.
 *
 * @return the exit status of the application
 */
int main(int argc, char * argv[]) {
    // Change this to your application class
    SwitchWitchApp app;
    
    // Set the properties of your application
    app.setName("Switch Witch");
    app.setOrganization("GDIAC");
    app.setHighDPI(true);
    //app.setFPS(60.0f);

    // VARY THIS TO TRY OUT YOUR SCENE GRAPH
    // Pixel Dimensions of Background Divided by 4.
    // Default Aspect Ratio 1125 x 2436
    app.setDisplaySize(332, 720); // 16x9,  Android phones, PC Gaming

    /// DO NOT MODIFY ANYTHING BELOW THIS LINE
    if (!app.init()) {
        return 1;
    }
    
    app.onStartup();
    while (app.step());
    app.onShutdown();

    exit(0);    // Necessary to quit on mobile devices
    return 0;   // This line is never reached
}
