#include "motor_controller.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <string>
#include "Arduino.h"

void motion_controller::input(float x, float y)
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

	left_speed = sqrt(abs(y_sign * y * y + x_sign * x * x)) * MAX_SPEED * left_sign;
	right_speed = sqrt(abs(y_sign * y * y - x_sign * x * x)) * MAX_SPEED * right_sign;

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

void driver_controller::update_wheels_speed(float in_left_speed, float in_right_speed)
{
	left_speed = in_left_speed;
	right_speed = in_right_speed;
}

void driver_controller::calculate_driver_values()
{
	int& high_left = left_speed > 0 ? L_in1_speed : L_in2_speed;
	int& low_left = !(left_speed > 0) ? L_in1_speed : L_in2_speed;

	int& high_right = right_speed > 0 ? R_in1_speed : R_in2_speed;
	int& low_right = !(right_speed > 0) ? R_in1_speed : R_in2_speed;

	low_left = 0;
	low_right = 0;

	high_left = abs(left_speed);
	high_right = abs(right_speed);

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

	prev_state_left  = get_state_from_speed(left_speed);
	prev_state_right = get_state_from_speed(right_speed);

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
	motion_state state_left  = get_state_from_speed(left_speed);
	motion_state state_right = get_state_from_speed(right_speed);
	
	left_needs_starting_voltage = prev_state_left != state_left && 
								  state_left != motion_state::Still && 
								  abs(left_speed) < threshold_speed_value;
	right_needs_starting_voltage = prev_state_right != state_right && 
								   state_right != motion_state::Still && 
								   abs(right_speed) < threshold_speed_value;

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

/*
void motion_controller::input_smooth_turn(float x, float y)
{
	prev_x = x;
	prev_y = y;
	int speed = sqrt(x*x + y*y) * MAX_SPEED;

	// t is joystick angle / (pi/2)
	float t = 1;
	if (x != 0)
		t = atan(abs(float(y) / float(x))) / (M_PI / 2);


	bool dir = y > 0;

	int& high_left = dir ? LIn1_speed : LIn2_speed;
	int& low_left = !dir ? LIn1_speed : LIn2_speed;

	int& high_right = dir ? RIn1_speed : RIn2_speed;
	int& low_right = !dir ? RIn1_speed : RIn2_speed;

	low_left = 0;
	low_right = 0;

	high_left = speed;
	high_right = speed;

	if (x >= 0)
	{
		high_right *= t;
	}
	else
	{
		high_left *= t;
	}
	
}
*/
