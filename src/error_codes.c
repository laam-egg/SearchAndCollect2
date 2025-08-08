#include "SearchAndCollect2/error_codes.h"

wchar_t const* const ErrorMessage[] = {
    L"Success",
    L"Error calling a WINAPI function",
    L"Stop iteration",
    L"Out of memory",
    L"Invalid handle - check arguments, variables' lifetime, and possibly buffer/stack overflow errors",
    L"Error from the lklist library",
    L"Access denied - check ownership and permissions",
    L"Invalid arguments - check arguments, variables' lifetime, and possibly buffer/stack overflow errors",
};

size_t const NUM_ERROR_MESSAGES = sizeof(ErrorMessage) / sizeof(ErrorMessage[0]);
