#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <iostream>

int main()
{
	base::Path path = "C:/Users/huang/dev/.temp/test";
	base::filesystem::RemoveReadOnlyAttributeRecursively(path);

	return 0;
}
