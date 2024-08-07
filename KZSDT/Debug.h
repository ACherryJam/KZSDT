#pragma once

#if _DEBUG

#include <iostream>

#define dcout std::cout

#else

#include <streambuf>
#include <ostream>

// Standard no-op output stream: https://stackoverflow.com/a/11826666
class NullBuffer : public std::streambuf
{
public:
	int overflow(int c) {
		return c;
	}
};

extern std::ostream null_stream;

#define dcout null_stream

#endif