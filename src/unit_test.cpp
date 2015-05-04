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

void test_time()
{
	Println("\nTesting timer\n");
	zz::Timer t;
	int key = zz::waitkey(100000);
	Println("KEY: " << key);
	Println("Time waited: " << t.get_elapsed_time_ms() << "ms");
}

void test_msg()
{
	Println("\nTesting messages\n");
	Print("slk\n");
	Println("newline" << "sljflsd int: " << 1 << " double: " << 2.120192354534534 << " end it");
	Println_d("This message will only shown in debug mode!");
	Warning("warning content with number: " << 21);
	Error("Test error stop");
}

void test_throw()
{
	//throw zz::Exception("throw here!");
	//throw zz::WarnException("warning exception test!");
	Warning("something should not thrown if ZULIB_STRICT_WARNING is not defined.");
}

void test_exception()
{
	try
	{
		test_throw();
	}
	catch (zz::Exception ex)
	{
		Println(ex.what() << " catched!");
		abort();
	}
}

void test_file()
{
	Println("\nTesting text file\n");
	zz::TextFile tf("../../LICENSE");
	zz::Timer t;
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

String put_match(bool ret)
{
	return ret ? String("Yes") : String("No");
}

void test_dir()
{
	Println("\n\nTesting directories!");
	Println("Dir exist: " << zz::Path::is_exist("../../src"));
	Println("File exist: " << zz::Path::is_exist("../../README.md"));
	Println("No exist: " << zz::Path::is_exist("../../bla.jpg"));
	Println("Dir: " << zz::Path::is_directory("../../build"));
	Println("File: " << zz::Path::is_directory("../../LICENSE"));
	Println("No exist: " << zz::Path::is_directory("../../something.txt"));

	zz::Path p("/very//messy//path///////slfjd///xljfl.some_extension");
	Println("Orig: " << p.str());
	Println("Exist?: " << p.exist());
	Println("Dir: " << p.get_dir());
	Println("basename: " << p.get_basename());
	Println("extension: " << p.get_extension());

	Println("\nTest dir function");
	zz::Timer t;
	zz::Dir dir = zz::Dir("../../", 1);
	Println("Creation time: " << t.get_elapsed_time_ms() << "ms");
	std::vector<String> files = dir.get_files();
	std::vector<zz::Dir> subfolders = dir.get_subfolders();
	//for (size_t i = 0; i < files.size(); i++)
	//{
	//	Println("Sub files: " << files[i]);
	//}
	for (size_t i = 0; i < subfolders.size(); i++)
	{
		Println("Sub folders: " << subfolders[i].str());
	}

	Println("------------------------------");
	const char *wildcards[] = { "*.jpg", "*.txt" };
	Vecstr list = dir.list_files(Vecstr(wildcards, wildcards+2));
	Println("Overall time: " << t.get_elapsed_time_ms() << "ms");
	for (size_t i = 0; i < list.size(); i++)
	{
		Println(list[i]);
	}

	String toabs = zz::Path::get_real_path("../../LICENSE");
	Println("Get realpath : " << toabs);
	Println("Get cwd: " << zz::Path::get_cwd());

	Println(put_match(zz::Path::wildcard_match("g*ks", "geeks")));
	Println(put_match(zz::Path::wildcard_match("ge?ks*", "geeksforgeeks")));
	Println(put_match(zz::Path::wildcard_match("*pqrs", "pqrst")));
	Println(put_match(zz::Path::wildcard_match("*.jpg", "owjfsdlfjl.jpg")));

	Println("\nMake dir: ");
	Println(zz::Dir::mk_dir("../../newfolder/newfolder2/newfolder3"));
}

void test_progbar()
{
	Println("Testing progress bar!");
	zz::ProgBar pb(10000, "I'm progress bar");
	for (int i = 0; i < 10000; i++)
	{
		pb.step(1);
	}
}



void dev()
{
	for (int i = 0; i < 500; i++)
	{
		Println("NONSKDJFOIWEJFLJLKJDSLFKJ");
	}
	Println("ljsldjfowijefljsldjf");
	Println("ljsldjfowijefljsldjf");
	Println("ljsldjfowijefljsldjf");
	Println("ljsldjfowijefljsldjf");
	Println("ljsldjfowijefljsldjf");
	Println("ljsldjfowijefljsldjf");
	Println("ljsldjfowijefljsldjf");
	int x;
	int y;
	zz::get_cursor_position(&y, &x);
	Println("x: " << x << "y:" << y);
	zz::set_cursor_position(y - 2, x);
	std::cout << "\r                            " << std::endl;
	zz::set_cursor_position(y+2, x);
	Println(".........................");
}

int main()
{
	test_time();
	test_file();
	test_dir();
	//test_msg();
	//test_progbar();
	//dev();
	test_exception();
	

#ifdef _DEBUG
	zz::hold_screen();
#endif
	return 0;
}