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

#if defined(_WIN32)
#include <Windows.h>
#include <direct.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>	/* POSIX flags */
#include <time.h>	/* clock_gettime(), time() */
#include <sys/time.h>	/* gethrtime(), gettimeofday() */
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#else
#error "Unable to support this unknown OS."
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
		timestamp_ = get_real_time();
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
		timestamp_ = 0;
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
			warning("File stream already opened!");
		}
	}


	TextFile::~TextFile()
	{

	}

	uint64 TextFile::count_lines()
	{
		const int bufSize = 1024 * 1024;	// using 1MB buffer
		char* buf = new (std::nothrow) char[bufSize];
		if (!buf)
		{
			error("Memory allocation : failed to create buffer.");
		}

		std::ifstream fread(path_.c_str());
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

	BinaryFile::~BinaryFile()
	{

	}


	String Path::get_cwd()
	{
#ifdef _WIN32
		char* buffer = NULL;
		if ((buffer = _getcwd(NULL, 0)) == NULL)
		{
			Warning("Failed to get current working directory, return default './'");
			return String("./");
		}
		else
		{
			String ret(buffer);
			free(buffer);
			return reform_backslash(ret);
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
			Warning("Failed to get current working directory, return default './'");
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

		
		Warning("Failed to get current working directory, return default './'");
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
			Warning("Failed to get realpath with code: " << GetLastError());
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
				Warning("Failed to allocate memory for path buffer!");
				return relativePath;
			}

			retval = GetFullPathNameA(relativePath.c_str(), siz, buffer, 0);
			if (retval > siz)
			{
				// There must be some problem
				Warning("Failed to get realpath, may be too lomg!");
				return relativePath;
			}
		}
		
		// now good to return
		String ret(buffer);
		free(buffer);
		ret = reform_backslash(ret);
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
			Warning("Failed to get realpath!");
			return relativePath;
		}
#else
		// try providing NULL buffer with good luck
		char *realPath = realpath(relativePath.c_str(), NULL);
		if (realPath)
		{
			String ret(realPath);
			free(realPath);
			return ret;
		}
		else
		{
			Warning("Failed to get realpath!");
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

	String Path::reform_backslash(String orig)
	{
		std::replace(orig.begin(), orig.end(), '\\', '/');
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


	void Dir::set_root(String path)
	{
		path = Path::get_real_path(path);

		if (Path::is_directory(path) < 1)
		{
			Warning(path << " is not a valid directory");
			String dir = Path(path).get_dir();
			if (Path::is_directory(dir))
			{
				Warning("use parent folder: " << dir << " as root directory instead!");
			}
			else
			{
				Error("Failed to solve problem caused by path: " << path);
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
			Warning("Cannot open directory: " << root_ << " to read!");
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
			Error("Cannot close directory: " << root_ );

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
}