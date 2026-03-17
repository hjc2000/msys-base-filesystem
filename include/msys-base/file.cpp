#include "base/filesystem/file.h"
#include "msys-base/FileStream.h"

std::shared_ptr<base::Stream> base::file::OpenOrCreate(base::Path const &path)
{
	return msys_base::FileStream::OpenOrCreate(path.ToString());
}

std::shared_ptr<base::Stream> base::file::OpenReadOnly(base::Path const &path)
{
	return msys_base::FileStream::OpenReadOnly(path.ToString());
}

std::shared_ptr<base::Stream> base::file::OpenExisting(base::Path const &path)
{
	return msys_base::FileStream::OpenExisting(path.ToString());
}

std::shared_ptr<base::Stream> base::file::CreateNewAnyway(base::Path const &path)
{
	return msys_base::FileStream::CreateNewAnyway(path.ToString());
}
