#pragma once
#include "base/container/iterator/IInputIterator.h"
#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <filesystem>
#include <memory>

namespace base
{
	class DirectoryEntryIterator :
		public base::IInputIterator<base::filesystem::DirectoryEntry const>
	{
	private:
		std::filesystem::directory_iterator _it{};
		base::filesystem::DirectoryEntry _current{};

	public:
		DirectoryEntryIterator() = default;

		DirectoryEntryIterator(base::Path const &path)
		{
			_it = std::filesystem::directory_iterator{base::filesystem::ToWindowsLongPathString(path)};
		}

		///
		/// @brief 派生类利用拷贝构造函数拷贝一个自己，然后返回。
		///
		/// @return
		///
		virtual std::shared_ptr<base::IInputIterator<base::filesystem::DirectoryEntry const>> Clone() override
		{
			std::shared_ptr<DirectoryEntryIterator> ret{new DirectoryEntryIterator{*this}};
			return ret;
		}

		///
		/// @brief 返回当前迭代器指向的对象的引用。
		///
		/// @return
		///
		virtual base::filesystem::DirectoryEntry const &Current() override
		{
			_current = base::filesystem::DirectoryEntry{base::filesystem::WindowsLongPathStringToPath(_it->path().string())};
			return _current;
		}

		virtual void Increment() override
		{
			++_it;
		}

		virtual bool Equal(base::IInputIterator<base::filesystem::DirectoryEntry const> const &other) const override
		{
			return _it == static_cast<DirectoryEntryIterator const &>(other)._it;
		}
	};

} // namespace base
