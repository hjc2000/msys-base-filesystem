#include "base/filesystem/filesystem.h"
#include "base/container/iterator/IEnumerator.h"
#include "base/container/Range.h"
#include "base/filesystem/Path.h"
#include "base/string/define.h"
#include "base/string/String.h"
#include "msys-base-filesystem/HandleGuard.h"
#include "msys-base-filesystem/REPARSE_DATA_BUFFER.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
#include <windows.h>

#undef CreateDirectory

namespace
{
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
		try
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
														 base::filesystem::ReadSymboliclink(source_path),
														 base::filesystem::IsSymbolicLinkDirectory(source_path.ToString()));
				}
				else
				{
					std::filesystem::copy(base::filesystem::ToWindowsLongPathString(source_path),
										  base::filesystem::ToWindowsLongPathString(destination_path),
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
														 base::filesystem::ReadSymboliclink(source_path),
														 base::filesystem::IsSymbolicLinkDirectory(source_path.ToString()));
				}
				else
				{
					std::filesystem::copy(base::filesystem::ToWindowsLongPathString(source_path),
										  base::filesystem::ToWindowsLongPathString(destination_path),
										  options);
				}

				return;
			}

			// 如果更新则覆盖
			std::filesystem::directory_entry src_entry{base::filesystem::ToWindowsLongPathString(source_path)};
			std::filesystem::directory_entry dst_entry{base::filesystem::ToWindowsLongPathString(destination_path)};

			if (src_entry.last_write_time() <= dst_entry.last_write_time())
			{
				return;
			}

			// 需要更新
			base::filesystem::Remove(destination_path);

			if (base::filesystem::IsSymbolicLink(source_path))
			{
				base::filesystem::CreateSymboliclink(destination_path,
													 base::filesystem::ReadSymboliclink(source_path),
													 base::filesystem::IsSymbolicLinkDirectory(source_path.ToString()));
			}
			else
			{
				std::filesystem::copy(base::filesystem::ToWindowsLongPathString(source_path),
									  base::filesystem::ToWindowsLongPathString(destination_path),
									  options);
			}

			return;
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

			_current_it = std::filesystem::directory_iterator{base::filesystem::ToWindowsLongPathString(path_str)};
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
	try
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
	catch (std::exception const &e)
	{
		throw std::runtime_error{CODE_POS_STR + e.what()};
	}
	catch (...)
	{
		throw std::runtime_error{CODE_POS_STR + "未知的异常。"};
	}
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

	msys::HandleGuard g{h};

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
}

bool base::filesystem::IsSymbolicLinkDirectory(base::Path const &path)
{
	if (!IsSymbolicLink(path))
	{
		return false;
	}

	DWORD attrs = GetFileAttributesA(path.ToString().c_str());

	if (attrs == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	// 是符号链接并且是目录
	bool ret = (attrs & FILE_ATTRIBUTE_REPARSE_POINT) &&
			   (attrs & FILE_ATTRIBUTE_DIRECTORY);

	return ret;
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
						   FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
						   nullptr);

	msys::HandleGuard g{h};

	if (h == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error{CODE_POS_STR + "CreateFileA 调用失败，无法打开符号链接文件。"};
	}

	int64_t buffer_size = 1024 * 32;
	std::unique_ptr<uint8_t[]> buffer{new uint8_t[buffer_size]};

	// === 核心修改开始 ===
	// 使用 DeviceIoControl 获取重分析点数据，而不是解析最终路径
	DWORD returned_len = 0;

	BOOL ioResult = DeviceIoControl(h,
									FSCTL_GET_REPARSE_POINT, // 关键指令：获取原始数据
									nullptr, 0,
									buffer.get(), static_cast<DWORD>(buffer_size),
									&returned_len,
									nullptr);

	if (!ioResult || returned_len == 0)
	{
		throw std::runtime_error{CODE_POS_STR + "DeviceIoControl 调用失败，无法读取重分析点数据。"};
	}

	// 解析缓冲区获取原始路径文本
	REPARSE_DATA_BUFFER *rdb = reinterpret_cast<REPARSE_DATA_BUFFER *>(buffer.get());

	// 检查是否为符号链接标签
	if (rdb->ReparseTag != IO_REPARSE_TAG_SYMLINK)
	{
		throw std::runtime_error{CODE_POS_STR + "该文件不是符号链接。"};
	}

	// 提取 SubstituteName (内部存储的原始路径)
	USHORT nameOffset = rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset;
	USHORT nameLength = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength;

	// 注意：Windows 内部使用 UTF-16，这里简单处理为字节偏移
	WCHAR *rawPathPtr = reinterpret_cast<WCHAR *>(reinterpret_cast<BYTE *>(&rdb->SymbolicLinkReparseBuffer.PathBuffer) +
												  nameOffset);

	int wcharCount = nameLength / sizeof(WCHAR);

	// 将 UTF-16 转换为 UTF-8 存入你的 buffer (覆盖之前的原始数据)
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, rawPathPtr, wcharCount,
									  reinterpret_cast<char *>(buffer.get()),
									  static_cast<int>(buffer_size),
									  nullptr, nullptr);

	if (utf8Len == 0 || utf8Len >= buffer_size)
	{
		throw std::runtime_error{CODE_POS_STR + "路径编码转换失败。"};
	}
	// === 核心修改结束 ===

	// 原有代码保持不变
	std::string result{
		reinterpret_cast<char *>(buffer.get()),
		static_cast<size_t>(utf8Len), // 使用转换后的长度
	};

	std::cout << CODE_POS_STR << result << std::endl;

	return WindowsLongPathStringToPath(result);
}

void base::filesystem::CreateSymboliclink(base::Path const &symbolic_link_obj_path,
										  base::Path const &link_to_path,
										  bool is_directory)
{
	DWORD flags = 0;

	if (is_directory)
	{
		flags = SYMBOLIC_LINK_FLAG_DIRECTORY;
	}

	flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

	base::String link_to_path_string = link_to_path.ToString();
	link_to_path_string.Replace("/", "\\");

	bool call_result = CreateSymbolicLinkA(ToWindowsLongPathString(symbolic_link_obj_path).c_str(),
										   link_to_path_string.StdString().c_str(),
										   flags);

	if (!call_result)
	{
		throw std::runtime_error{CODE_POS_STR + "创建符号链接失败。"};
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
	bool ret = std::filesystem::create_directory(ToWindowsLongPathString(path), error_code);

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
	auto removed_count = std::filesystem::remove_all(ToWindowsLongPathString(path),
													 error_code);

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
	try
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
		if (base::filesystem::IsSymbolicLink(source_path) ||
			!base::filesystem::IsDirectory(source_path))
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

			if (IsDirectory(src_path))
			{
				// 源路径是一个目录
				EnsureDirectory(dst_path);
			}
			else
			{
				// 源路径是一个文件
				CopySingleFile(src_path, dst_path, overwrite_method);
			}
		}
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

		std::filesystem::rename(ToWindowsLongPathString(source_path),
								ToWindowsLongPathString(destination_path),
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

		std::filesystem::rename(ToWindowsLongPathString(source_path),
								ToWindowsLongPathString(destination_path),
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
	std::filesystem::directory_entry src_entry{ToWindowsLongPathString(source_path)};
	std::filesystem::directory_entry dst_entry{ToWindowsLongPathString(destination_path)};

	if (src_entry.last_write_time() <= dst_entry.last_write_time())
	{
		return;
	}

	// 需要更新
	base::filesystem::Remove(destination_path);
	std::error_code error_code{};

	std::filesystem::rename(ToWindowsLongPathString(source_path),
							ToWindowsLongPathString(destination_path),
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
