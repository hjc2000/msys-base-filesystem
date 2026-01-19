#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <iostream>

int main()
{
	base::Path path = "C:/Users/huang/dev/.temp/1.png";
	base::filesystem::RemoveReadOnlyAttribute(path);

	return 0;
}
