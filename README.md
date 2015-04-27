ZULib - Z's Utility Library
====================================

Personal C++ Util libraries.
Cross-Platform.
Mainly focus on system independent I/O operations. 

## License
MIT License.

## Usage
+ include "zuLib.hpp" in source code

## Compatibility
+ C++03 compatible with MSVC/GCC/Clang
+ Windows XP/Server 2003? or newer
+ Mainstream linux
+ Mac OS X
+ Some POSIX compiant platform

## Examples
+ Print message (Currently use Macro to hack chain input in order to support pre-c++11 compilers.)
```
Print("print to std::cout" << "U can chain input various types" << ", e.g. int: " << 5 << " or double: " << 3.1415926);
Println("this will automatically start new line at end. Print will not, otherwise same functionality.");
Warning("print to std::cerr with not fatal error, will continue running");
Error("Will print error msg to std::cerr and exit");
```
+ Open text file
```
// open a text file "test.txt"
zz::TextFile tf("test.txt");
// count number of lines in the text file, cautious: may take a while for extremely large files
Println(tf.count_lines()); 
// jump to line 20
int ret = tf.goto_line(20);
Println("jumped to line: " << ret);
// print lines
String line;
while ( tf.next_line(line) >= 0)
{
	Println(line);
}
```
+ Time benchmark
```
// use Timer class
Println("Testing Timer");
zz:Timer t;
for (int i = 0; i < 1000; i++)
{
    execute_some_function();
}
Println("Time elapsed: " << t.get_elapsed_time_ms() << "ms");
// also a asynchronus waitkey function provided
// wait 1000 ms key strok
int key = waitkey(1000);
```
+ Directory handling
```
// Path utility
zz::Path path("../../cache/test.jpg");
// detect if exist
Println("Exist? " << path.exist());
// detect if is a directory or file
Println("Is dir? " << path.dir());
// get basename w/o directory or extension
Println("Basename: " << path.get_basename());
// get root directory
Println("Directory: " << path.get_dir());
// get extension
Println("Extension: " << path.get_extension());
// get absolute path from relative path
Println("Absolute path: " << zz::Path::get_real_path(path.str());
// get current working directory
Println("CWD: " << zz::Path::get_cwd());
//----------------------------------------//
// Directory
zz::Dir dir("../../", RECURSIVE, SHOW_HIDDEN);
// list all files
Vecstr list = dir.list_files();
for (size_t i = 0; i < list.size(); i++)
{
    Println(list[i]);
}
```

## Capability

+ TBD

## What's under Construction...

+ More TBD

## Acknowledgement

+ Pending
