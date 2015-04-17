/*
/#   Script File: zuLib.cpp
/#
/#   Description:
/#
/#   zuLib implementation
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

#include "zuLib.hpp"
#include <limits>

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>	/* POSIX flags */
#include <time.h>	/* clock_gettime(), time() */
#include <sys/time.h>	/* gethrtime(), gettimeofday() */

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#else
#error "Unable to define get_real_time( ) for an unknown OS."
#endif

#if defined(_WIN32)
#include <conio.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>	/* POSIX flags */
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

struct termios orig_termios;
int termios_status = 0;

void reset_terminal_mode()
{
	tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
	struct termios new_termios;

	if (termios_status == 0)
	{
		/* take two copies - one for now, one for later */
		tcgetattr(0, &orig_termios);
		termios_status = 1;
	}
	memcpy(&new_termios, &orig_termios, sizeof(new_termios));

	/* register cleanup handler, and set the new terminal mode */
	atexit(reset_terminal_mode);
	cfmakeraw(&new_termios);
	tcsetattr(0, TCSANOW, &new_termios);
}

int get_last_key()
{
	int r;
	uchar c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	}
	else {
		reset_terminal_mode();
		return c;
	}
}



#else
#error "Unable to define get_key( ) for an unknown OS."
#endif

namespace zz
{
	/// <summary>
	/// Time is measured since an arbitrary and OS-dependent start time.
	/// The returned real time is only useful for computing an elapsed time
	/// between two calls to this function.
	/// </summary>
	/// <returns>Returns the real time, in seconds, or -1.0 if an error occurred.</returns>
	double get_real_time()
	{
#if defined(_WIN32)
		FILETIME tm;
		ULONGLONG t;
#if defined(NTDDI_WIN8) && NTDDI_VERSION >= NTDDI_WIN8 && 0
		/* Windows 8, Windows Server 2012 and later. ---------------- */
		GetSystemTimePreciseAsFileTime(&tm);
#else
		/* Windows 2000 and later. ---------------------------------- */
		GetSystemTimeAsFileTime(&tm);
#endif
		t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
		return (double)t / 10000000.0;

#elif (defined(__hpux) || defined(hpux)) || ((defined(__sun__) || defined(__sun) || defined(sun)) && (defined(__SVR4) || defined(__svr4__)))
		/* HP-UX, Solaris. ------------------------------------------ */
		return (double)gethrtime() / 1000000000.0;

#elif defined(__MACH__) && defined(__APPLE__)
		/* OSX. ----------------------------------------------------- */
		static double timeConvert = 0.0;
		if (timeConvert == 0.0)
		{
			mach_timebase_info_data_t timeBase;
			(void)mach_timebase_info(&timeBase);
			timeConvert = (double)timeBase.numer /
				(double)timeBase.denom /
				1000000000.0;
		}
		return (double)mach_absolute_time() * timeConvert;

#elif defined(_POSIX_VERSION)
		/* POSIX. --------------------------------------------------- */
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
		{
			struct timespec ts;
#if defined(CLOCK_MONOTONIC_PRECISE)
			/* BSD. --------------------------------------------- */
			const clockid_t id = CLOCK_MONOTONIC_PRECISE;
#elif defined(CLOCK_MONOTONIC_RAW)
			/* Linux. ------------------------------------------- */
			const clockid_t id = CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_HIGHRES)
			/* Solaris. ----------------------------------------- */
			const clockid_t id = CLOCK_HIGHRES;
#elif defined(CLOCK_MONOTONIC)
			/* AIX, BSD, Linux, POSIX, Solaris. ----------------- */
			const clockid_t id = CLOCK_MONOTONIC;
#elif defined(CLOCK_REALTIME)
			/* AIX, BSD, HP-UX, Linux, POSIX. ------------------- */
			const clockid_t id = CLOCK_REALTIME;
#else
			const clockid_t id = (clockid_t)-1;	/* Unknown. */
#endif /* CLOCK_* */
			if (id != (clockid_t)-1 && clock_gettime(id, &ts) != -1)
				return (double)ts.tv_sec +
				(double)ts.tv_nsec / 1000000000.0;
			/* Fall thru. */
		}
#endif /* _POSIX_TIMERS */

		/* AIX, BSD, Cygwin, HP-UX, Linux, OSX, POSIX, Solaris. ----- */
		struct timeval tm;
		gettimeofday(&tm, NULL);
		return (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
#else
		return -1.0;		/* Failed. */
#endif
	}

	//////////////////////////////// Timer class ////////////////////////////////////
	/// <summary>
	/// Updates the current timestamp.
	/// </summary>
	void Timer::update()
	{
		timestamp = get_real_time();
	}

	/// <summary>
	/// Default constructor, will call update() to record current timestamp.
	/// </summary>
	Timer::Timer()
	{
		update();
	}

	Timer::~Timer()
	{
		timestamp = 0;
	}

	/// <summary>
	/// Get the time elapsed in ms since last update.
	/// </summary>
	/// <param name="timeStamp">The last timestamp.</param>
	/// <returns>The time elapsed in ms</returns>
	double Timer::get_elapsed_time_ms()
	{
		return (get_real_time() - timestamp) * 1000.0;
	}

	/// <summary>
	/// Get the time elapsed in second since last update.
	/// </summary>
	/// <returns>The time elapsed in second</returns>
	double Timer::get_elapsed_time_s()
	{
		return get_real_time() - timestamp;
	}

