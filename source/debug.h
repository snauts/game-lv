#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <fstream>

using namespace std;

#define SOURCE_POS __FILE__ << ", line " <<__LINE__

#ifdef _DEBUG_OUTPUT

typedef ofstream debug_out;

#else

class m_streambuf : public streambuf {
public:
	m_streambuf() : streambuf() {
	}
	~m_streambuf() {
	}
};
extern m_streambuf m_stream;

class m_debug : public ostream{
public:
	m_debug(const char* str) : ostream(&m_stream), ios(&m_stream) {}
	~m_debug() {}

#ifdef __GNUC__
	inline  m_debug& operator<<(ios_base& (*fn)(ios_base&)){ return *this;}
	inline  m_debug& operator<<(ostream& (*fn)(ostream&)){ return *this; }
#else
	inline  m_debug& operator<<(ios& (__cdecl * _f)(ios&)){ return *this;}
	inline  m_debug& operator<<(ostream& (__cdecl * _f)(ostream&)){ return *this;}
#endif
	inline  m_debug& operator<<(const char *){ return *this;}
	inline  m_debug& operator<<(const unsigned char *){ return *this;}
	inline  m_debug& operator<<(const signed char *){ return *this;}
	inline  m_debug& operator<<(char){ return *this;}
	inline  m_debug& operator<<(unsigned char){ return *this;}
	inline  m_debug& operator<<(signed char){ return *this;}
	inline  m_debug& operator<<(short){ return *this;}
	inline  m_debug& operator<<(unsigned short){ return *this;}
	inline  m_debug& operator<<(int){ return *this;}
	inline  m_debug& operator<<(unsigned int){ return *this;}
	inline  m_debug& operator<<(long){ return *this;}
	inline  m_debug& operator<<(unsigned long){ return *this;}
	inline  m_debug& operator<<(float){ return *this;}
	inline  m_debug& operator<<(double){ return *this;}
	inline  m_debug& operator<<(long double){ return *this;}
	inline  m_debug& operator<<(const void *){ return *this;}
	inline  m_debug& operator<<(streambuf*){ return *this;}

};
typedef m_debug debug_out;
#endif
#endif
