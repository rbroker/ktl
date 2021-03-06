#pragma once

struct ServiceHandle
{
	ServiceHandle(SC_HANDLE handle) :
		handle_(handle)
	{
	}

	~ServiceHandle()
	{
		if (!handle_)
			return;

		CloseServiceHandle(handle_);
	}

	explicit operator bool()
	{
		return handle_ != nullptr;
	}

	SC_HANDLE operator&()
	{
		return handle_;
	}

private:
	SC_HANDLE handle_;
};
