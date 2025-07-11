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

#include "generated/wrappedfmodstudiodefs.h"

const char* fmodstudioName = "libfmodstudio.so.13";
#define LIBNAME fmodstudio
#define ALTNAME "libfmodstudio.so"

#include "generated/wrappedfmodstudiotypes.h"

#include "wrappercallback.h"

typedef enum FMOD_RESULT
{
    FMOD_OK,
    FMOD_ERR_BADCOMMAND,
    FMOD_ERR_CHANNEL_ALLOC,
    FMOD_ERR_CHANNEL_STOLEN,
    FMOD_ERR_DMA,
    FMOD_ERR_DSP_CONNECTION,
    FMOD_ERR_DSP_DONTPROCESS,
    FMOD_ERR_DSP_FORMAT,
    FMOD_ERR_DSP_INUSE,
    FMOD_ERR_DSP_NOTFOUND,
    FMOD_ERR_DSP_RESERVED,
    FMOD_ERR_DSP_SILENCE,
    FMOD_ERR_DSP_TYPE,
    FMOD_ERR_FILE_BAD,
    FMOD_ERR_FILE_COULDNOTSEEK,
    FMOD_ERR_FILE_DISKEJECTED,
    FMOD_ERR_FILE_EOF,
    FMOD_ERR_FILE_ENDOFDATA,
    FMOD_ERR_FILE_NOTFOUND,
    FMOD_ERR_FORMAT,
    FMOD_ERR_HEADER_MISMATCH,
    FMOD_ERR_HTTP,
    FMOD_ERR_HTTP_ACCESS,
    FMOD_ERR_HTTP_PROXY_AUTH,
    FMOD_ERR_HTTP_SERVER_ERROR,
    FMOD_ERR_HTTP_TIMEOUT,
    FMOD_ERR_INITIALIZATION,
    FMOD_ERR_INITIALIZED,
    FMOD_ERR_INTERNAL,
    FMOD_ERR_INVALID_FLOAT,
    FMOD_ERR_INVALID_HANDLE,
    FMOD_ERR_INVALID_PARAM,
    FMOD_ERR_INVALID_POSITION,
    FMOD_ERR_INVALID_SPEAKER,
    FMOD_ERR_INVALID_SYNCPOINT,
    FMOD_ERR_INVALID_THREAD,
    FMOD_ERR_INVALID_VECTOR,
    FMOD_ERR_MAXAUDIBLE,
    FMOD_ERR_MEMORY,
    FMOD_ERR_MEMORY_CANTPOINT,
    FMOD_ERR_NEEDS3D,
    FMOD_ERR_NEEDSHARDWARE,
    FMOD_ERR_NET_CONNECT,
    FMOD_ERR_NET_SOCKET_ERROR,
    FMOD_ERR_NET_URL,
    FMOD_ERR_NET_WOULD_BLOCK,
    FMOD_ERR_NOTREADY,
    FMOD_ERR_OUTPUT_ALLOCATED,
    FMOD_ERR_OUTPUT_CREATEBUFFER,
    FMOD_ERR_OUTPUT_DRIVERCALL,
    FMOD_ERR_OUTPUT_FORMAT,
    FMOD_ERR_OUTPUT_INIT,
    FMOD_ERR_OUTPUT_NODRIVERS,
    FMOD_ERR_PLUGIN,
    FMOD_ERR_PLUGIN_MISSING,
    FMOD_ERR_PLUGIN_RESOURCE,
    FMOD_ERR_PLUGIN_VERSION,
    FMOD_ERR_RECORD,
    FMOD_ERR_REVERB_CHANNELGROUP,
    FMOD_ERR_REVERB_INSTANCE,
    FMOD_ERR_SUBSOUNDS,
    FMOD_ERR_SUBSOUND_ALLOCATED,
    FMOD_ERR_SUBSOUND_CANTMOVE,
    FMOD_ERR_TAGNOTFOUND,
    FMOD_ERR_TOOMANYCHANNELS,
    FMOD_ERR_TRUNCATED,
    FMOD_ERR_UNIMPLEMENTED,
    FMOD_ERR_UNINITIALIZED,
    FMOD_ERR_UNSUPPORTED,
    FMOD_ERR_VERSION,
    FMOD_ERR_EVENT_ALREADY_LOADED,
    FMOD_ERR_EVENT_LIVEUPDATE_BUSY,
    FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH,
    FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT,
    FMOD_ERR_EVENT_NOTFOUND,
    FMOD_ERR_STUDIO_UNINITIALIZED,
    FMOD_ERR_STUDIO_NOT_LOADED,
    FMOD_ERR_INVALID_STRING,
    FMOD_ERR_ALREADY_LOCKED,
    FMOD_ERR_NOT_LOCKED,
    FMOD_ERR_RECORD_DISCONNECTED,
    FMOD_ERR_TOOMANYSAMPLES,

    FMOD_RESULT_FORCEINT = 65536
} FMOD_RESULT;

