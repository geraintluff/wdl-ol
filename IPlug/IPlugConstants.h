#pragma once

/**
 * @file
 * @brief Constant definitions, magic numbers
 */


#if !defined(SAMPLE_TYPE_FLOAT) && !defined(SAMPLE_TYPE_DOUBLE)
#define SAMPLE_TYPE_DOUBLE
#endif

#ifdef SAMPLE_TYPE_DOUBLE
typedef double PLUG_SAMPLE_DST;
typedef float PLUG_SAMPLE_SRC;
#else
typedef float PLUG_SAMPLE_DST;
typedef double PLUG_SAMPLE_SRC;
#endif

typedef PLUG_SAMPLE_DST sample;

#define LOGFILE "IPlugLog.txt"
#define MAX_PROCESS_TRACE_COUNT 100
#define MAX_IDLE_TRACE_COUNT 15

enum EIPlugKeyCodes
{
  KEY_SPACE,
  KEY_UPARROW,
  KEY_DOWNARROW,
  KEY_LEFTARROW,
  KEY_RIGHTARROW,
  KEY_DIGIT_0,
  KEY_DIGIT_9=KEY_DIGIT_0+9,
  KEY_ALPHA_A,
  KEY_ALPHA_Z=KEY_ALPHA_A+25,
  KEY_NONE
};

enum EVST3ParamIDs
{
#ifndef IPLUG1_COMPATIBILITY
  kBypassParam = 'bpas',
  kPresetParam = 'prst',
  kMIDICCParamStartIdx
#else
  kBypassParam = 65536,
  kPresetParam, // not used unless baked in presets declared
  kMIDICCParamStartIdx
#endif
};

//TODO: these should be in a namespace, to avoid conflicts with third-party libraries
static const double PI = 3.141592653589793238;

/** @brief Magic number for gain to dB conversion.
 * Approximates \f$ 20*log_{10}(x) \f$
 * @see AmpToDB
*/
static const double AMP_DB = 8.685889638065036553;
/** @brief Magic number for dB to gain conversion.
 * Approximates \f$ 10^{\frac{x}{20}} \f$
 * @see DBToAmp
*/
static const double IAMP_DB = 0.11512925464970;
static const double DEFAULT_SAMPLE_RATE = 44100.0;
static const int MAX_PRESET_NAME_LEN = 256;
#define UNUSED_PRESET_NAME "empty"
#define DEFAULT_USER_PRESET_NAME "user preset"

#define AU_MAX_IO_CHANNELS 128

#define MAX_WIN32_PATH_LEN 256
#define MAX_WIN32_PARAM_LEN 256

#define MAX_PLUGIN_NAME_LEN 128

#define MAX_PARAM_NAME_LEN 32 // e.g. "Gain"
#define MAX_PARAM_LABEL_LEN 32 // e.g. "Percent"
#define MAX_PARAM_DISPLAY_LEN 32 // e.g. "100" / "Mute"
#define MAX_PARAM_GROUP_LEN 32 // e.g. "oscillator section"
#define MAX_BUS_NAME_LEN 32 // e.g. "sidechain input"
#define MAX_CHAN_NAME_LEN 32 // e.g. "input 1"

#define MAX_VERSION_STR_LEN 32
#define MAX_BUILD_INFO_STR_LEN 256
static const int MAX_PARAM_DISPLAY_PRECISION = 6;

#define MAX_AAX_PARAMID_LEN 32

#define PARAM_UNINIT 99.99e-9

#ifndef MAX_BLOB_LENGTH
#define MAX_BLOB_LENGTH 2048
#endif

// All version ints are stored as 0xVVVVRRMM: V = version, R = revision, M = minor revision.
#define IPLUG_VERSION 0x010000
#define IPLUG_VERSION_MAGIC 'pfft'

static const int DEFAULT_BLOCK_SIZE = 1024;
static const double DEFAULT_TEMPO = 120.0;
static const int kNoParameter = -1;

#define MAX_BUS_CHANS 64 // wild cards in channel i/o strings will result in this many channels

//#ifdef VST3_API
//#undef stricmp
//#undef strnicmp
//#include "pluginterfaces/vst/vsttypes.h"
//static const uint64_t kInvalidBusType = Steinberg::Vst::SpeakerArr::kEmpty;
//#elif defined AU_API || AUv3_API
//#include <CoreAudio/CoreAudio.h>
//static const uint64_t kInvalidBusType = kAudioChannelLayoutTag_Unknown;
//#elif defined AAX_API
//#include "AAX_Enums.h"
//static const uint64_t kInvalidBusType = AAX_eStemFormat_None;
//#else
//static const uint64_t kInvalidBusType = 0;
//#endif

/** @enum EParamSource
 * Used to identify the source of a parameter change
 */
enum EParamSource
{
  kReset,
  kAutomation,
  kPresetRecall,
  kGUI
};

static const char* ParamSourceStrs[4] = { "Reset", "Automation", "Preset", "GUI" };

/** @enum ERoute
 * Used to identify whether a bus/channel connection is an input or an output
 */
enum ERoute
{
  kInput = 0,
  kOutput = 1
};

static const char* RoutingDirStrs[2]  = { "input", "output" };

enum EAPI
{
  kAPIVST2 = 0,
  kAPIVST3 = 1,
  kAPIAU = 2,
  kAPIAUv3 = 3,
  kAPIAAX = 4,
  kAPIAPP = 5
};

/** @enum EHost
 * Host identifier
 */
enum EHost
{
  kHostUninit = -1,
  kHostUnknown = 0,
  kHostReaper,
  kHostProTools,
  kHostCubase,
  kHostNuendo,
  kHostSonar,
  kHostVegas,
  kHostFL,
  kHostSamplitude,
  kHostAbletonLive,
  kHostTracktion,
  kHostNTracks,
  kHostMelodyneStudio,
  kHostVSTScanner,
  kHostAULab,
  kHostForte,
  kHostChainer,
  kHostAudition,
  kHostOrion,
  kHostBias,
  kHostSAWStudio,
  kHostLogic,
  kHostGarageBand,
  kHostDigitalPerformer,
  kHostStandalone,
  kHostAudioMulch,
  kHostStudioOne,
  kHostVST3TestHost,
  kHostArdour,
  kHostRenoise,
  kHostOpenMPT,
  kHostWaveLab,
  kHostWaveLabElements,
  kHostTwistedWave,
  kHostBitwig
  
  // These hosts don't report the host name:
  // EnergyXT2
  // MiniHost
};


