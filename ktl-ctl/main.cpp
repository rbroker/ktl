#define NOMINMAX
#include <Windows.h>
#include <newdev.h>
#include <SetupAPI.h>

#include <iostream>
#include <filesystem>
#include <mutex>
#include <new>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include "handle.h"
#include "service.h"
#include <ktl_shared.h>

std::wstring DriverName = L"ktl_test.sys";
std::wstring ServiceName = L"KTL Test Driver";

void print_help()
{
	std::wcout << L"Expected a command:" << std::endl;
	std::wcout << L"ktl-ctl.exe install [inf_path]" << std::endl;
	std::wcout << L"ktl-ctl.exe uninstall [inf_path]" << std::endl;
	std::wcout << L"ktl-ctl.exe start" << std::endl;
	std::wcout << L"ktl-ctl.exe stop" << std::endl;
	std::wcout << L"ktl-ctl.exe test" << std::endl;
	std::wcout << L"ktl-ctl.exe soak" << std::endl;
}

void DriverInstall(const std::wstring& inf_path)
{
	BOOL needsReboot = FALSE;

	std::filesystem::path servicePath = std::filesystem::absolute(std::filesystem::path(inf_path)).parent_path();
	servicePath /= DriverName;

	if (!std::filesystem::exists(servicePath))
		throw std::exception((std::string("Unable to locate driver binary: ") + servicePath.string()).c_str());

	if (!DiInstallDriverW(nullptr, inf_path.c_str(), 0, &needsReboot))
		throw std::system_error(std::error_code(::GetLastError(), std::system_category()), "Failed to install driver");

	ServiceHandle serviceManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	ServiceHandle service = CreateServiceW(&serviceManager,
		ServiceName.c_str(),
		L"KTL Test Driver Service",
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		servicePath.wstring().c_str(),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr);

	if (!service)
	{
		auto err = ::GetLastError();
		if (err != ERROR_SERVICE_EXISTS)
		{
			throw std::system_error(std::error_code(err, std::system_category()), "Failed to create driver service");
		}
	}

	std::wcout << L"Successfully installed driver." << (needsReboot ? L" Please reboot the machine." : L" Reboot is not required.") << std::endl;
	std::wcout << L"Service Path: " << servicePath.wstring() << std::endl;
}

void DriverUninstall(const std::wstring& inf_path)
{
	BOOL needsReboot = FALSE;

	ServiceHandle serviceManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	ServiceHandle service = OpenServiceW(&serviceManager, ServiceName.c_str(), SERVICE_ALL_ACCESS);

	if (service)
	{
		if (!DeleteService(&service))
		{
			auto err = ::GetLastError();
			if (err != ERROR_SERVICE_MARKED_FOR_DELETE)
				throw std::system_error(std::error_code(err, std::system_category()), "Failed to schedule driver service for deletion");
		}
	}

	if (!DiUninstallDriverW(nullptr, inf_path.c_str(), 0, &needsReboot))
		throw std::system_error(std::error_code(::GetLastError(), std::system_category()), "Failed to uninstall driver");

	std::wcout << L"Successfully uninstalled driver." << (needsReboot ? L" Please reboot the machine." : L" Reboot is not required.") << std::endl;
}

void DriverStart()
{
	ServiceHandle serviceManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	ServiceHandle service = OpenServiceW(&serviceManager, ServiceName.c_str(), SERVICE_ALL_ACCESS);

	if (!service)
		throw std::system_error(std::error_code(::GetLastError(), std::system_category()), "Failed to open driver service");

	if (!StartServiceW(&service, 0, nullptr))
	{
		auto err = ::GetLastError();
		if (err == ERROR_SERVICE_ALREADY_RUNNING)
			return;

		throw std::system_error(std::error_code(err, std::system_category()), "Failed to start driver service");
	}
}

void DriverStop()
{
	ServiceHandle serviceManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	ServiceHandle service = OpenServiceW(&serviceManager, ServiceName.c_str(), SERVICE_ALL_ACCESS);

	if (!service)
		throw std::system_error(std::error_code(::GetLastError(), std::system_category()), "Failed to open driver service");

	SERVICE_STATUS status;
	if (!ControlService(&service, SERVICE_CONTROL_STOP, &status))
		throw std::system_error(std::error_code(::GetLastError(), std::system_category()), "Failed to stop driver service");
}

