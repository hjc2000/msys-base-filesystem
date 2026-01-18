#include "base/filesystem/filesystem.h"
#include "base/container/iterator/IEnumerator.h"
#include "base/container/Range.h"
#include "base/filesystem/Path.h"
#include "base/string/define.h"
#include "base/string/String.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
#include <windows.h>

#undef CreateDirectory

namespace
{
	class HandleGuard
	{
	private:
		HANDLE _handle = INVALID_HANDLE_VALUE;

	public:
		HandleGuard(HANDLE handle)
			: _handle{handle}
		{
		}

		~HandleGuard()
		{
			CloseHandle(_handle);
			_handle = INVALID_HANDLE_VALUE;
		}
	};

	std::string ToWindowsLongPathString(base::Path const &path)
	{
		try
		{
			base::Path absolute_path = base::filesystem::ToAbsolutePath(path);
			base::String absolute_path_string = absolute_path.ToString();
			absolute_path_string.Replace("/", "\\");
			absolute_path_string = "\\\\?\\" + absolute_path_string;
			return absolute_path_string.StdString();
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error{CODE_POS_STR + e.what()};
		}
		catch (...)
		{
			throw std::runtime_error{CODE_POS_STR + "未知的异常。"};
		}
	}

	base::Path WindowsLongPathStringToPath(std::string const &path_string)
	{
		try
		{
			base::String result{path_string};

			// 去掉 \\?\ 前缀
			base::String prefix = "\\\\?\\";

			if (result.StartWith(prefix))
			{
				result.Remove(base::Range{0, prefix.Length()});
			}

			return result;
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error{CODE_POS_STR + e.what()};
		}
		catch (...)
		{
			throw std::runtime_error{CODE_POS_STR + "未知的异常。"};
		}
	}

	///
	/// @brief 拷贝单个文件。
	///
	/// @param source_path 源路径。
	/// 	@warning 这里没有进行检查。必须确保源路径是一个文件。
	///
	/// @param destination_path 目标路径
	/// 	@note source_path 指向的对象复制后将是这个路径。也就是复制可以顺便重命名。
	///
	/// @param overwrite_method
	///
	void CopySingleFile(base::Path const &source_path,
						base::Path const &destination_path,
						base::filesystem::OverwriteOption overwrite_method)
	{
		if (destination_path.IsRootPath())
		{
			throw std::runtime_error{CODE_POS_STR + "无法将源路径移动为根路径。"};
		}

		std::filesystem::copy_options options = std::filesystem::copy_options::copy_symlinks;

		if (!base::filesystem::Exists(destination_path))
		{
			// 目标路径不存在，直接复制。
			base::filesystem::EnsureDirectory(destination_path.ParentPath());

			// 拷贝单个文件。
			// 没有进行检查，调用者必须确保源路径是一个文件。
			if (base::filesystem::IsSymbolicLink(source_path))
			{
				base::filesystem::CreateSymboliclink(destination_path,
													 base::filesystem::ReadSymboliclink(source_path));
			}
			else
			{
				std::filesystem::copy(ToWindowsLongPathString(source_path),
									  ToWindowsLongPathString(destination_path),
									  options);
			}

			return;
		}

		// 目标路径存在
		if (overwrite_method == base::filesystem::OverwriteOption::Skip)
		{
			return;
		}

		if (overwrite_method == base::filesystem::OverwriteOption::Overwrite)
		{
			// 无条件覆盖。
			base::filesystem::Remove(destination_path);

			if (base::filesystem::IsSymbolicLink(source_path))
			{
				base::filesystem::CreateSymboliclink(destination_path,
													 base::filesystem::ReadSymboliclink(source_path));
			}
			else
			{
				std::filesystem::copy(ToWindowsLongPathString(source_path),
									  ToWindowsLongPathString(destination_path),
									  options);
			}

			return;
		}

		// 如果更新则覆盖
		std::filesystem::directory_entry src_entry{ToWindowsLongPathString(source_path)};
		std::filesystem::directory_entry dst_entry{ToWindowsLongPathString(destination_path)};

		if (src_entry.last_write_time() <= dst_entry.last_write_time())
		{
			return;
		}

		// 需要更新
		base::filesystem::Remove(destination_path);

		if (base::filesystem::IsSymbolicLink(source_path))
		{
			base::filesystem::CreateSymboliclink(destination_path,
												 base::filesystem::ReadSymboliclink(source_path));
		}
		else
		{
			std::filesystem::copy(ToWindowsLongPathString(source_path),
								  ToWindowsLongPathString(destination_path),
								  options);
		}

		return;
	}

