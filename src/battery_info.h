#include <string>

class battery_info
{
	float bat_level = 3.3f;
	int count = 1;

public:
	std::string get_battery_string(unsigned short pin_reading);
};