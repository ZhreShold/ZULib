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
	Println("newline" << "sljflsd" << 1 << "double" << 2.120192354534534 << "end");
	Warning("warning content with number" << 21);
	Error("intensional error stop");
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

int main()
{
	test_time();
	test_file();
	test_msg();
	

#ifdef _DEBUG
	hold_screen();
#endif
	return 0;
}