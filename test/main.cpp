#include "base/filesystem/filesystem.h"
#include "base/filesystem/number_directory/NumberDirectoryCollector.h"
#include "base/filesystem/number_directory/NumberRangeDirectoryCollector.h"
#include "base/filesystem/Path.h"
#include "base/math/interval/ClosedInterval.h"
#include "base/stream/MemoryStream.h"
#include "base/stream/Span.h"
#include "base/string/define.h"
#include "base/string/encoding/Utf16LeWriter.h"
#include "base/time/UtcHourOffset.h"
#include "msys-base/windows_api.h"
#include <consoleapi.h>
#include <cstdint>
#include <iostream>
#include <string>

int main()
{
	// 测试块。
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

	// 测试块。
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

	// 测试块。
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

	// 测试块。
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

	// 测试块。
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

	// 测试块。
	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::Path path = base::filesystem::ReadSymboliclink("C:/Users/huang/disk/ti600_2TB/cpp-lib-build-scripts/msys-clang/.repos/msys-base/CMakeLists.txt");
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

	{
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		char16_t buffer[1024];

		base::Span buffer_span{
			reinterpret_cast<uint8_t *>(buffer),
			sizeof(buffer),
		};

		base::MemoryStream stream{buffer_span};
		base::string::encoding::Utf16LeWriter writer{stream};

		std::u32string str{U"测试中文。🆒😥\n"};
		writer.Write(str);

		uint32_t actual_number_of_chars_written;

		WriteConsoleW(handle,
					  stream.Span().Buffer(),
					  writer.Utf16UnitCount(),
					  reinterpret_cast<LPDWORD>(&actual_number_of_chars_written),
					  nullptr);

		std::cout << "actual_number_of_chars_written: " << actual_number_of_chars_written << std::endl;
	}

	// 测试块。
	try
	{
		// std::cout << std::endl;
		// std::cout << "======================================================" << std::endl;
		// std::cout << CODE_POS_STR;

		// base::filesystem::DirectoryEntryCollector collector{base::Path{}};

		// for (base::filesystem::DirectoryEntry const &entry : collector)
		// {
		// 	std::cout << entry.Path() << std::endl;
		// }
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	// 测试块。
	try
	{
		// std::cout << std::endl;
		// std::cout << "======================================================" << std::endl;
		// std::cout << CODE_POS_STR;

		// base::filesystem::DirectoryEntryRecursiveCollector collector{base::Path{"../"}};

		// for (base::filesystem::DirectoryEntry const &entry : collector)
		// {
		// 	std::cout << entry.Path() << std::endl;
		// }
	}
	catch (std::exception const &e)
	{
		std::cerr << CODE_POS_STR << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << CODE_POS_STR << "未知异常。" << std::endl;
	}

	// 测试块。
	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::filesystem::NumberDirectoryCollector collector{
			base::Path{"C:/Users/huang/disk/ti600_2TB/.temp"},
			nullptr,
		};

		for (base::filesystem::DirectoryEntry const &entry : collector)
		{
			std::cout << entry.Path() << std::endl;
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

	// 测试块。
	try
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << CODE_POS_STR;

		base::ClosedInterval<int64_t> interval{2026, 2027};

		base::filesystem::NumberRangeDirectoryCollector collector{
			base::Path{"C:/Users/huang/disk/ti600_2TB/.temp"},
			interval,
			nullptr,
		};

		for (base::filesystem::DirectoryEntry const &entry : collector)
		{
			std::cout << entry.Path() << std::endl;
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

	return 0;
}
