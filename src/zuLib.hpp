/*
/#   Script File: zuLib.hpp
/#
/#   Description:
/#
/#   zuLib header
/#
/#
/#   Author: Joshua Zhang (zzbhf@mail.missouri.edu)
/#   Date since: APR-2015
/#
/#   Copyright (c) <2015> <JOSHUA Z. ZHANG>	 - All Rights Reserved.
/#
/#	 Open source according to MIT License.
/#	 No warrenty implied, use at your own risk.
*/
/***********************************************************************/

#ifndef _ZULIB_HPP_
#define _ZULIB_HPP_

#ifndef __cplusplus
#  error ZULib.hpp header must be compiled as C++
#endif

#if defined(_MSC_VER) && _MSC_VER > 1400
#define _CRT_SECURE_NO_WARNINGS // suppress warnings about fopen() and similar "unsafe" functions defined by MS
#endif


#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <climits>
#include <sstream>
#include <cstdlib>
#include <cmath>



/*-----------------------------------------------------------------------*/
// No maximum char size garanteed in typedef, but minimum size is ensured
//CHAR_BIT * sizeof (char) >= 8
//CHAR_BIT * sizeof (short) >= 16
//CHAR_BIT * sizeof (int) >= 16
//CHAR_BIT * sizeof (long) >= 32
//CHAR_BIT * sizeof (long long) >= 64
/*-----------------------------------------------------------------------*/
//////////////////////////////// Typedef ////////////////////////////////
typedef unsigned char		uchar;		//!< unsigned 8-bit integer
typedef char				schar;		//!< signed 8-bit integer
typedef unsigned short		ushort;		//!< unsigned 16-bit integer
typedef short				sshort;		//!< signed 16-bit integer
typedef unsigned long		uint32;		//!< unsigned 32-bit integer
typedef long				int32;		//!< signed 32-bit integer
typedef unsigned long long	uint64;		//!< unsigned 64-bit integer
typedef long long			int64;		//!< signed 64-bit integer
typedef std::string			String;		//!< STL string
typedef std::vector<int32>	Veci;		//!< signed 32-bit integer STL vector



// ----------------------- Error/Warn/Info handling ---------------------//

enum EXIT_STATUS { FATAL_ERROR = -1, OKAY = 0 };

inline void error(const String &msg)
{
	std::cerr << "[Error] - " << msg << std::endl;
#if (defined _DEBUG) || (defined DEBUG) || (defined NDEBUG)
	std::cerr << "Error captured, press enter to exit!" << std::endl;
	std::cin.ignore();
#endif
	exit(FATAL_ERROR);
}

inline void warning(const String &msg)
{
	std::cerr << "[Warning] - " << msg << std::endl;
}

inline void info(const String &msg, bool newline = true)
{
	std::cout << msg;
	if (newline)
	{
		std::cout << std::endl;
	}
}

inline void info_debug(const String &msg, bool newline = true)
{
#if (defined _DEBUG) || (defined DEBUG) || (defined NDEBUG)
	std::cout << msg;
	if (newline)
	{
		std::cout << std::endl;
	}
#endif
}

#define SSTR( x ) (dynamic_cast< std::ostringstream & >( std::ostringstream() << std::dec << x).str())
// print error and throw
#define Error(err) (error(SSTR(err)))
// print warning, continue
#define Warning(warn) (warning(SSTR(warn)))
#define Print(msg) (info(SSTR(msg), false))
#define Println(msg) (info(SSTR(msg), true))
#define Print_d(msg) (info_debug(SSTR(msg), false))
#define Println_d(msg) (info_debug(SSTR(msg), true))

namespace zz
{


	// ----------------------------------- Math ---------------------------------//
	/// <summary>
	/// Return the abosolute value
	/// </summary>
	/// <param name="v">The input value.</param>
	/// <returns>Abosolute value</returns>
	template<class T> inline const T abs(const T& v) { return (v < 0) ? -v : v; }

	/// <summary>
	/// Return the smaller value
	/// </summary>
	/// <param name="a">a.</param>
	/// <param name="b">b.</param>
	/// <returns>Smaller value</returns>
	template<class T> inline const T min(const T& a, const T& b) { return (b < a) ? b : a; }

	/// <summary>
	/// Return the larger value
	/// </summary>
	/// <param name="a">a.</param>
	/// <param name="b">b.</param>
	/// <returns>Larger value</returns>
	template<class T> inline const T max(const T& a, const T& b) { return (b > a) ? b : a; }

