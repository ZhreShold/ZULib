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
#include <algorithm>
#include <cctype>


#ifndef ZULIB_OS
#if defined(unix)        || defined(__unix)      || defined(__unix__) \
	|| defined(linux) || defined(__linux) || defined(__linux__) \
	|| defined(sun) || defined(__sun) \
	|| defined(BSD) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__FreeBSD__) || defined (__DragonFly__) \
	|| defined(sgi) || defined(__sgi) \
	|| (defined(__MACOSX__) || defined(__APPLE__)) \
	|| defined(__CYGWIN__) || defined(__MINGW32__)
#define ZULIB_OS 1
#elif defined(_MSC_VER) || defined(WIN32)  || defined(_WIN32) || defined(__WIN32__) \
	|| defined(WIN64)    || defined(_WIN64) || defined(__WIN64__)
#define ZULIB_OS 0
#else
#error Unable to support this unknown OS.
#endif
#elif !(ZULIB_OS==0 || ZULIB_OS==1)
#error ZULIB: Invalid configuration variable 'ZULIB_OS'.
#error (correct values are '0 = Microsoft Windows', '1 = Unix-like OS').
#endif

#if ZULIB_OS == 0
#include <Windows.h>
#include <direct.h>
#include <conio.h>
#include <io.h>
#elif ZULIB_OS == 1

// Apple Mac_OS_X specific
#if defined(__MACH__) || defined(__APPLE__)	
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#include <unistd.h>	/* POSIX flags */
#include <time.h>	/* clock_gettime(), time() */
#include <sys/time.h>	/* gethrtime(), gettimeofday() */
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>

#endif



namespace zz
{

	//////////////////////////////// Timer class ////////////////////////////////////
	/// <summary>
	/// Updates the current timestamp.
	/// </summary>
	void Timer::update()
	{
		timestamp_ = get_real_time();
	}

	
	Timer::Timer()
	{
		update();
	}

	Timer::~Timer()
	{
		timestamp_ = 0;
	}

	double Timer::get_real_time()
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

	/// <summary>
	/// Get the time elapsed in ms since last update.
	/// </summary>
	/// <param name="timeStamp">The last timestamp.</param>
	/// <returns>The time elapsed in ms</returns>
	double Timer::get_elapsed_time_ms()
	{
		return (get_real_time() - timestamp_) * 1000.0;
	}

	/// <summary>
	/// Get the time elapsed in second since last update.
	/// </summary>
	/// <returns>The time elapsed in second</returns>
	double Timer::get_elapsed_time_s()
	{
		return get_real_time() - timestamp_;
	}

	/// <summary>
	/// Get the time elapsed in us since last update.
	/// </summary>
	/// <returns>The time elapsed in us</returns>
	double Timer::get_elapsed_time_us()
	{
		return (get_real_time() - timestamp_) * 1000000.0;
	}


	//////////////////////////////// miscellaneous functions ////////////////////////////////////

	
	/// <summary>
	/// Check if stdout is in terminal/console
	/// </summary>
	/// <returns></returns>
	int is_atty()
	{
#if ZULIB_OS == 1
		return isatty(fileno(stdout));
#elif ZULIB_OS == 0
		return _isatty(_fileno(stdout));
#endif
	}

#if ZULIB_OS == 1
	// linux termios manipulation
	static inline int rd(const int fd)
	{
		const int RD_EOF = -1;
		const int RD_EIO = -2;

		unsigned char   buffer[4];
		ssize_t         n;


		while (1) {


			n = read(fd, buffer, 1);
			if (n > (ssize_t)0)
				return buffer[0];


			else
			if (n == (ssize_t)0)
				return RD_EOF;


			else
			if (n != (ssize_t)-1)
				return RD_EIO;


			else
			if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
				return RD_EIO;
		}
	}


	static inline int wr(const int fd, const char *const data, const size_t bytes)
	{
		const char       *head = data;
		const char *const tail = data + bytes;
		ssize_t           n;


		while (head < tail) {


			n = write(fd, head, (size_t)(tail - head));
			if (n >(ssize_t)0)
				head += n;


			else
			if (n != (ssize_t)-1)
				return EIO;


			else
			if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
				return errno;
		}


		return 0;
	}

#endif

