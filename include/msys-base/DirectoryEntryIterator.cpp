#include "DirectoryEntryIterator.h" // IWYU pragma: keep
#include <memory>

std::shared_ptr<base::IInputIterator<base::filesystem::DirectoryEntry const>>
base::detail::interface::filesystem::GetDirectoryEntryBeginIterator(base::Path const &path)
{
	std::shared_ptr<base::DirectoryEntryIterator> ret{new base::DirectoryEntryIterator{path}};
	return ret;
}

std::shared_ptr<base::IInputIterator<base::filesystem::DirectoryEntry const>>
base::detail::interface::filesystem::GetDirectoryEntryEndIterator()
{
	std::shared_ptr<base::DirectoryEntryIterator> ret{new base::DirectoryEntryIterator{}};
	return ret;
}
