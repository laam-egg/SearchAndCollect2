#include "SearchAndCollect2/error_codes.h"

wchar_t const* const ErrorMessage[] = {
    L"Success, no error",
    L"Error calling a WINAPI function",
    L"Unknown error",
    L"Developer fault",
    L"Stop iteration",
    L"Out of memory",
    L"Invalid handle - check arguments, variables' lifetime, and possibly buffer/stack overflow errors",
    L"Error from the lklist library",
    L"Access denied - check ownership and permissions",
    L"Invalid argument",
    L"Invalid command line arguments"
};

size_t const NUM_ERROR_MESSAGES = sizeof(ErrorMessage) / sizeof(ErrorMessage[0]);