void RunTest(DWORD ioctl, std::vector<std::system_error>* errors, std::mutex* mtx, const std::string& name)
{
	try
	{
		Handle h = CreateFileW(L"\\\\.\\" KTL_TEST_DEVICE_USERMODE_NAME,
			GENERIC_READ,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);

		if (!h)
			throw std::system_error(std::error_code(::GetLastError(), std::system_category()), "Failed to open handle for: " + name + " test");

		if (!DeviceIoControl(h.get(), ioctl, nullptr, 0, nullptr, 0, nullptr, nullptr))
			throw std::system_error(std::error_code(::GetLastError(), std::system_category()), "Failed " + name + " test");
	}
	catch (std::system_error& err)
	{
		std::scoped_lock lock(*mtx);
		errors->emplace_back(std::move(err));
	}
}

void DriverTest(const std::wstring_view mode = L"all")
{
	std::mutex mtx;
	std::vector<std::system_error> errors;

	{
		if (mode == L"all" || mode == L"list")
			std::jthread listTestThr(RunTest, IOCTL_KTLTEST_METHOD_LIST_TEST, &errors, &mtx, "<list>");

		if (mode == L"all" || mode == L"memory")
			std::jthread memoryTestThr(RunTest, IOCTL_KTLTEST_METHOD_MEMORY_TEST, &errors, &mtx, "<memory>");

		if (mode == L"all" || mode == L"set")
			std::jthread setTestThr(RunTest, IOCTL_KTLTEST_METHOD_SET_TEST, &errors, &mtx, "<set>");

		if (mode == L"all" || mode == L"vector")
			std::jthread vectorTestThr(RunTest, IOCTL_KTLTEST_METHOD_VECTOR_TEST, &errors, &mtx, "<vector>");

		if (mode == L"all" || mode == L"unicode_string")
			std::jthread stringTestThr(RunTest, IOCTL_KTLTEST_METHOD_STRING_TEST, &errors, &mtx, "<unicode_string>");

		if (mode == L"all" || mode == L"unicode_string_view")
			std::jthread stringViewTestThr(RunTest, IOCTL_KTLTEST_METHOD_STRING_VIEW_TEST, &errors, &mtx, "<unicode_string_view>");

		if (mode == L"all" || mode == L"tuple")
			std::jthread tupleTestThr(RunTest, IOCTL_KTLTEST_METHOD_TUPLE_TEST, &errors, &mtx, "<tuple>");

		if (mode == L"all" || mode == L"optional")
			std::jthread tupleTestThr(RunTest, IOCTL_KTLTEST_METHOD_OPTIONAL_TEST, &errors, &mtx, "<optional>");

		if (mode == L"all" || mode == L"map")
			std::jthread tupleTestThr(RunTest, IOCTL_KTLTEST_METHOD_MAP_TEST, &errors, &mtx, "<map>");
	}

	for (const auto& err : errors)
		throw err;
}

int wmain(int argc, wchar_t** argv)
{
	try
	{
		std::wstring_view command;
		std::wstring_view mode = L"all";
		std::wstring inf_path = L"ktl_test.inf";

		for (int i = 0; i < argc; ++i)
		{
			std::wstring_view current = argv[i];

			if (i == 1)
			{
				command = current;
			}
			else if (i == 2)
			{
				if (command == L"test")
					mode = current;

				inf_path = current;
			}
		}

		if (command.empty())
		{
			print_help();
			return -1;
		}

		if (command == L"install")
		{
			DriverInstall(inf_path);
		}
		else if (command == L"uninstall")
		{
			DriverUninstall(inf_path);
		}
		else if (command == L"start")
		{
			DriverStart();
		}
		else if (command == L"stop")
		{
			DriverStop();
		}
		else if (command == L"test")
		{
			DriverStart();
			DriverTest(mode);
			DriverStop();
		}
		else if (command == L"soak")
		{
			for (int j = 0; j < 5; ++j)
			{
				DriverStart();
				for (int i = 0; i < 500; ++i)
				{
					DriverTest();
				}
				DriverStop();
			}
		}
	}
	catch (std::system_error& ex)
	{
		std::cout << "[ERROR]: " << ex.what() << "(" << ex.code().value() << ")" << std::endl;
		try { DriverStop(); } catch(...) {}
		return -1;
	}

	return 0;
}