	/// <summary>
	/// Get cursor position in terminal/console
	/// </summary>
	/// <param name="row">The row.</param>
	/// <param name="col">The col.</param>
	int get_cursor_position(int *row, int *col)
	{
		std::cout.flush();
#if ZULIB_OS == 0
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) 
		{
			*col = csbi.dwCursorPosition.X;
			*row = csbi.dwCursorPosition.Y;
			return 0;
		}
		return -1;
#elif ZULIB_OS == 1

		// first get current terminal id
		const char *dev;
		int         fd;


		dev = ttyname(STDIN_FILENO);
		if (!dev)
			dev = ttyname(STDOUT_FILENO);
		if (!dev)
			dev = ttyname(STDERR_FILENO);
		if (!dev) {
			errno = ENOTTY;
			return -1;
		}


		do {
			fd = open(dev, O_RDWR | O_NOCTTY);
		} while (fd == -1 && errno == EINTR);
		if (fd == -1)
			return -1;


		// try to get the cursor position
		int tty = fd;


		struct termios  saved, temporary;
		int             retval, result, rows, cols, saved_errno;


		/* Bad tty? */
		if (tty == -1)
			return ENOTTY;


		saved_errno = errno;


		/* Save current terminal settings. */
		do {
			result = tcgetattr(tty, &saved);
		} while (result == -1 && errno == EINTR);
		if (result == -1) {
			retval = errno;
			errno = saved_errno;
			return retval;
		}


		/* Get current terminal settings for basis, too. */
		do {
			result = tcgetattr(tty, &temporary);
		} while (result == -1 && errno == EINTR);
		if (result == -1) {
			retval = errno;
			errno = saved_errno;
			return retval;
		}


		/* Disable ICANON, ECHO, and CREAD. */
		temporary.c_lflag &= ~ICANON;
		temporary.c_lflag &= ~ECHO;
		temporary.c_cflag &= ~CREAD;


		/* This loop is only executed once. When broken out,
		* the terminal settings will be restored, and the function
		* will return retval to caller. It's better than goto.
		*/
		do {


			/* Set modified settings. */
			do {
				result = tcsetattr(tty, TCSANOW, &temporary);
			} while (result == -1 && errno == EINTR);
			if (result == -1) {
				retval = errno;
				break;
			}


			/* Request cursor coordinates from the terminal. */
			retval = wr(tty, "\033[6n", 4);
			if (retval)
				break;


			/* Assume coordinate reponse parsing fails. */
			retval = EIO;


			/* Expect an ESC. */
			result = rd(tty);
			if (result != 27)
				break;


			/* Expect [ after the ESC. */
			result = rd(tty);
			if (result != '[')
				break;


			/* Parse rows. */
			rows = 0;
			result = rd(tty);
			while (result >= '0' && result <= '9') {
				rows = 10 * rows + result - '0';
				result = rd(tty);
			}


			if (result != ';')
				break;


			/* Parse cols. */
			cols = 0;
			result = rd(tty);
			while (result >= '0' && result <= '9') {
				cols = 10 * cols + result - '0';
				result = rd(tty);
			}

			if (result != 'R')
				break;

			/* Success! */
			if (row)
				*row = rows;

			if (col)
				*col = cols;

			retval = 0;
		} while (0);


		/* Restore saved terminal settings. */
		do {
			result = tcsetattr(tty, TCSANOW, &saved);
		} while (result == -1 && errno == EINTR);
		if (result == -1 && !retval)
			retval = errno;


		/* Done. */
		return retval;

#endif
	}

	/// <summary>
	/// Set cursor positions at specified (row, col).
	/// </summary>
	/// <param name="row">The row.</param>
	/// <param name="col">The col.</param>
	void set_cursor_position(int row, int col)
	{
		std::cout.flush();
#if ZULIB_OS == 0
		//Initialize the coordinates
		COORD coord = { col, row };
		//Set the position
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#elif ZULIB_OS == 1
		std::cout << "\033[" << (row + 1) << ":" << (col+1) << "H";
		std::cout.flush();
#endif
	}
	
	void sleep(const int milliseconds) {
#if ZULIB_OS == 1
		struct timespec tv;
		tv.tv_sec = milliseconds/1000;
		tv.tv_nsec = (milliseconds%1000)*1000000;
		nanosleep(&tv,0);
#elif ZULIB_OS==0
		Sleep(milliseconds);
#endif
	}

