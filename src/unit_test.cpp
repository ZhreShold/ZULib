/*
/#   Script File: unit_test.cpp
/#
/#   Description:
/#
/#   Unit test module
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
using namespace zz;

void test_time()
{
	Println("\nTesting timer\n");
	Timer t;
	waitkey(1000);
	Println("Time waited: " << t.get_elapsed_time_ms() << "ms");
}

void test_msg()
{
	Println("\nTesting messages\n");
	Print("slk\n");
	Println("newline" << "sljflsd int: " << 1 << " double: " << 2.120192354534534 << " end it");
	Warning("warning content with number: " << 21);
	Error("Test error stop");
}

void test_file()
{
	Println("\nTesting text file\n");
	TextFile tf("../../LICENSE");
	Timer t;
	Println(tf.count_lines());
	Println("Time elapsed: " << t.get_elapsed_time_ms() << "ms");

	String line;
	int s = 0;
	int limit = 10;

	int ret = tf.goto_line(3);
	Println("jumped to line: " << ret);

	s = tf.next_line(line);
	while ( s >= 0 && limit > 0)
	{
		Println(line);
		s = tf.next_line(line);
		limit--;
	}
}

void test_dir()
{
	Println("\n\nTesting directories!");
	Println("Dir exist: " << zz::Path::is_exist("D:/Dev/vs_projects/ZULib/cache"));
	Println("File exist: " << zz::Path::is_exist("D:/Dev/vs_projects/ZULib/cache/graf.jpg"));
	Println("No exist: " << zz::Path::is_exist("D:/Dev/vs_projects/ZULib/bla.jpg"));
	Println("Dir: " << zz::Path::is_directory("D:/Dev/vs_projects/ZULib/cache"));
	Println("File: " << zz::Path::is_directory("D:/Dev/vs_projects/ZULib/cache/key1.txt"));
	Println("No exist: " << zz::Path::is_directory("D:/Dev/vs_projects/ZULib/cachecache/something.txt"));

	zz::Path p("/woj/slfjd/xljfl");
	Println("Orig: " << p.str());
	Println("Exist?: " << p.exist());
	Println("Dir: " << p.get_dir());
	Println("basename: " << p.get_basename());
	Println("extension: " << p.get_extension());

	Println("\nTest dir function");
	Timer t;
	zz::Dir dir = zz::Dir("../../", 1);
	Println("Creation time: " << t.get_elapsed_time_ms() << "ms");
	std::vector<String> files = dir.get_files();
	std::vector<Dir> subfolders = dir.get_subfolders();
	for (int i = 0; i < files.size(); i++)
	{
		Println("Sub files: " << files[i]);
	}
	for (int i = 0; i < subfolders.size(); i++)
	{
		Println("Sub folders: " << subfolders[i].str());
	}

	Println("------------------------------");
	Vecstr list = dir.list_files();
	Println("Overall time: " << t.get_elapsed_time_ms() << "ms");
	for (int i = 0; i < list.size(); i++)
	{
		Println(list[i]);
	}

	String toabs = zz::Path::get_real_path("..\\..\\ojwf\\wo.jpg");
	Println("Get realpath : " << toabs);
	Println("Get cwd: " << zz::Path::get_cwd());
}

int main()
{
	test_time();
	test_file();
	test_dir();
	test_msg();
	

#ifdef _DEBUG
	hold_screen();
#endif
	return 0;
}