typedef enum FMOD_OUTPUTTYPE
{
    FMOD_OUTPUTTYPE_AUTODETECT,
    FMOD_OUTPUTTYPE_UNKNOWN,
    FMOD_OUTPUTTYPE_NOSOUND,
    FMOD_OUTPUTTYPE_WAVWRITER,
    FMOD_OUTPUTTYPE_NOSOUND_NRT,
    FMOD_OUTPUTTYPE_WAVWRITER_NRT,
    FMOD_OUTPUTTYPE_WASAPI,
    FMOD_OUTPUTTYPE_ASIO,
    FMOD_OUTPUTTYPE_PULSEAUDIO,
    FMOD_OUTPUTTYPE_ALSA,
    FMOD_OUTPUTTYPE_COREAUDIO,
    FMOD_OUTPUTTYPE_AUDIOTRACK,
    FMOD_OUTPUTTYPE_OPENSL,
    FMOD_OUTPUTTYPE_AUDIOOUT,
    FMOD_OUTPUTTYPE_AUDIO3D,
    FMOD_OUTPUTTYPE_WEBAUDIO,
    FMOD_OUTPUTTYPE_NNAUDIO,
    FMOD_OUTPUTTYPE_WINSONIC,
    FMOD_OUTPUTTYPE_AAUDIO,
    FMOD_OUTPUTTYPE_AUDIOWORKLET,
    FMOD_OUTPUTTYPE_PHASE,
    FMOD_OUTPUTTYPE_OHAUDIO,

    FMOD_OUTPUTTYPE_MAX,
    FMOD_OUTPUTTYPE_FORCEINT = 65536
} FMOD_OUTPUTTYPE;

typedef struct FMOD_GUID
{
    unsigned int   Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} FMOD_GUID;

typedef enum FMOD_SPEAKERMODE
{
    FMOD_SPEAKERMODE_DEFAULT,
    FMOD_SPEAKERMODE_RAW,
    FMOD_SPEAKERMODE_MONO,
    FMOD_SPEAKERMODE_STEREO,
    FMOD_SPEAKERMODE_QUAD,
    FMOD_SPEAKERMODE_SURROUND,
    FMOD_SPEAKERMODE_5POINT1,
    FMOD_SPEAKERMODE_7POINT1,
    FMOD_SPEAKERMODE_7POINT1POINT4,

    FMOD_SPEAKERMODE_MAX,
    FMOD_SPEAKERMODE_FORCEINT = 65536
} FMOD_SPEAKERMODE;

static void* fmod_core;
static void* fmod_studio;
static FMOD_RESULT(*System_setOutput)(void*, FMOD_OUTPUTTYPE);
static FMOD_RESULT(*System_setDriver)(void*, int);
static FMOD_RESULT(*System_getNumDrivers)(void*, int*);
static FMOD_RESULT(*System_getDriverInfo)(void*, int, char*, int, FMOD_GUID*, int*, FMOD_SPEAKERMODE*, int*);
static FMOD_RESULT(*Studio_System_create)(void*, unsigned int);
static FMOD_RESULT(*Studio_System_getCoreSystem)(void*, void**);


static int init_fmod_symbols() {
    fmod_core = dlopen("libfmod.so", RTLD_LOCAL);
    if (!fmod_core) {
        printf("FmodWrapper: failed to load libfmod.so\n");
        return 0;
    }

    fmod_studio = dlopen("libfmodstudio.so", RTLD_LOCAL);
    if (!fmod_studio) {
        printf("FmodWrapper: failed to load libfmodstudio.so\n");
        return 0;
    }

    System_setOutput = dlsym(fmod_core, "_ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE");
    if (!System_setOutput) {
        printf("FmodWrapper: failed to locate _ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE\n");
        return 0;
    }

    System_setDriver = dlsym(fmod_core, "_ZN4FMOD6System9setDriverEi");
    if (!System_setDriver) {
        printf("FmodWrapper: failed to locate _ZN4FMOD6System9setDriverEi\n");
        return 0;
    }

    System_getNumDrivers = dlsym(fmod_core, "_ZN4FMOD6System13getNumDriversEPi");
    if (!System_getNumDrivers) {
        printf("FmodWrapper: failed to locate _ZN4FMOD6System13getNumDriversEPi\n");
        return 0;
    }

    System_getDriverInfo = dlsym(fmod_core, "_ZN4FMOD6System13getDriverInfoEiPciP9FMOD_GUIDPiP16FMOD_SPEAKERMODES4_");
    if (!System_getDriverInfo) {
        printf("FmodWrapper: failed to locate _ZN4FMOD6System13getDriverInfoEiPciP9FMOD_GUIDPiP16FMOD_SPEAKERMODES4_\n");
        return 0;
    }

    Studio_System_create = dlsym(fmod_studio, "_ZN4FMOD6Studio6System6createEPPS1_j");
    if (!Studio_System_create) {
        printf("FmodWrapper: failed to locate _ZN4FMOD6Studio6System6createEPPS1_j\n");
        return 0;
    }

    Studio_System_getCoreSystem = dlsym(fmod_studio, "_ZNK4FMOD6Studio6System13getCoreSystemEPPNS_6SystemE");
    if (!Studio_System_getCoreSystem) {
        printf("FmodWrapper: failed to locate _ZNK4FMOD6Studio6System13getCoreSystemEPPNS_6SystemE\n");
        return 0;
    }

    return 1;
}

FMOD_RESULT my__ZN4FMOD6Studio6System6createEPPS1_j(void** this, uint some_arg) {
    static int once = 1;
    static int init = 0;

    if (once) {
        init = init_fmod_symbols();
        once = 0;
    }

    FMOD_RESULT result = Studio_System_create(this, some_arg);
    if (result != FMOD_OK || !init) {
        return result;
    }

    void* coreSystem = NULL;
    Studio_System_getCoreSystem(*this, &coreSystem);
    if (!coreSystem) {
        printf("FmodWrapper: failed to get core system from studio system\n");
        return result;
    }

    const char* audioAPI = getenv("ZOMDROID_AUDIO_API");
    if (!audioAPI) {
        return result;
    }

    if (strstr(audioAPI, "AAUDIO")) {
        System_setOutput(coreSystem, FMOD_OUTPUTTYPE_AAUDIO);
    } else if (strstr(audioAPI, "OPENSL")) {
        System_setOutput(coreSystem, FMOD_OUTPUTTYPE_OPENSL);
    }

    return result;
}
// Insert code here

#include "wrappedlib_init.h"