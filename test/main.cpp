#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include "base/string/define.h"
#include <iostream>

int main()
{
	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		std::cout << base::Path{"../../../../../"} << std::endl;
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::Path path = base::filesystem::CurrentPath();
		std::cout << path << std::endl;

		for (int i = 0; i < 100; i++)
		{
			path += "../";
			std::cout << path << std::endl;
		}
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::Path path = "/a/b/c";
		std::cout << path << std::endl;

		for (int i = 0; i < 10; i++)
		{
			path += "../";
			std::cout << path << std::endl;
		}
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::Path path = "a/b/c";
		std::cout << path << std::endl;

		for (int i = 0; i < 10; i++)
		{
			path += "../";
			std::cout << path << std::endl;
		}
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::Path path = "a/b/../../../../c/../..";
		std::cout << path << std::endl;
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::Path path = base::filesystem::ReadSymboliclink("C:/Users/huang/dev/cpp-lib-build-scripts/msys-clang/.repos/msys-base-filesystem/CMakeLists.txt");
		std::cout << path << std::endl;
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	return 0;
}