	/// <summary>
	/// Lock the specified value inside range [low, high]
	/// </summary>
	/// <param name="value">The value.</param>
	/// <param name="low">The low threshold.</param>
	/// <param name="high">The high threshold.</param>
	/// <returns>Original or low threshold or high threshold</returns>
	template<class T> inline const T saturate(const T& value, const T& low, const T& high)
	{
		// fool proof max/min values
		T h = max(low, high);
		T l = min(low, high);
		return max(min(value, h), l);
	}

	/// <summary>
	/// inline fast round function
	/// </summary>
	/// <param name="value">The floating value.</param>
	/// <returns>The nearest interger value.</returns>
	inline int round(double value)
	{
#if ((defined _MSC_VER && defined _M_X64) || (defined __GNUC__ && defined __x86_64__ && defined __SSE2__ && !defined __APPLE__)) && !defined(__CUDACC__) && 0
		__m128d t = _mm_set_sd(value);
		return _mm_cvtsd_si32(t);
#elif defined _MSC_VER && defined _M_IX86
		int t;
		__asm
		{
			fld value;
			fistp t;
		}
		return t;
#elif defined _MSC_VER && defined _M_ARM && defined HAVE_TEGRA_OPTIMIZATION
		TEGRA_ROUND(value);
#elif defined CV_ICC || defined __GNUC__
#  ifdef HAVE_TEGRA_OPTIMIZATION
		TEGRA_ROUND(value);
#  else
		return (int)lrint(value);
#  endif
#else
		double intpart, fractpart;
		fractpart = modf(value, &intpart);
		if ((fabs(fractpart) != 0.5) || ((((int)intpart) % 2) != 0))
			return (int)(value + (value >= 0 ? 0.5 : -0.5));
		else
			return (int)intpart;
#endif
	}

	//! \cond
	/////////////// saturate_cast (used in image & signal processing) ///////////////////

	template<typename _Tp> static inline _Tp saturate_cast(uchar v) { return _Tp(v); }
	template<typename _Tp> static inline _Tp saturate_cast(schar v) { return _Tp(v); }
	template<typename _Tp> static inline _Tp saturate_cast(ushort v) { return _Tp(v); }
	template<typename _Tp> static inline _Tp saturate_cast(short v) { return _Tp(v); }
	template<typename _Tp> static inline _Tp saturate_cast(unsigned v) { return _Tp(v); }
	template<typename _Tp> static inline _Tp saturate_cast(int v) { return _Tp(v); }
	template<typename _Tp> static inline _Tp saturate_cast(float v) { return _Tp(v); }
	template<typename _Tp> static inline _Tp saturate_cast(double v) { return _Tp(v); }

	template<> inline uchar saturate_cast<uchar>(schar v)
	{
		return (uchar)max((int)v, 0);
	}
	template<> inline uchar saturate_cast<uchar>(ushort v)
	{
		return (uchar)min((unsigned)v, (unsigned)UCHAR_MAX);
	}
	template<> inline uchar saturate_cast<uchar>(int v)
	{
		return (uchar)((unsigned)v <= UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0);
	}
	template<> inline uchar saturate_cast<uchar>(short v)
	{
		return saturate_cast<uchar>((int)v);
	}
	template<> inline uchar saturate_cast<uchar>(unsigned v)
	{
		return (uchar)min(v, (unsigned)UCHAR_MAX);
	}
	template<> inline uchar saturate_cast<uchar>(float v)
	{
		int iv = round(v); return saturate_cast<uchar>(iv);
	}
	template<> inline uchar saturate_cast<uchar>(double v)
	{
		int iv = round(v); return saturate_cast<uchar>(iv);
	}

	template<> inline schar saturate_cast<schar>(uchar v)
	{
		return (schar)min((int)v, SCHAR_MAX);
	}
	template<> inline schar saturate_cast<schar>(ushort v)
	{
		return (schar)min((unsigned)v, (unsigned)SCHAR_MAX);
	}
	template<> inline schar saturate_cast<schar>(int v)
	{
		return (schar)((unsigned)(v - SCHAR_MIN) <= (unsigned)UCHAR_MAX ?
		v : v > 0 ? SCHAR_MAX : SCHAR_MIN);
	}
	template<> inline schar saturate_cast<schar>(short v)
	{
		return saturate_cast<schar>((int)v);
	}
	template<> inline schar saturate_cast<schar>(unsigned v)
	{
		return (schar)min(v, (unsigned)SCHAR_MAX);
	}

