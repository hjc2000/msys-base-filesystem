#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <iostream>

int main()
{
	{
		base::Path path = "F:/cpp-lib-build-scripts/.clang-format";
		bool result = base::filesystem::IsSymbolicLinkDirectory(path);

		if (result)
		{
			std::cout << path << " 是符号链接目录。" << std::endl;
		}
		else
		{
			std::cout << path << " 不是符号链接目录。" << std::endl;
		}
	}

	{
		base::Path path = "C:/Users/huang/disk/ti600_2TB/目录链接";
		bool result = base::filesystem::IsSymbolicLinkDirectory(path);

		if (result)
		{
			std::cout << path << " 是符号链接目录。" << std::endl;
		}
		else
		{
			std::cout << path << " 不是符号链接目录。" << std::endl;
		}
	}

	return 0;
}
