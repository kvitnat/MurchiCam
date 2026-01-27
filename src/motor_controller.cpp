#include "motor_controller.h"
#include <cmath>
#include <algorithm>
#include "Arduino.h"

// in: x, y
// out: A speed, A dir
//		B speed, B dir

int MAX_SPEED = 250;

void motor_controller::input(float x, float y)
{
	prev_x = x;
	prev_y = y;
	int speed = sqrt(x*x + y*y) * MAX_SPEED;

	// t is joystick angle / (pi/2)
	float t = 1;
	if (x != 0)
		t = atan(abs(float(y) / float(x))) / (M_PI / 2);

	if (x!= 0)
	{
		Serial.print("y/x = ");
		Serial.print(y);
		Serial.print("/");
		Serial.print(x);
		Serial.print(" = ");
		Serial.println(float(y)/float(x));
		Serial.print("arctan = ");
		Serial.println(atan(abs(float(y)/float(x))));
	}



	bool dir = y > 0;

	int& HighLeft = dir ? LIn1_speed : LIn2_speed;
	int& LowLeft = !dir ? LIn1_speed : LIn2_speed;

	int& HighRight = dir ? RIn1_speed : RIn2_speed;
	int& LowRight = !dir ? RIn1_speed : RIn2_speed;

	LowLeft = 0;
	LowRight = 0;

	HighLeft = speed;
	HighRight = speed;

	if (x >= 0)
	{
		HighRight *= t;
	}
	else
	{
		HighLeft *= t;
	}
	
}

void motor_controller::input2(float x, float y)
{
	prev_x = x;
	prev_y = y;

	float leftSpeed  = y + x;
	float rightSpeed = y - x;

	bool leftFw = (leftSpeed > 0);
	bool rightFw = (rightSpeed > 0);

	int y_sign = (y > 0) ? 1 : -1;
	int x_sign = (x > 0) ? 1 : -1;
	x_sign *= y_sign; 

	leftSpeed = sqrt(abs(y_sign * y * y + x_sign * x * x)) * MAX_SPEED;
	rightSpeed = sqrt(abs(y_sign * y * y - x_sign * x * x)) * MAX_SPEED;

	int& HighLeft = leftFw ? LIn1_speed : LIn2_speed;
	int& LowLeft = !leftFw ? LIn1_speed : LIn2_speed;

	int& HighRight = rightFw ? RIn1_speed : RIn2_speed;
	int& LowRight = !rightFw ? RIn1_speed : RIn2_speed;

	LowLeft = 0;
	LowRight = 0;

	HighLeft = abs(leftSpeed);
	HighRight = abs(rightSpeed);

}

bool motor_controller::is_input_changed(float x, float y)
{
	return (x != prev_x) || (y != prev_y);
}