	/// <summary>
	/// Get the time elapsed in us since last update.
	/// </summary>
	/// <returns>The time elapsed in us</returns>
	double Timer::get_elapsed_time_us()
	{
		return (get_real_time() - timestamp) * 1000000.0;
	}

	/// <summary>
	/// Get asynchronized keyboard press.
	/// </summary>
	/// <returns>
	/// The pressed key value.
	/// </returns>
	int get_key()
	{
#if defined(_WIN32)
		return _getch();

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
		int buf = -1;
		struct termios old = { 0 };
		if (tcgetattr(0, &old) < 0)
			perror("tcsetattr()");
		old.c_lflag &= ~ICANON;
		old.c_lflag &= ~ECHO;
		old.c_cc[VMIN] = 1;
		old.c_cc[VTIME] = 0;
		if (tcsetattr(0, TCSANOW, &old) < 0)
			perror("tcsetattr ICANON");
		if (read(0, &buf, 1) < 0)
			perror("read()");
		old.c_lflag |= ICANON;
		old.c_lflag |= ECHO;
		if (tcsetattr(0, TCSADRAIN, &old) < 0)
			perror("tcsetattr ~ICANON");
		return (buf);

#else
		return -1;		// failed
#endif
	}


	/// <summary>
	/// Detect keyboard hit, no wait
	/// </summary>
	/// <returns>The key pressed or -1 if not(ASC-II not guaranteed, platform specific).</returns>
	int kb_hit()
	{
#if defined(_WIN32)
		if (_kbhit())
			return _getch();
		else
			return -1;

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))


		//set_conio_terminal_mode();

		struct timeval tv = { 0L, 0L };
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		if (select(1, &fds, NULL, NULL, &tv))
		{
			return get_last_key();
		}
		else
		{
			return -1;
		}

#else
		return -1;		// unsupported OS
#endif
	}

	/// <summary>
	/// wait for the specficed ms until keypressed
	/// </summary>
	/// <param name="ms">The ms to wait.</param>
	/// <returns>The key pressed(ASC-II not guaranteed).</returns>
	int waitkey(double ms)
	{
		int key = -1;
		double start = get_real_time();
		double end = start + ms / 1000;

#if defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
		set_conio_terminal_mode();
#endif

		if (ms <= 0)
		{
			// disable timer if ms <= 0
			end = (std::numeric_limits<double>::max)();
		}

		while (key == -1)
		{
			key = kb_hit();
			//printf("%d", (int)get_elapsed_time_ms(start));
			if (get_real_time() >= end)
			{
				break;
			}
		}
		//std::cin.get();
#if defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
		reset_terminal_mode();
#endif
		return key;
	}




	BaseFile::BaseFile()
	{
		this->flag = INIT;
		this->openmode = std::ios::in;
	}

	BaseFile::BaseFile(const String &file, std::ios_base::openmode openmode)
	{

		this->flag = INIT;

		// detect if file exists
		if (file_exists(file))
		{
			flag |= 0x02;
		}

		this->path = file;
		this->openmode = openmode;
		open();
		if (fp.is_open())
		{
			flag |= 0x01;
		}
	}

	BaseFile::~BaseFile()
	{
		if (fp.is_open())
		{
			fp.close();
		}
		flag = INIT;
	}

	void BaseFile::open()
	{
		if (!fp.is_open())
		{
			fp.open(path.c_str(), this->openmode);
		}
		else
		{
			warning("File stream already opened!");
		}
	}


	TextFile::~TextFile()
	{

	}

	/// <summary>
	/// Count number of lines in text file. Note that \r(CR) only old Mac OS won't be supported.
	/// </summary>
	/// <returns>Number of Lines</returns>
	uint64 TextFile::count_lines()
	{
		const int bufSize = 1024 * 1024;	// using 1MB buffer
		char* buf = new (std::nothrow) char[bufSize];
		if (!buf)
		{
			error("Memory allocation : failed to create buffer.");
		}

		std::ifstream fread(path.c_str());
		if (!fread.is_open())
		{
			error("Failed to open file.");
		}

		uint64 ct = 0;
		int nbuf = 0;
		char last = 0;
		do
		{
			fread.read(buf, bufSize);
			nbuf = static_cast<int>(fread.gcount());
			for (int i = 0; i < nbuf; i++)
			{
				last = buf[i];
				if (last == '\n')
				{
					ct++;
				}
			}
		} while (nbuf > 0);

		if (last != '\n')
			ct++;

		delete buf;
		fread.close();
		return ct;
	}

	/// <summary>
	/// Get next line of opened file
	/// </summary>
	/// <param name="line">The next line.</param>
	/// <returns>Number of characters in line if success, -1 or 0 if fail</returns>
	int TextFile::next_line(String &line)
	{
		if (!fp.good() || !fp.is_open())
			return -1;

		if (!fp.eof())
		{
			std::getline(fp, line);
			if (!line.empty())
			{
				return line.length();
			}
		}

		return 0;
	}

	int TextFile::goto_line(int n)
	{
		if (!fp.good() || !fp.is_open())
			return -1;

		fp.seekg(std::ios::beg);

		if (n < 0)
		{
			warning("Seek back to the first line");
			return 0;
		}

		if (n == 0)
		{
			return 0;
		}

		int i = 0;
		for (i = 0; i < n - 1; ++i)
		{

			fp.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

			if (fp.eof())
			{
				Warning("Reached end of file, line: " << (i + 1));
				break;
			}
		}

		return i+1;
	}

	BinaryFile::BinaryFile()
	{
		this->openmode |= std::ios_base::binary;
	}

	BinaryFile::~BinaryFile()
	{

	}
}