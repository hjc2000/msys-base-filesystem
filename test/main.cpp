#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <iostream>

int main()
{
	base::Path path = "C:/Users/huang/disk/ti600_2TB/.temp/link";

	{
		bool result = base::filesystem::Exists(path);

		if (result)
		{
			std::cout << path << " 存在。" << std::endl;
		}
		else
		{
			std::cout << path << " 不存在。" << std::endl;
		}
	}

	{
		bool result = base::filesystem::IsSymbolicLinkDirectory(path);

		if (result)
		{
			std::cout << path << " 是一个符号链接目录。" << std::endl;
		}
		else
		{
			std::cout << path << " 不是一个符号链接目录。" << std::endl;
		}
	}

	{
		bool result = base::filesystem::IsReadable(path);

		if (result)
		{
			std::cout << path << " 可读。" << std::endl;
		}
		else
		{
			std::cout << path << " 不可读。" << std::endl;
		}
	}

	return 0;
}
