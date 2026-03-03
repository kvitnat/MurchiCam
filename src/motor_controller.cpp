#include "motor_controller.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <string>
#include "Arduino.h"

car_speed motion_controller::input(float x, float y)
{
	prev_x = x;
	prev_y = y;

	bool left_dir  = y + x > 0;
	bool right_dir = y - x > 0;

	int left_sign  = left_dir  ? 1 : -1;
	int right_sign = right_dir ? 1 : -1;

	int y_sign = (y > 0) ? 1 : -1;
	int x_sign = (x > 0) ? 1 : -1;
	x_sign *= y_sign; 

	float left_speed = sqrt(abs(y_sign * y * y + x_sign * x * x)) * left_sign;
	float right_speed = sqrt(abs(y_sign * y * y - x_sign * x * x)) * right_sign;

	return { left_speed, right_speed };
}

bool motion_controller::is_input_changed(float x, float y)
{
	return (x != prev_x) || (y != prev_y);
}

const char* driver_controller::get_debug_info()
{
	std::ostringstream oss;
	oss << "L_in1 " << L_in1_speed << ", L_in2 " << L_in2_speed << ", R_in1 " << R_in1_speed << ", R_in2 " << R_in2_speed << "\n";
	return oss.str().c_str();
}

void driver_controller::set_speed(car_speed in_speed)
{
	speed_ = in_speed;
}

void driver_controller::calculate_driver_values()
{
	int& high_left = speed_.left > 0 ? L_in1_speed : L_in2_speed;
	int& low_left = !(speed_.left > 0) ? L_in1_speed : L_in2_speed;

	int& high_right = speed_.right > 0 ? R_in1_speed : R_in2_speed;
	int& low_right = !(speed_.right > 0) ? R_in1_speed : R_in2_speed;

	low_left = 0;
	low_right = 0;
	high_left = int(min_speed_value + speed_range * abs(speed_.left));
	high_right = int(min_speed_value + speed_range * abs(speed_.right));

	if (abs(speed_.left) < 0.1)
		high_left = 0;
	
	if (abs(speed_.right) < 0.1)
		high_right = 0;


	if (left_needs_starting_voltage)
	{
		high_left = threshold_speed_value;
		left_needs_starting_voltage = false;
	}

	if (right_needs_starting_voltage)
	{
		high_right = threshold_speed_value;
		right_needs_starting_voltage = false;
	}

	prev_state_left  = get_state_from_speed(speed_.left);
	prev_state_right = get_state_from_speed(speed_.right);

}

motion_state driver_controller::get_state_from_speed(float speed)
{
	if (speed > 0) 
		return motion_state::Forward;
	else if (speed < 0)
		return motion_state::Backward;
	else
		return motion_state::Still;
}

bool driver_controller::needs_starting_voltage()
{	
	motion_state state_left  = get_state_from_speed(speed_.left);
	motion_state state_right = get_state_from_speed(speed_.right);
	
	left_needs_starting_voltage = prev_state_left != state_left && 
								  state_left != motion_state::Still && 
								  abs(speed_.left) < threshold_speed_value;
	right_needs_starting_voltage = prev_state_right != state_right && 
								   state_right != motion_state::Still && 
								   abs(speed_.right) < threshold_speed_value;

	return left_needs_starting_voltage || right_needs_starting_voltage;
}


void driver_controller::write_values_to_driver()
{
    ledcWrite(A0, R_in1_speed);
    ledcWrite(A1, R_in2_speed);
    ledcWrite(A2, L_in2_speed);
    ledcWrite(A3, L_in1_speed);
}

void driver_controller::init_driver_pins()
{
	ledcSetup(A0, pwm_frequency, pwm_resolution);
    ledcSetup(A1, pwm_frequency, pwm_resolution);
    ledcSetup(A2, pwm_frequency, pwm_resolution);
    ledcSetup(A3, pwm_frequency, pwm_resolution);

    ledcAttachPin(A0, A0);
    ledcAttachPin(A1, A1);
    ledcAttachPin(A2, A2);
    ledcAttachPin(A3, A3);
}
