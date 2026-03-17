#include "base/filesystem/access_permission_check.h"
#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <string>
#include <unistd.h>

bool base::filesystem::IsReadable(base::Path const &path)
{
	return access(base::filesystem::ToWindowsLongPathString(path).c_str(), R_OK) == 0;
}

bool base::filesystem::IsWriteable(base::Path const &path)
{
	return access(base::filesystem::ToWindowsLongPathString(path).c_str(), W_OK) == 0;
}

bool base::filesystem::IsExcuteable(base::Path const &path)
{
	return access(base::filesystem::ToWindowsLongPathString(path).c_str(), X_OK) == 0;
}
