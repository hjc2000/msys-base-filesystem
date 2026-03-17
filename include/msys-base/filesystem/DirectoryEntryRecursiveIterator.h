#pragma once
#include "base/container/iterator/IInputIterator.h"
#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include "base/string/define.h"
#include <filesystem>
#include <memory>
#include <stdexcept>

namespace msys_base::filesystem
{
	class DirectoryEntryRecursiveIterator :
		public base::IInputIterator<base::filesystem::DirectoryEntry const>
	{
	private:
		std::filesystem::recursive_directory_iterator _it{};
		base::filesystem::DirectoryEntry _current{};

	public:
		DirectoryEntryRecursiveIterator() = default;

		DirectoryEntryRecursiveIterator(base::Path const &path)
		{
			std::string path_str = path.ToString();

			if (path_str == "")
			{
				path_str = "./";
			}

			_it = std::filesystem::recursive_directory_iterator{base::filesystem::ToWindowsLongPathString(path_str)};
		}

		///
		/// @brief 派生类利用拷贝构造函数拷贝一个自己，然后返回。
		///
		/// @return
		///
		virtual std::shared_ptr<base::IInputIterator<base::filesystem::DirectoryEntry const>> Clone() override
		{
			std::shared_ptr<DirectoryEntryRecursiveIterator> ret{new DirectoryEntryRecursiveIterator{*this}};
			return ret;
		}

		///
		/// @brief 返回当前迭代器指向的对象的引用。
		///
		/// @return
		///
		virtual base::filesystem::DirectoryEntry const &Current() override
		{
			if (_it == std::filesystem::recursive_directory_iterator{})
			{
				throw std::runtime_error{CODE_POS_STR + "不能对 end 迭代器进行此操作。"};
			}

			_current = base::filesystem::DirectoryEntry{base::filesystem::WindowsLongPathStringToPath(_it->path().string())};
			return _current;
		}

		virtual void Increment() override
		{
			if (_it == std::filesystem::recursive_directory_iterator{})
			{
				throw std::runtime_error{CODE_POS_STR + "不能对 end 迭代器进行此操作。"};
			}

			++_it;
		}

		virtual bool Equal(base::IInputIterator<base::filesystem::DirectoryEntry const> const &other) const override
		{
			return _it == static_cast<DirectoryEntryRecursiveIterator const &>(other)._it;
		}
	};

} // namespace msys_base::filesystem
