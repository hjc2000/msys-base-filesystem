#pragma once
#include "base/container/iterator/IEnumerator.h"
#include "base/filesystem/filesystem.h"
#include "base/filesystem/Path.h"
#include <filesystem>
#include <string>
#include <unistd.h>
#include <windows.h>

namespace msys
{
	///
	/// @brief 目录条目递归迭代器。
	///
	///
	class RecursiveDirectoryEntryEnumerator :
		public base::IEnumerator<base::filesystem::DirectoryEntry const>
	{
	private:
		base::filesystem::DirectoryEntry _current;
		std::filesystem::recursive_directory_iterator _current_it;
		std::filesystem::recursive_directory_iterator _end_it;
		base::IEnumerator<base::filesystem::DirectoryEntry const>::Context_t _context{};

	public:
		RecursiveDirectoryEntryEnumerator(base::Path const &path)
		{
			std::string path_str = path.ToString();
			if (path_str == "")
			{
				path_str = "./";
			}

			_current_it = std::filesystem::recursive_directory_iterator{base::filesystem::ToWindowsLongPathString(path_str)};
		}

		///
		/// @brief 迭代器当前是否指向尾后元素。
		///
		/// @return
		///
		virtual bool IsEnd() const override
		{
			return _current_it == _end_it;
		}

		///
		/// @brief 获取当前值的引用。
		///
		/// @return ItemType&
		///
		virtual base::filesystem::DirectoryEntry const &CurrentValue() override
		{
			_current = base::filesystem::DirectoryEntry{base::filesystem::WindowsLongPathStringToPath(_current_it->path().string())};
			return _current;
		}

		///
		/// @brief 递增迭代器的位置。
		///
		///
		virtual void Add() override
		{
			++_current_it;
		}

		///
		/// @brief 派生类需要提供一个该对象。
		///
		/// @return
		///
		virtual base::IEnumerator<base::filesystem::DirectoryEntry const>::Context_t &Context() override
		{
			return _context;
		}
	};

} // namespace msys
