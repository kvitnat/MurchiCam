
enum class motion_state 
{
	Forward, 
	Backward, 
	Still
};

struct car_speed
{
	float left  = 0.f;
	float right = 0.f;

	car_speed() {}
	car_speed(float l, float r) : left(l), right(r) {}
};

/*
	motion_controller is responsible for taking (x,y) values that came from joystick
	and calculating speed of each wheel in (-1; 1) range
*/
class motion_controller
{
private:
	float prev_x = -1.f;
	float prev_y = -1.f;

public:
	car_speed input(float x, float y);
	bool is_input_changed(float x, float y);
};

class driver_controller
{
public:
	void set_speed(car_speed in_speed);

	void calculate_driver_values();
	bool needs_starting_voltage();

	void init_driver_pins();
	void write_values_to_driver();
	
	const char* get_debug_info();

private:
	
	const unsigned int pwm_frequency = 15000;
	const unsigned char pwm_resolution = 8;
	
	
	float min_speed_value = 155;
	float max_speed_value = 255;
	float threshold_speed_value = 190;
	float speed_range = max_speed_value - min_speed_value;

	// Speed values (from -1.f to 1.f) passed from motion_controller
	car_speed speed_;
		
	// pwm values for driver
	int L_in1_speed = 0;
	int L_in2_speed = 0;

	int R_in1_speed = 0;
	int R_in2_speed = 0;

	// save previous motion state to check if we need starting voltage next time
	motion_state prev_state_left;
	motion_state prev_state_right;

	bool left_needs_starting_voltage = false;
	bool right_needs_starting_voltage = false;

	motion_state get_state_from_speed(float speed);

};