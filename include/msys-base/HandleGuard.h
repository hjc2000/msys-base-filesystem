#pragma once
#include <windows.h>

namespace msys
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

} // namespace msys
