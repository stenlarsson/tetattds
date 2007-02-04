#pragma once

class IpAddress
{
public:
	IpAddress()
	:	address(0), port(0)
	{
	}

	IpAddress(unsigned int address, int port)
	:	address(address), port(port)
	{
	}
	
	void Print()
	{
		printf("%u.%u.%u.%u:%i",
			(address >>  0) & 0xFF,
			(address >>  8) & 0xFF,
			(address >> 16) & 0xFF,
			(address >> 24) & 0xFF,
			port);
	}


	bool operator==(const IpAddress& right)
	{
		return address == right.address && port == right.port;
	}

	unsigned int address;
	int port;
};
