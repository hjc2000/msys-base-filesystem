#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <iostream>

int main()
{
	base::Path path = "C:/Users/huang/disk/ti600_2TB/.temp/link";
	base::filesystem::Remove(path);

	return 0;
}
