#ifndef SearchAndCollect2_CONFIG_INCLUDED
#define SearchAndCollect2_CONFIG_INCLUDED

#include "SearchAndCollect2/common.h"

typedef enum {
	Config_DEFAULT = 0,
	Config_INVALID_COMMAND_LINE_ARGS = 1,
	Config_APPEND_RANDOM_BYTES = 2
} ConfigFlags;

typedef struct {
	ConfigFlags flags;
	int argc;
	wchar_t const* const* argv;
	wchar_t const* inputPath;
	wchar_t const* outputDirPath;
} Config;

ErrorCode Config_Init(Config* const lpConfig);

void Config_Close(Config* const lpConfig);

void printHelp();

#endif // SearchAndCollect2_CONFIG_INCLUDED
