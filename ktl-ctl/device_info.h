#pragma once

struct DeviceInfo
{
	DeviceInfo(const std::string& classGuid);
	~DeviceInfo();


private:
	HDEVINFO handle_;
};
