#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

#include "generated/wrappedfmoddefs.h"

const char* fmodName = "libfmod.so.14";
#define LIBNAME fmod
#define ALTNAME "libfmod.so.13"
#define ALTNAME2 "libfmod.so"
#include "wrappedlib_init.h"

#include "generated/wrappedfmodtypes.h"

#include "wrappercallback.h"

// Project Zomboid's fmodintegration will try to create DSP for noise cancellation regardless of user settings.
// Creating DSP will set x86 callbacks in arm fmod which is not supported atm, so we need to skip key functions.
// This can also be solved by patching isAecEnabled global var in fmodintegration, but I prefer not to do that

int my_FMOD_System_CreateDSP() {
    printf("FmodWrapper: Skipping FMOD_System_CreateDSP\n");
    return 0; 
}

int my_FMOD_DSP_SetBypass() {
    printf("FmodWrapper: Skipping FMOD_DSP_SetBypass\n");
    return 0;
}

int my_FMOD_ChannelGroup_AddDSP() {
    printf("FmodWrapper: Skipping FMOD_ChannelGroup_AddDSP\n");
    return 0;
}


/* other unimplemented functions that set callbacks */

int my_FMOD_ChannelGroup_SetCallback() {
    printf("FmodWrapper: Unimplemented function FMOD_ChannelGroup_SetCallback called\n");
    return 1;
}

int my_FMOD_Channel_SetCallback() {
    printf("FmodWrapper: Unimplemented function FMOD_Channel_SetCallback called\n");
    return 1;
}

int my_FMOD_Debug_Initialize() {
    printf("FmodWrapper: Unimplemented function FMOD_Debug_Initialize called\n");
    return 1;
}

int my_FMOD_Memory_Initialize() {
    printf("FmodWrapper: Unimplemented function FMOD_Memory_Initialize called\n");
    return 1;
}

int my_FMOD_System_AttachFileSystem() {
    printf("FmodWrapper: Unimplemented function FMOD_System_AttachFileSystem called\n");
    return 1;
}

int my_FMOD_System_Set3DRolloffCallback() {
    printf("FmodWrapper: Unimplemented function FMOD_System_Set3DRolloffCallback called\n");
    return 1;
}

int my_FMOD_System_SetCallback() {
    printf("FmodWrapper: Unimplemented function FMOD_System_SetCallback called\n");
    return 1;
}

int my_FMOD_System_SetFileSystem() {
    printf("FmodWrapper: Unimplemented function FMOD_System_SetFileSystem called\n");
    return 1;
}


// Insert code here

#include "wrappedlib_init.h"
