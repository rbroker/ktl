#pragma once

struct Handle
{
	Handle(HANDLE handle) :
		handle_(handle)
	{
	}

	explicit operator bool()
	{
		return handle_ != INVALID_HANDLE_VALUE;
	}

	HANDLE get()
	{
		return handle_;
	}

	~Handle()
	{
		if (handle_ == INVALID_HANDLE_VALUE)
			return;

		CloseHandle(handle_);
	}

private:
	HANDLE handle_ = INVALID_HANDLE_VALUE;
};
