#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <string.h>
#include "platform/server/os_server.h"
#include "main/main.h"
#include "core/math/face3.h"

// Find the index of the --godot argument. If no argument
// found then returns -1
int get_start_of_godot_args(int argc, char* argv[]) {
    // The first index is reserved for the program name so we can skip it
    for (int i = 1; i < argc; i++) {
        if (strncmp (argv[i],"--godot", 2) == 0) {
            return i;
        }
    }

    // Because 0 is invalid this could also have just been 0, but
    // let's try to keep things a little less confusing
    return -1;
}

// Allocates and fills a new array of char pointers that points to godot arguments
char** make_godot_args(int godot_args_length, int start_of_godot_args, char* argv[]) {
    // We shouldn't need to worry too much about memory management here
    char** godot_args = new char*[godot_args_length];

    // We don't want the --godot arg but we do want the program name
    godot_args[0] = argv[0];

    for (int i = 1; i < godot_args_length; i++) {
        godot_args[i] = argv[start_of_godot_args + i];
    }

    return godot_args;
}

int main( int argc, char* argv[] ) {
    // Allow passing in commands into both Catch and the Godot server
    // by splitting on a "--godot" arg
    int start_of_godot_args = get_start_of_godot_args(argc, argv);
    int godot_args_length = start_of_godot_args != -1 ? argc - start_of_godot_args : 0;
    int catch_args_length = argc - godot_args_length;
    char** godot_args = make_godot_args(godot_args_length, start_of_godot_args, argv);

	OS_Server os;
	Error err = Main::setup(argv[0], godot_args_length, godot_args);
	if (err != OK)
		return 255;

    int result = Catch::Session().run(catch_args_length, argv);
	
    Main::cleanup();
    delete godot_args;

    return result;
}