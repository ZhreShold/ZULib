ZULib - Z's Utility Library
====================================

Personal C++ Util libraries.
Cross-Platform.
Mainly focus on system independent I/O operations. 

## License
MIT License.

## Usage
+ include "zuLib.hpp"


## Examples
+ Print message
```
Print("print to std::cout" << "U can chain input various types" << ", e.g. int" << 5 << " or double" << 3.1415926);
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


## Capability

+ TBD

## What's under Construction...

+ More TBD

## Acknowledgement

