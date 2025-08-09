#include "SearchAndCollect2/config.h"
#include <windows.h>

#define TRAP(ERR) err = ERR; if (err != ERR_NONE) goto cleanup;
#define THROW(ERR) err = ERR; goto cleanup;


/**
 * Maximum length of an option name,
 * including the prefix "--"
 */
#define MAX_OPTION_NAME_LENGTH 32
typedef struct {
	wchar_t const* name;
	ConfigFlags flags;
	wchar_t const* explanation;
} ConfigOption;

static ConfigOption const AVAILABLE_OPTIONS[] = {
	{
		L"append-random-bytes",
		Config_APPEND_RANDOM_BYTES,
		L"    If set, add a few random bytes at the end of the copied file to change the\n"
		L"file's signature, which evades signature-based malware detection. The copied file\n"
		L"is still named after the hash of the original file.\n"
	}
};
static int const NUM_AVAILABLE_OPTIONS = sizeof(AVAILABLE_OPTIONS) / sizeof(AVAILABLE_OPTIONS[0]);

ErrorCode matchOption(Config* const lpConfig, wchar_t const* const optionName);
ErrorCode matchPositionalArgument(Config* const lpConfig, int const positionalArgumentOrdinal, wchar_t const* const argument);
static int const NUM_POSITIONAL_PARAMETERS;

ErrorCode Config_Init(Config* const lpConfig) {
	ErrorCode err = ERR_NONE;
	memset(lpConfig, 0, sizeof(Config));

	{
		lpConfig->argv = CommandLineToArgvW(GetCommandLineW(), &lpConfig->argc);
		if (!lpConfig->argv) {
			DWORD dwLastError = GetLastError();
			debug(L"ERROR: Config_Init(): CommandLineTArgvW() failed with GetLastError = %lu", dwLastError);
			THROW(ERR_WINAPI)
		}
	}

	int const argc = lpConfig->argc;
	wchar_t const* const* const argv = lpConfig->argv;
	{
		if (argc < 3) {
			wprintf(L"ERROR: Too few arguments\n");
			THROW(ERR_COMMAND_LINE_ARGS)
		}

		int numPositionalArgs = 0;
		for (int i = 1; i < argc; ++i) {
			wchar_t const* const elem = argv[i];
			size_t elemSize = wcslen(elem);
			if (elemSize == 0) {
				continue;
			} else if (elemSize >= 2 && elem[0] == L'-' && elem[1] == L'-') {
				// Options starting with L"--"
				wchar_t const* const optionName = elem + 2;
				TRAP(matchOption(lpConfig, optionName))
			} else {
				// Positional argument
				++numPositionalArgs;
				TRAP(matchPositionalArgument(lpConfig, numPositionalArgs, elem))
			}
		}

		if (numPositionalArgs < NUM_POSITIONAL_PARAMETERS) {
			wprintf(L"ERROR: Missing one or more positional arguments\n");
			THROW(ERR_COMMAND_LINE_ARGS)
		}
	}

	cleanup:
		if (err != ERR_NONE) {
			Config_Close(lpConfig);
		}

	return err;
}

void Config_Close(Config* const lpConfig) {
	if (lpConfig->argv != NULL) {
		LocalFree(lpConfig->argv);
	}
}

///////////////////////////////////////////
/////////// MATCHING OPTIONS //////////////
///////////////////////////////////////////

ErrorCode matchOption(Config* const lpConfig, wchar_t const* const optionName) {
	if (wcsnlen(optionName, 1) == 0) {
		wprintf(L"ERROR: Option name not specified after '--'\n");
		return ERR_COMMAND_LINE_ARGS;
	}

	for (int i = 0; i < NUM_AVAILABLE_OPTIONS; ++i) {
		ConfigOption const* const currentOption = &AVAILABLE_OPTIONS[i];
		if (0 == wcscmp(optionName, currentOption->name)) {
			lpConfig->flags |= currentOption->flags;
			return ERR_NONE;
		}
	}

	wprintf(L"ERROR: Unsupported option: %ls", optionName);
	return ERR_COMMAND_LINE_ARGS;
}

///////////////////////////////////////////
////// MATCHING POSITIONAL ARGUMENTS //////
///////////////////////////////////////////

int const NUM_POSITIONAL_PARAMETERS = 2;

ErrorCode matchPositionalArgument(Config* const lpConfig, int const positionalArgumentOrdinal, wchar_t const* const argument) {
	ErrorCode err = ERR_NONE;

	BOOL requireNonEmpty = FALSE;
	if (positionalArgumentOrdinal == 1) {
		lpConfig->inputPath = argument;
		requireNonEmpty = TRUE;
	} else if (positionalArgumentOrdinal == 2) {
		lpConfig->outputDirPath = argument;
		requireNonEmpty = TRUE;
	} else {
		wprintf(L"ERROR: Too many arguments\n");
		THROW(ERR_COMMAND_LINE_ARGS)
	}

	if (requireNonEmpty) {
		if (wcsnlen(argument, 1) == 0) {
			wprintf(L"ERROR: empty (positional) argument at position %d\n", positionalArgumentOrdinal);
			THROW(ERR_COMMAND_LINE_ARGS)
		}
	}

	cleanup:
	return err;
}

///////////////////////////////////////////
////////////////// HELP ///////////////////
///////////////////////////////////////////

void printHelp() {
	wprintf(
		L"\n"
		L"===========================\n"
		L"Usage:\n"
		L"    SearchAndCollect2.exe <inputdir> <outputdir> [options_if_necessary]\n"
		L"e.g.\n"
		L"    SearchAndCollect2.exe C:\\Windows\\System32 D:\\PE\n"
		L"would (recursively) copy every PE file in System32 to the specified PE folder.\n"
		L"\n"
		L"Every copied file is named after the original file's SHA256 hash, while retaining\n"
		L"the original file extension.\n"
		L"\n"
		L"Options:\n"
	);

	for (int i = 0; i < NUM_AVAILABLE_OPTIONS; ++i) {
		ConfigOption const* const currentOption = &AVAILABLE_OPTIONS[i];
		wprintf(L"--%ls\n%ls\n\n", currentOption->name, currentOption->explanation);
	}

	wprintf(
		L"===========================\n"
		L"\n"
	);
}