	/* #region 目录条目迭代器 */

	///
	/// @brief 目录条目迭代器。
	///
	///
	class DirectoryEntryEnumerator :
		public base::IEnumerator<base::filesystem::DirectoryEntry const>
	{
	private:
		base::filesystem::DirectoryEntry _current;
		std::filesystem::directory_iterator _current_it;
		std::filesystem::directory_iterator _end_it;
		base::IEnumerator<base::filesystem::DirectoryEntry const>::Context_t _context{};

	public:
		DirectoryEntryEnumerator(base::Path const &path)
		{
			std::string path_str = path.ToString();
			if (path_str == "")
			{
				path_str = "./";
			}

			_current_it = std::filesystem::directory_iterator{ToWindowsLongPathString(path_str)};
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
			_current = base::filesystem::DirectoryEntry{WindowsLongPathStringToPath(_current_it->path().string())};
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

			_current_it = std::filesystem::recursive_directory_iterator{ToWindowsLongPathString(path_str)};
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
			_current = base::filesystem::DirectoryEntry{WindowsLongPathStringToPath(_current_it->path().string())};
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

	/* #endregion */

} // namespace

/* #region 访问权限检查 */

bool base::filesystem::IsReadable(base::Path const &path)
{
	return access(path.ToString().c_str(), R_OK) == 0;
}

bool base::filesystem::IsWriteable(base::Path const &path)
{
	return access(path.ToString().c_str(), W_OK) == 0;
}

bool base::filesystem::IsExcuteable(base::Path const &path)
{
	return access(path.ToString().c_str(), X_OK) == 0;
}

/* #endregion */

/* #region 目标类型检查 */

bool base::filesystem::IsDirectory(base::Path const &path)
{
	std::error_code error_code{};
	bool ret = std::filesystem::is_directory(ToWindowsLongPathString(path), error_code);

	if (error_code.value() != 0)
	{
		std::string message = CODE_POS_STR;

		message += std::format("错误代码：{}，错误消息：{}",
							   error_code.value(),
							   error_code.message());

		throw std::runtime_error{message};
	}

	return ret;
}

bool base::filesystem::IsRegularFile(base::Path const &path)
{
	std::error_code error_code{};
	bool ret = std::filesystem::is_regular_file(ToWindowsLongPathString(path), error_code);

	if (error_code.value() != 0)
	{
		std::string message = CODE_POS_STR;

		message += std::format("错误代码：{}，错误消息：{}",
							   error_code.value(),
							   error_code.message());

		throw std::runtime_error{message};
	}

	return ret;
}

bool base::filesystem::IsSymbolicLink(base::Path const &path)
{
	HANDLE h = CreateFileA(ToWindowsLongPathString(path).c_str(),
						   0,
						   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						   nullptr,
						   OPEN_EXISTING,
						   FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
						   nullptr);

	HandleGuard g{h};

	if (h == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error{CODE_POS_STR + "调用 CreateFileA 失败，无法打开文件。"};
	}

	FILE_ATTRIBUTE_TAG_INFO info;

	WINBOOL call_result = GetFileInformationByHandleEx(h,
													   FileAttributeTagInfo,
													   &info,
													   sizeof(info));

	if (!call_result)
	{
		throw std::runtime_error{CODE_POS_STR + "调用 GetFileInformationByHandleEx 获取文件属性失败。"};
	}

	if (info.ReparseTag == IO_REPARSE_TAG_SYMLINK)
	{
		return true;
	}

	return false;

	{
		// 这是基于标准库的实现，在 msys2 中不可用。

		// std::error_code error_code{};
		// bool ret = std::filesystem::is_symlink(path.ToString(), error_code);

		// if (error_code.value() != 0)
		// {
		// 	std::string message = CODE_POS_STR;

		// 	message += std::format("错误代码：{}，错误消息：{}",
		// 						   error_code.value(),
		// 						   error_code.message());

		// 	throw std::runtime_error{message};
		// }

		// return ret;
	}
}

/* #endregion */

base::Path base::filesystem::CurrentPath()
{
	auto path = std::filesystem::current_path();
	base::Path ret{WindowsLongPathStringToPath(path.string())};
	return ret;
}

bool base::filesystem::Exists(base::Path const &path)
{
	std::error_code error_code{};
	bool ret = std::filesystem::exists(ToWindowsLongPathString(path), error_code);

	if (error_code.value() != 0)
	{
		std::string message = CODE_POS_STR;

		message += std::format("检查文件是否存在失败。错误代码：{}，错误消息：{}",
							   error_code.value(),
							   error_code.message());

		throw std::runtime_error{message};
	}

	return ret;
}

base::Path base::filesystem::ReadSymboliclink(base::Path const &symbolic_link_obj_path)
{
	HANDLE h = CreateFileA(ToWindowsLongPathString(symbolic_link_obj_path).c_str(),
						   0,
						   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						   nullptr,
						   OPEN_EXISTING,
						   FILE_FLAG_BACKUP_SEMANTICS,
						   nullptr);

	HandleGuard g{h};

	if (h == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error{CODE_POS_STR + "CreateFileA 调用失败，无法打开符号链接文件。"};
	}

	int64_t buffer_size = 1024 * 32;
	std::unique_ptr<uint8_t[]> buffer{new uint8_t[buffer_size]};

	DWORD len = GetFinalPathNameByHandleA(h,
										  reinterpret_cast<char *>(buffer.get()),
										  buffer_size,
										  FILE_NAME_NORMALIZED);

	if (len == 0 || len >= buffer_size)
	{
		throw std::runtime_error{CODE_POS_STR + "读取符号链接失败。"};
	}

	std::string result{
		reinterpret_cast<char *>(buffer.get()),
		static_cast<size_t>(len),
	};

	return WindowsLongPathStringToPath(result);

	{
		// if (!base::filesystem::IsSymbolicLink(symbolic_link_obj_path))
		// {
		// 	throw std::runtime_error{CODE_POS_STR + "传进来的路径必须是一个符号链接的路径。"};
		// }

		// std::error_code error_code{};
		// std::filesystem::path target_path = std::filesystem::read_symlink(symbolic_link_obj_path.ToString(), error_code);

		// if (error_code.value() != 0)
		// {
		// 	std::string message = CODE_POS_STR;

		// 	message += std::format("读取符号链接失败。错误代码：{}，错误消息：{}",
		// 						   error_code.value(),
		// 						   error_code.message());

		// 	throw std::runtime_error{message};
		// }

		// return target_path.string();
	}
}

void base::filesystem::CreateSymboliclink(base::Path const &symbolic_link_obj_path,
										  base::Path const &link_to_path)
{
	DWORD flags = 0;

	if (base::filesystem::IsDirectory(link_to_path))
	{
		flags = SYMBOLIC_LINK_FLAG_DIRECTORY;
	}

	flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

	bool call_result = CreateSymbolicLinkA(ToWindowsLongPathString(symbolic_link_obj_path).c_str(),
										   ToWindowsLongPathString(link_to_path).c_str(),
										   flags);

	if (!call_result)
	{
		throw std::runtime_error{CODE_POS_STR + "创建符号链接失败。"};
	}

	{
		// std::error_code error_code{};

		// std::filesystem::create_symlink(link_to_path.ToString(),
		// 								symbolic_link_obj_path.ToString(),
		// 								error_code);

		// if (error_code.value() != 0)
		// {
		// 	std::string message = CODE_POS_STR;

		// 	message += std::format("创建符号链接失败。错误代码：{}，错误消息：{}",
		// 						   error_code.value(),
		// 						   error_code.message());

		// 	throw std::runtime_error{message};
		// }
	}
}

/* #region 创建目录 */

void base::filesystem::CreateDirectory(base::Path const &path)
{
	if (base::filesystem::Exists(path))
	{
		std::string message = CODE_POS_STR;
		message += std::format("目标路径 {} 已存在。", path.ToString());
		throw std::runtime_error{message};
	}

	std::error_code error_code{};
	bool ret = std::filesystem::create_directory(path.ToString(), error_code);

	if (error_code.value() != 0)
	{
		std::string message = CODE_POS_STR;

		message += std::format("创建目录失败。错误代码：{}，错误消息：{}",
							   error_code.value(),
							   error_code.message());

		throw std::runtime_error{message};
	}

	if (!ret)
	{
		std::string message = CODE_POS_STR + "创建目录失败，但是没有错误代码。";
		throw std::runtime_error{message};
	}
}

void base::filesystem::CreateDirectoryRecursively(base::Path const &path)
{
	if (base::filesystem::Exists(path))
	{
		std::string message = CODE_POS_STR;
		message += std::format("目标路径 {} 已存在。", path.ToString());
		throw std::runtime_error{message};
	}

	std::error_code error_code{};
	bool ret = std::filesystem::create_directories(ToWindowsLongPathString(path), error_code);

	if (error_code.value() != 0)
	{
		std::string message = CODE_POS_STR;

		message += std::format("创建目录 {} 失败。错误代码：{}，错误消息：{}",
							   path.ToString(),
							   error_code.value(),
							   error_code.message());

		throw std::runtime_error{message};
	}

	if (!ret)
	{
		std::string message = CODE_POS_STR + "创建目录失败，但是没有错误代码。";
		throw std::runtime_error{message};
	}
}

/* #endregion */

/* #region Remove */

void base::filesystem::Remove(base::Path const &path)
{
	if (!Exists(path))
	{
		// 路径不存在，直接返回。
		return;
	}

	std::error_code error_code{};

	// 返回值是 uintmax_t ，含义是递归删除的项目总数。
	auto removed_count = std::filesystem::remove_all(path.ToString(), error_code);

	if (error_code.value() != 0)
	{
		std::string message = std::format("{} 删除失败。错误代码：{}，错误消息：{}",
										  CODE_POS_STR,
										  error_code.value(),
										  error_code.message());

		throw std::runtime_error{message};
	}

	if (removed_count == 0)
	{
		std::string message = CODE_POS_STR + "删除失败，因为删除了 0 个项目，但是没有错误代码。";
		throw std::runtime_error{message};
	}
}

/* #endregion */

/* #region Copy */

void base::filesystem::Copy(base::Path const &source_path,
							base::Path const &destination_path,
							base::filesystem::OverwriteOption overwrite_method)
{
	if (!base::filesystem::Exists(source_path))
	{
		std::string message = CODE_POS_STR;
		message += std::format("源路径 {} 不存在。", source_path.ToString());
		throw std::runtime_error{message};
	}

	if (destination_path.IsRootPath())
	{
		throw std::runtime_error{CODE_POS_STR + "无法将源路径移动为根路径。"};
	}

	// 执行到这里说明源路径存在
	if (base::filesystem::IsFile(source_path))
	{
		CopySingleFile(source_path, destination_path, overwrite_method);
		return;
	}

	// 执行到这里说明源路径是目录
	EnsureDirectory(destination_path);

	// 开始递归复制
	for (auto entry : std::filesystem::recursive_directory_iterator{ToWindowsLongPathString(source_path)})
	{
		base::Path relative_path{WindowsLongPathStringToPath(entry.path().string())};
		relative_path.RemoveBasePath(source_path);

		base::Path src_path = source_path + relative_path;
		base::Path dst_path = destination_path + relative_path;

		if (IsFile(src_path))
		{
			// 源路径是一个文件
			CopySingleFile(src_path, dst_path, overwrite_method);
		}
		else
		{
			// 源路径是一个目录
			EnsureDirectory(dst_path);
		}
	}
}

/* #endregion */

/* #region Move */

void base::filesystem::Move(base::Path const &source_path,
							base::Path const &destination_path,
							base::filesystem::OverwriteOption overwrite_method)
{
	if (!base::filesystem::Exists(source_path))
	{
		std::string message = CODE_POS_STR;
		message += std::format("源路径 {} 不存在。", source_path.ToString());
		throw std::runtime_error{message};
	}

	if (destination_path.IsRootPath())
	{
		throw std::runtime_error{CODE_POS_STR + "无法将源路径移动为根路径。"};
	}

	if (!base::filesystem::Exists(destination_path))
	{
		// 目标路径不存在，直接移动

		// 先确保父目录存在，否则会抛出异常
		base::filesystem::EnsureDirectory(destination_path.ParentPath());
		std::error_code error_code{};

		std::filesystem::rename(source_path.ToString(),
								destination_path.ToString(),
								error_code);

		if (error_code.value() != 0)
		{
			std::string message = CODE_POS_STR;

			message += std::format("移动失败。错误代码：{}，错误消息：{}",
								   error_code.value(),
								   error_code.message());

			throw std::runtime_error{message};
		}

		return;
	}

	// 目标路径存在
	if (overwrite_method == base::filesystem::OverwriteOption::Skip)
	{
		return;
	}

	if (overwrite_method == base::filesystem::OverwriteOption::Overwrite)
	{
		// 直接覆盖目标文件
		base::filesystem::Remove(destination_path);
		std::error_code error_code{};

		std::filesystem::rename(source_path.ToString(),
								destination_path.ToString(),
								error_code);

		if (error_code.value() != 0)
		{
			std::string message = CODE_POS_STR;

			message += std::format("移动失败。错误代码：{}，错误消息：{}",
								   error_code.value(),
								   error_code.message());

			throw std::runtime_error{message};
		}

		return;
	}

	// 如果更新则覆盖
	std::filesystem::directory_entry src_entry{source_path.ToString()};
	std::filesystem::directory_entry dst_entry{destination_path.ToString()};

	if (src_entry.last_write_time() <= dst_entry.last_write_time())
	{
		return;
	}

	// 需要更新
	base::filesystem::Remove(destination_path);
	std::error_code error_code{};

	std::filesystem::rename(source_path.ToString(),
							destination_path.ToString(),
							error_code);

	if (error_code.value() != 0)
	{
		std::string message = CODE_POS_STR;

		message += std::format("移动失败。错误代码：{}，错误消息：{}",
							   error_code.value(),
							   error_code.message());

		throw std::runtime_error{message};
	}

	return;
}

/* #endregion */

/* #region 迭代目录条目 */

std::shared_ptr<base::IEnumerator<base::filesystem::DirectoryEntry const>> base::filesystem::CreateDirectoryEntryEnumerator(base::Path const &path)
{
	return std::shared_ptr<DirectoryEntryEnumerator>{new DirectoryEntryEnumerator{path}};
}

std::shared_ptr<base::IEnumerator<base::filesystem::DirectoryEntry const>> base::filesystem::CreateDirectoryEntryRecursiveEnumerator(base::Path const &path)
{
	return std::shared_ptr<RecursiveDirectoryEntryEnumerator>{new RecursiveDirectoryEntryEnumerator{path}};
}

/* #endregion */
