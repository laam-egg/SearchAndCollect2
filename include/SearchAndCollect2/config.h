#ifndef SearchAndCollect2_CONFIG_INCLUDED
#define SearchAndCollect2_CONFIG_INCLUDED

#include "SearchAndCollect2/common.h"

typedef enum {
	Config_DEFAULT = 0,
	Config_APPEND_RANDOM_BYTES = 1,
	Config_KEEP_ORIGINAL_NAMES = 2
} ConfigFlags;

typedef struct {
	ConfigFlags flags;
	int argc;
	wchar_t const* const* argv;
	wchar_t const* inputPath;
	wchar_t const* outputDirPath;
} Config;

extern Config _g_config; // should not access this variable directly. Use Config_Get() below.

static inline Config const* Config_Get(void) {
	return &_g_config;
}

ErrorCode Config_Init(void);

Config const* Config_Get(void);

void Config_Close(void);

void printHelp(void);

#endif // SearchAndCollect2_CONFIG_INCLUDED
