#include "battery_info.h"
#include <cmath>
#include <iomanip>
#include <sstream>

std::string battery_info::get_battery_string(unsigned short pin_reading)
{
	float new_value = pin_reading * 3.3f / 4095 * 2;
    new_value = std::round(new_value * 100.0f) / 100.0f;
    bat_level += (new_value - bat_level) / count++;
    int percentage = (bat_level - 3) * 100;
    if (count == 1000) count = 1;

    std::stringstream stream;
    stream << percentage << "% ";
    stream << std::fixed << std::setprecision(2) << bat_level << " V";
    return stream.str();
}
