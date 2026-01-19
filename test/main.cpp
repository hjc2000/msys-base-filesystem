#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include "base/string/define.h"
#include <iostream>

int main()
{
	{
		std::cout << CODE_POS_STR;

		std::cout << base::Path{"../../../../../"} << std::endl;

		std::cout << std::endl;
	}

	{
		std::cout << CODE_POS_STR;

		base::Path path = base::filesystem::CurrentPath();
		std::cout << path << std::endl;

		for (int i = 0; i < 9; i++)
		{
			path += "../";
			std::cout << path << std::endl;
		}

		std::cout << std::endl;
	}

	// for (base::filesystem::DirectoryEntry const &entry : base::filesystem::RecursiveDirectoryEntryEnumerable{path})
	// {
	// 	std::cout << entry.Path() << std::endl;
	// }

	return 0;
}
