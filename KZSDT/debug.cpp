#include "Debug.h"

#ifndef _DEBUG
NullBuffer null_buffer;
std::ostream null_stream(&null_buffer);
#endif