#if ZULIB_OS == 1
	// toggle terminal mode in unix systems, for any key press to work
	void change_terminal_mode(int dir)
	{
		static struct termios oldt, newt;

		if (dir == 1)
		{
			tcgetattr(STDIN_FILENO, &oldt);
			newt = oldt;
			newt.c_lflag &= ~(ICANON | ECHO);
			tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		}
		else
			tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	}
#endif

	/// <summary>
	/// Detect keyboard hit, no block
	/// </summary>
	/// <returns>The key pressed or -1 if not(ASC-II not guaranteed, OS specific).</returns>
	int kb_hit()
	{
#if ZULIB_OS == 0
		if (_kbhit())
			return _getch();
		else
			return -1;

#elif ZULIB_OS == 1
		change_terminal_mode(1);

		struct timeval tv;
		fd_set rdfs;

		tv.tv_sec = 0;
		tv.tv_usec = 0;

		FD_ZERO(&rdfs);
		FD_SET (STDIN_FILENO, &rdfs);

		select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);

		int ret;
		if (FD_ISSET(STDIN_FILENO, &rdfs))
			ret = getchar();
		else
			ret = -1;

		change_terminal_mode(0);
		return ret;


#else
		return -1;		// unsupported OS
#endif
	}


	int waitkey(double ms)
	{
		int key = -1;
		double start = Timer::get_real_time();
		double end = start + ms / 1000;


		if (ms <= 0)
		{
			// disable timer if ms <= 0
			end = (std::numeric_limits<double>::max)();
		}

		while (key == -1)
		{
			key = kb_hit();
			//printf("%d", (int)get_elapsed_time_ms(start));
			sleep(1);
			if (Timer::get_real_time() >= end)
			{
				break;
			}
		}

		return key;
	}

	ProgBar::ProgBar(int size, const char* message)
	{
		if (size < 1)
		{
			throw zz::ArgException("ProgBar: task size < 1, invalid!");
			hide_ = 1;
			size_ = 0;
			return;
		}

		size_ = size;
		progress_ = 0;
		if (is_atty())
			hide_ = 0;
		else
			hide_ = 1;

		if (message != NULL)
		{
			std::cout << message << std::endl;
		}

	}

	ProgBar::~ProgBar()
	{
		std::cout << std::endl;
	}

	void ProgBar::step(int step)
	{
		if (!is_atty())
			return;

		progress_ += step;
		if (progress_ < 0) progress_ = 0;
		if (progress_ > size_) progress_ = size_;
		int percent = progress_ * 100 / size_;

		const int width = 57;
		char buf[width];

		// buf: [=======>        ] \0
		memset(buf, (int)' ', width);
		buf[0] = '[';
		buf[width - 1] = '\0';


		int pos = max(percent / 2, 0);
		memset(buf+1, (int)'=', pos);
		buf[pos + 1] = '>';
		buf[52] = ']';

		std::cout << "\r" << buf << "[ " << percent << "% ] [" << progress_ << "/" << size_ << "]";
		std::cout.flush();
	}


	BaseFile::BaseFile()
	{
		this->flag_ = INIT;
		this->openmode_ = std::ios::in;
	}

	BaseFile::BaseFile(String file, std::ios_base::openmode openmode)
	{

		flag_ = INIT;

		// detect if file exists
		if (Path::is_directory(file) == 0)
		{
			flag_ |= 0x02;
		}

		path_ = file;
		openmode_ = openmode;
		open();
		if (fp_.is_open())
		{
			flag_ |= 0x01;
		}
	}

	BaseFile::~BaseFile()
	{
		if (fp_.is_open())
		{
			fp_.close();
		}
		flag_ = INIT;
	}

	void BaseFile::open()
	{
		if (!fp_.is_open())
		{
			fp_.open(path_.c_str(), openmode_);
		}
		else
		{
			throw RuntimeException("Unexpected file stream already opened!");
			close();
			fp_.open(path_.c_str(), openmode_);
		}

		if (!fp_.is_open())
			throw IOException(TO_STRING("Failed to open file: " << path_));
	}

	TextFile::TextFile()
	{

	}

	int TextFile::count_lines()
	{
		std::ifstream fread(path_.c_str());
		if (!fread.is_open())
		{
			throw IOException("Failed to open file to count lines.");
			return -1;
		}

		const int bufSize = 1024 * 1024;	// using 1MB buffer
		std::vector<char> buf(bufSize);

		int ct = 0;
		int nbuf = 0;
		char last = 0;
		do
		{
			fread.read(&buf.front(), bufSize);
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

		return ct;
	}


	int TextFile::next_line(String &line)
	{
		if (!fp_.good() || !fp_.is_open())
			return -1;

		if (!fp_.eof())
		{
			std::getline(fp_, line);
			if (!line.empty())
			{
				return line.length();
			}
		}

		return 0;
	}


	int TextFile::goto_line(int n)
	{
		if (!fp_.good() || !fp_.is_open())
			return -1;

		fp_.seekg(std::ios::beg);

		if (n < 0)
		{
			Warning("n < 0, seek back to the first line");
			return 0;
		}

		if (n == 0)
		{
			return 0;
		}

		int i = 0;
		for (i = 0; i < n - 1; ++i)
		{

			fp_.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

			if (fp_.eof())
			{
				Warning("Reached end of file, line: " << (i + 1));
				break;
			}
		}

		return i+1;
	}

	BinaryFile::BinaryFile()
	{
		openmode_ |= std::ios_base::binary;
	}



	String Path::get_cwd()
	{
#ifdef _WIN32
		char* buffer = NULL;
		if ((buffer = _getcwd(NULL, 0)) == NULL)
		{
			throw IOException("Failed to get current working directory, try use default './' instead");
			return String("./");
		}
		else
		{
			String ret(buffer);
			free(buffer);
			return reform(ret);
		}
#else
#ifdef _PC_PATH_MAX
		long size;
		char *buf = NULL;
		char *ptr = NULL;

		size = pathconf(".", _PC_PATH_MAX);
		if ((buf = (char *)malloc((size_t)size)) != NULL)
		{
			ptr = getcwd(buf, (size_t)size);
		}

		if (ptr)
		{
			String ret(ptr);
			free(buf);
			return ret;
		}
		else
		{
			throw IOException("Failed to get current working directory, try use default './' instead");
			return String("./");
		}
#else
		// rarely happens, but in this case you need to preallocate memory 
		// and increase if buffer is not large enough
		size_t bufSize = 1024;
		const size_t maxBufSize = 8192;
		char* buf = NULL;
		char* rBuf;

		do 
		{
			buf = static_cast<char*>(realloc(buf, bufSize));
			rBuf = getcwd(buf, bufSize);
			if (!rBuf) 
			{
				if (errno == ERANGE) 
				{
					bufSize *= 2;
					if (bufSize > maxBufSize)
					{
						free(buf);
						break;
					}
				} 
				else 
				{
					free(buf);
					break;
				}
			}
			else
			{
				String ret(buf);
				free(buf);
				return ret;
			}
		} while (!rBuf);

		
		throw IOException("Failed to get current working directory, try use default './' instead");
		return String("./");

#endif

#endif
	}


	String Path::get_real_path(String relativePath)
	{
#ifdef _WIN32
		char *buffer = NULL;
		DWORD siz = MAX_PATH;
		buffer = (char*)malloc(siz);
		DWORD retval = GetFullPathNameA(relativePath.c_str(), siz, buffer, 0);
		if (retval == 0)
		{
			throw IOException(TO_STRING("Failed to get realpath with code: " << GetLastError()));
			return relativePath;
		}
		else if (retval > siz)
		{
			// need larger buffer
			free(buffer);
			siz = retval;
			buffer = (char*)malloc(siz);
			if (!buffer)
			{
				throw RuntimeException("Failed to allocate memory for path buffer!");
				return relativePath;
			}

			retval = GetFullPathNameA(relativePath.c_str(), siz, buffer, 0);
			if (retval > siz)
			{
				// There must be some problem
				throw RuntimeException("Failed to get realpath, may be too lomg!");
				return relativePath;
			}
		}
		
		// now good to return
		String ret(buffer);
		free(buffer);
		ret = reform(ret);
		return ret;
#else
#ifdef PATH_MAX
		char buffer[PATH_MAX+1];
		char *realPath = realpath(relativePath.c_str(), buffer);
		if (realPath)
		{
			String ret(realPath);
			return ret;
		}
		else
		{
			throw IOException("Failed to get realpath!");
			return relativePath;
		}
#else
		// try providing NULL buffer with good luck
		char *realPath = realpath(relativePath.c_str(), NULL);
		if (realPath)
		{
			String ret(realPath);
			free(realPath);
			return reform(ret);
		}
		else
		{
			throw IOException("Failed to get realpath!");
			return relativePath;
		}
#endif

#endif
	}


	int Path::is_exist(String path)
	{
#ifdef _WIN32
		DWORD ret = GetFileAttributesA(path.c_str());
		if (ret == INVALID_FILE_ATTRIBUTES)
		{
			const DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}

		return 1;
#else
		struct stat sb;
		if (stat(path.c_str(), &sb) == 0)
		{
			return 1;
		}
		else
		{
			return -1;
		}
#endif
	}


	int Path::is_directory(String path)
	{
#ifdef _WIN32
		if (is_exist(path) < 1)
		{
			return -1;
		}

		DWORD ret = GetFileAttributesA(path.c_str());
		if (ret & FILE_ATTRIBUTE_DIRECTORY)
		{
			return 1;
		}
		else
		{
			return 0;
		}
#else
		struct stat sb;
		if (stat(path.c_str(), &sb) == 0)
		{
			if (S_ISDIR(sb.st_mode))
			{
				return 1;
			}

			if (S_ISREG(sb.st_mode))
			{
				return 0;
			}
		}

		return -1;
#endif
	}



	String Path::reform(String orig)
	{
		std::replace(orig.begin(), orig.end(), '\\', '/');

		size_t pos;
		while ((pos = orig.find("//")) != String::npos)
		{
			orig.erase(orig.begin() + pos);
		}

		return orig;
	}

	String Path::get_dir()
	{
		String ret = this->path_;
		const size_t last_slash_idx = ret.find_last_of("\\/");
		if (std::string::npos != last_slash_idx)
		{
			ret.erase(last_slash_idx);
		}
		// in case a double slash
		while (*ret.rbegin() == '\\' || *ret.rbegin() == '/')
		{
			ret.erase(ret.length() - 1);
		}

		return ret;
	}


	String Path::get_basename()
	{
		String ret = this->path_;
		const size_t last_slash_idx = ret.find_last_of("\\/");
		if (std::string::npos != last_slash_idx)
		{
			ret.erase(0, last_slash_idx + 1);
		}

		// Remove extension if present.
		const size_t period_idx = ret.rfind('.');
		if (std::string::npos != period_idx)
		{
			ret.erase(period_idx);
		}
		return ret;
	}


	String Path::get_extension()
	{
		String ret = this->path_;
		// Remove extension if present.
		const size_t period_idx = ret.rfind('.');
		if (std::string::npos != period_idx)
		{
			ret.erase(0, period_idx+1);
		}
		else
		{
			ret = "";
		}
		return ret;
	}


	bool Path::wildcard_match(const char *first, const char * second)
	{
		// If we reach at the end of both strings, we are done
		if (*first == '\0' && *second == '\0')
			return true;

		// Make sure that the characters after '*' are present in second string.
		// This function assumes that the first string will not contain two
		// consecutive '*' 
		if (*first == '*' && *(first + 1) != '\0' && *second == '\0')
			return false;

		// If the first string contains '?', or current characters of both 
		// strings match
		if (*first == '?' || *first == *second)
			return wildcard_match(first + 1, second + 1);

		// If there is *, then there are two possibilities
		// a) We consider current character of second string
		// b) We ignore current character of second string.
		if (*first == '*')
			return wildcard_match(first + 1, second) || wildcard_match(first, second + 1);
		return false;
	}

	
	int Dir::mk_dir(String dir)
	{
		Path p(dir);
		String target = p.str();
		for (String::iterator i = target.begin(); i != target.end(); i++)
		{
			if (*i == '/')
			{
				String tmp = target.substr(0, i - target.begin() + 1);
#ifdef _WIN32
				CreateDirectoryA(tmp.c_str(), NULL);
#else
				mkdir(tmp.c_str(), 0755);
#endif
			}
		}

#ifdef _WIN32
		if (CreateDirectory(target.c_str(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError())
		{
			return 1;
		}

#else
		if (mkdir(target.c_str(), 0755) == 0 || errno == EEXIST)
		{
			return 1;
		}
#endif

		return 0;
	}

	void Dir::set_root(String path)
	{
		path = Path::get_real_path(path);

		if (Path::is_directory(path) < 1)
		{
			throw IOException(TO_STRING(path << " is not a valid directory"));
			String dir = Path(path).get_dir();
			if (Path::is_directory(dir))
			{
				Warning("use parent folder: " << dir << " as root directory instead!");
			}
			else
			{
				throw RuntimeException(TO_STRING("Failed to solve problem caused by path: " << path));
			}
		}
		
		if (*path.rbegin() == '/')
		{
			root_ = path.substr(0, path.size() - 1);
		}
		else
			root_ = path;
	}

	void Dir::search()
	{
		files_.clear();
		childs_.clear();

#ifdef _WIN32
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFileA((root_ + "/*").c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) 
		{
			do {
				// skip hidden files if not explicitly enabled
				if (showHidden_ <= 0 && (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
					continue;

				// read all (real) files or directories in current folder
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// get rid of "." and ".." folders
					if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
						continue;
					else
					{
						// subfolder
						Dir child(root_ + "/" + fd.cFileName, recursive_, showHidden_);
						childs_.push_back(child);
					}
				}
				else
					files_.push_back(fd.cFileName);

			} while (FindNextFileA(hFind, &fd));
			FindClose(hFind);
		}

#else

		DIR *dir = opendir(root_.c_str());
		
		if (dir == NULL)
		{
			throw zz::IOException(TO_STRING("Cannot open directory: " << root_ << " to read!"));
			return;
		}
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			if (showHidden_ <= 0 && entry->d_name[0] == '.')
			{
				// skip hidden files/directories
				continue;
			}

			if (entry->d_name[strlen(entry->d_name)-1] == '~')
			{
				// skip backup files end with '~'
				continue;
			}

			if (entry->d_type == DT_DIR)
			{
				// we always want to skip "." ".."
				if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
				{
					continue;
				}
				else
				{
					// subfolder
					Dir child(root_ + "/" + entry->d_name, recursive_, showHidden_);
					childs_.push_back(child);
				}
			}
			else if (entry->d_type == DT_REG || entry->d_type == DT_LNK)
			{
				files_.push_back(String(entry->d_name));
			}
		}

		if (closedir(dir) != 0)
			throw IOException(TO_STRING("Cannot close directory: " << root_));

#endif
	}


	Vecstr Dir::list_files(int absolutePath)
	{
		Vecstr fileList;
		if (absolutePath <= 0)
			fileList.insert(fileList.end(), files_.begin(), files_.end());
		else
		{
			for (Vecstr::iterator i = files_.begin(); i != files_.end(); i++)
			{
				String tmp = root_ + "/" + *i;
				fileList.push_back(tmp);
			}
		}
		// if resursive flag enabled, asking subfolders to provide their lists
		if (recursive_)
		{
			for (std::vector<Dir>::iterator i = childs_.begin(); i != childs_.end(); i++)
			{
				Vecstr listFromChild = i->list_files(1);
				if (absolutePath <= 0)
				{
					for (Vecstr::iterator j = listFromChild.begin(); j != listFromChild.end(); j++)
					{
						String prefix = root_ + "/";
						Path::remove_substring(prefix, *j);
					}
				}
				fileList.insert(fileList.end(), listFromChild.begin(), listFromChild.end());
			}
		}
		return fileList;
	}


	Vecstr Dir::list_files(Vecstr wildcards, int caseSensitive, int absolutePath)
	{
		Vecstr rawList = list_files(absolutePath);

		if (wildcards.size() < 1)
		{
			// no any filter, just return all file list
			return rawList;
		}

		if (caseSensitive < 1)
		{
			// not case sensitive
			for (Vecstr::iterator j = wildcards.begin(); j != wildcards.end(); j++)
			{
				std::transform(j->begin(), j->end(), j->begin(), tolower);
			}
		}

		Vecstr retList;
		for (Vecstr::iterator i = rawList.begin(); i != rawList.end(); i++)
		{
			for (Vecstr::iterator j = wildcards.begin(); j != wildcards.end(); j++)
			{
				if (Path::wildcard_match(j->c_str(), i->c_str()))
				{
					retList.push_back(*i);
				}
			}
		}

		return retList;
	}
}