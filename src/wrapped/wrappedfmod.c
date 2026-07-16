#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian/library_private.h"
#include "x64emu.h"

#include "generated/wrappedfmoddefs.h"

const char* fmodName = "libfmod.so.14";
#define LIBNAME fmod
#define ALTNAME "libfmod.so.13"
#define ALTNAME2 "libfmod.so"

#include "generated/wrappedfmodtypes.h"

#include "wrappercallback.h"

/* ---------------------------------------------------------------------------
 * Sound callback bridges.
 *
 * FMOD_System_CreateSound/CreateStream take a FMOD_CREATESOUNDEXINFO that can carry
 * callbacks. Project Zomboid's VOIP uses this: the sound for INCOMING voice is created
 * with a pcmreadcallback that fmod's mixer thread calls to pull voice PCM. Those pointers
 * are x86 code; handing them to the native arm fmod makes it jump straight into x86 bytes
 * -> SIGILL. So we swap each one for a native trampoline that re-enters the emulator.
 * ------------------------------------------------------------------------- */

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// pcmreadcallback: FMOD_RESULT (*)(FMOD_SOUND* sound, void* data, unsigned int datalen)
#define GO(A)   \
static uintptr_t my_pcmread_fct_##A = 0;                                                \
static uint32_t my_pcmread_##A(void* sound, void* data, uint32_t datalen)               \
{                                                                                       \
    return (uint32_t)RunFunctionFmt(my_pcmread_fct_##A, "ppu", sound, data, datalen);   \
}
SUPER()
#undef GO
static void* find_pcmread_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_pcmread_fct_##A == (uintptr_t)fct) return my_pcmread_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pcmread_fct_##A == 0) {my_pcmread_fct_##A = (uintptr_t)fct; return my_pcmread_##A;}
    SUPER()
    #undef GO
    printf("FmodWrapper: Warning, no more slot for fmod pcmread callback\n");
    return NULL;
}

// pcmsetposcallback: FMOD_RESULT (*)(FMOD_SOUND* sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
#define GO(A)   \
static uintptr_t my_pcmsetpos_fct_##A = 0;                                                          \
static uint32_t my_pcmsetpos_##A(void* sound, int32_t subsound, uint32_t position, uint32_t postype)\
{                                                                                                   \
    return (uint32_t)RunFunctionFmt(my_pcmsetpos_fct_##A, "piuu", sound, subsound, position, postype); \
}
SUPER()
#undef GO
static void* find_pcmsetpos_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_pcmsetpos_fct_##A == (uintptr_t)fct) return my_pcmsetpos_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pcmsetpos_fct_##A == 0) {my_pcmsetpos_fct_##A = (uintptr_t)fct; return my_pcmsetpos_##A;}
    SUPER()
    #undef GO
    printf("FmodWrapper: Warning, no more slot for fmod pcmsetpos callback\n");
    return NULL;
}

// nonblockcallback: FMOD_RESULT (*)(FMOD_SOUND* sound, FMOD_RESULT result)
#define GO(A)   \
static uintptr_t my_nonblock_fct_##A = 0;                                       \
static uint32_t my_nonblock_##A(void* sound, uint32_t result)                   \
{                                                                               \
    return (uint32_t)RunFunctionFmt(my_nonblock_fct_##A, "pu", sound, result);  \
}
SUPER()
#undef GO
static void* find_nonblock_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nonblock_fct_##A == (uintptr_t)fct) return my_nonblock_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nonblock_fct_##A == 0) {my_nonblock_fct_##A = (uintptr_t)fct; return my_nonblock_##A;}
    SUPER()
    #undef GO
    printf("FmodWrapper: Warning, no more slot for fmod nonblock callback\n");
    return NULL;
}

#undef SUPER

