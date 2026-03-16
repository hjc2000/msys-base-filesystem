#include "DirectoryEntryRecursiveIterator.h" // IWYU pragma: keep
#include <memory>

std::shared_ptr<base::IInputIterator<base::filesystem::DirectoryEntry const>>
base::detail::interface::filesystem::GetDirectoryEntryRecursiveBeginIterator(base::Path const &path)
{
	std::shared_ptr<base::filesystem::DirectoryEntryRecursiveIterator> ret{new base::filesystem::DirectoryEntryRecursiveIterator{path}};
	return ret;
}

std::shared_ptr<base::IInputIterator<base::filesystem::DirectoryEntry const>>
base::detail::interface::filesystem::GetDirectoryEntryRecursiveEndIterator()
{
	std::shared_ptr<base::filesystem::DirectoryEntryRecursiveIterator> ret{new base::filesystem::DirectoryEntryRecursiveIterator{}};
	return ret;
}