	template<> inline schar saturate_cast<schar>(float v)
	{
		int iv = round(v); return saturate_cast<schar>(iv);
	}
	template<> inline schar saturate_cast<schar>(double v)
	{
		int iv = round(v); return saturate_cast<schar>(iv);
	}

	template<> inline ushort saturate_cast<ushort>(schar v)
	{
		return (ushort)max((int)v, 0);
	}
	template<> inline ushort saturate_cast<ushort>(short v)
	{
		return (ushort)max((int)v, 0);
	}
	template<> inline ushort saturate_cast<ushort>(int v)
	{
		return (ushort)((unsigned)v <= (unsigned)USHRT_MAX ? v : v > 0 ? USHRT_MAX : 0);
	}
	template<> inline ushort saturate_cast<ushort>(unsigned v)
	{
		return (ushort)min(v, (unsigned)USHRT_MAX);
	}
	template<> inline ushort saturate_cast<ushort>(float v)
	{
		int iv = round(v); return saturate_cast<ushort>(iv);
	}
	template<> inline ushort saturate_cast<ushort>(double v)
	{
		int iv = round(v); return saturate_cast<ushort>(iv);
	}

	template<> inline short saturate_cast<short>(ushort v)
	{
		return (short)min((int)v, SHRT_MAX);
	}
	template<> inline short saturate_cast<short>(int v)
	{
		return (short)((unsigned)(v - SHRT_MIN) <= (unsigned)USHRT_MAX ?
		v : v > 0 ? SHRT_MAX : SHRT_MIN);
	}
	template<> inline short saturate_cast<short>(unsigned v)
	{
		return (short)min(v, (unsigned)SHRT_MAX);
	}
	template<> inline short saturate_cast<short>(float v)
	{
		int iv = round(v); return saturate_cast<short>(iv);
	}
	template<> inline short saturate_cast<short>(double v)
	{
		int iv = round(v); return saturate_cast<short>(iv);
	}

	template<> inline int saturate_cast<int>(float v) { return round(v); }
	template<> inline int saturate_cast<int>(double v) { return round(v); }

	// we intentionally do not clip negative numbers, to make -1 become 0xffffffff etc.
	template<> inline unsigned saturate_cast<unsigned>(float v){ return round(v); }
	template<> inline unsigned saturate_cast<unsigned>(double v) { return round(v); }
	



	
	// ----------------------------------- Time ---------------------------------//
	
	double get_real_time();

	class Timer
	{
	public:
		// default constructor, will call update() to record current timestamp.
		Timer();
		~Timer();

		// update current timestamp
		void update();

		// get elapsed time
		double get_elapsed_time_s();
		double get_elapsed_time_ms();
		double get_elapsed_time_us();

	private:

		double timestamp;
	};

	// ----------------------------------- I/O ---------------------------------//
	int waitkey(double ms = -1.0);

	/// <summary>
	/// Hold screen for key press
	/// </summary>
	inline void hold_screen()
	{
		info("Press any key to continue...");
		waitkey();
	}
	
	// --------------------------------- FILE IO -------------------------------//
	class BaseFile
	{

		// FILE_STATUS: | append? | write? | exists? | init? |
		enum FILE_STATUS { INIT = 0, NEW = 1, READ = 3, WRITE = 5, REWRITE = 7, APPEND = 15, };

	public:
		BaseFile();
		BaseFile(const String &file, std::ios_base::openmode openmode = std::ios_base::in);
		virtual ~BaseFile();

		static inline bool file_exists(const String& name)
		{
			std::ifstream f(name.c_str());
			if (f.good())
			{
				f.close();
				return true;
			}
			else {
				f.close();
				return false;
			}
		}

		void open();
		void open(const String &file, std::ios_base::openmode openmode = std::ios_base::in)
		{ 
			this->openmode = openmode; 
			this->path = file;
			open();
		}
		bool is_open() { return fp.is_open(); }

	protected:
		std::fstream	fp;
		String			path;
		int				flag;
		std::ios_base::openmode		openmode;

	};

	class TextFile : public BaseFile
	{
	public:
		TextFile(const String &file, std::ios_base::openmode openmode = std::ios_base::in) : BaseFile(file, openmode){};
		~TextFile();
		
		uint64 count_lines();
		int next_line(String &line);
		int goto_line(int n);
	};

	class BinaryFile : public BaseFile
	{
	public:
		BinaryFile();
		BinaryFile(const String &file, std::ios_base::openmode openmode = std::ios::in) : BaseFile(file, openmode) { openmode |= std::ios_base::binary; };
		~BinaryFile();

	};
	

	
}



#endif //_ZULIB_HPP_