// Head of FMOD_CREATESOUNDEXINFO. Only the fields up to nonblockcallback are declared: fmod
// versions agree on these, and diverge only further down. We copy the caller's struct verbatim
// (cbsize bytes) and patch just the three callback pointers, so the tail — whatever version it
// is — passes through untouched. x86_64 and arm64 are both LP64, so the layout matches.
// Project Zomboid reports "fmodintegration built with version 20206, fmod shared library
// version 20206" — i.e. both sides are FMOD 2.02.06, where sizeof(FMOD_CREATESOUNDEXINFO)==224.
typedef struct {
    int32_t     cbsize;
    uint32_t    length;
    uint32_t    fileoffset;
    int32_t     numchannels;
    int32_t     defaultfrequency;
    int32_t     format;
    uint32_t    decodebuffersize;
    int32_t     initialsubsound;
    int32_t     numsubsounds;
    void*       inclusionlist;
    int32_t     inclusionlistnum;
    void*       pcmreadcallback;
    void*       pcmsetposcallback;
    void*       nonblockcallback;
} fmod_exinfo_head_t;

// Lock the layout down here rather than discovering a mismatch as memory corruption on a
// tester's device: these offsets are what we patch into the caller's struct.
_Static_assert(offsetof(fmod_exinfo_head_t, pcmreadcallback)   == 56, "fmod exinfo layout drift");
_Static_assert(offsetof(fmod_exinfo_head_t, pcmsetposcallback) == 64, "fmod exinfo layout drift");
_Static_assert(offsetof(fmod_exinfo_head_t, nonblockcallback)  == 72, "fmod exinfo layout drift");
_Static_assert(sizeof(fmod_exinfo_head_t)                      == 80, "fmod exinfo layout drift");

#define FMOD_EXINFO_MAX 512
#define FMOD_EXINFO_EXPECTED 224 // sizeof(FMOD_CREATESOUNDEXINFO) in FMOD 2.02

static uint32_t create_sound_common(uFppupp_t native_fn, void* system, void* name_or_data,
                                    uint32_t mode, void* exinfo, void** sound, const char* who)
{
    if (!exinfo) return native_fn(system, name_or_data, mode, exinfo, sound);

    int32_t cbsize = *(int32_t*)exinfo; // the game writes sizeof() of the struct it compiled against
    if (cbsize < (int32_t)sizeof(fmod_exinfo_head_t) || cbsize > FMOD_EXINFO_MAX) {
        // Unexpected layout — don't touch it, better an un-bridged call than corrupted memory.
        printf("FmodWrapper: %s got exinfo with unexpected cbsize=%d, passing through unbridged\n", who, cbsize);
        return native_fn(system, name_or_data, mode, exinfo, sound);
    }
    if (cbsize != FMOD_EXINFO_EXPECTED) {
        // Still safe to bridge (we only touch the head, which is version-stable), but worth
        // knowing about: the game is on a different fmod than the 2.02 we sized against.
        printf("FmodWrapper: %s exinfo cbsize=%d, expected %d (fmod version drift?)\n",
               who, cbsize, FMOD_EXINFO_EXPECTED);
    }

    uint8_t buf[FMOD_EXINFO_MAX];
    memcpy(buf, exinfo, cbsize);
    fmod_exinfo_head_t* head = (fmod_exinfo_head_t*)buf;
    head->pcmreadcallback   = find_pcmread_Fct(head->pcmreadcallback);
    head->pcmsetposcallback = find_pcmsetpos_Fct(head->pcmsetposcallback);
    head->nonblockcallback  = find_nonblock_Fct(head->nonblockcallback);

    return native_fn(system, name_or_data, mode, buf, sound);
}

uint32_t my_FMOD_System_CreateSound(void* system, void* name_or_data, uint32_t mode, void* exinfo, void** sound)
{
    return create_sound_common(my->FMOD_System_CreateSound, system, name_or_data, mode, exinfo,
                               sound, "FMOD_System_CreateSound");
}

uint32_t my_FMOD_System_CreateStream(void* system, void* name_or_data, uint32_t mode, void* exinfo, void** sound)
{
    return create_sound_common(my->FMOD_System_CreateStream, system, name_or_data, mode, exinfo,
                               sound, "FMOD_System_CreateStream");
}

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