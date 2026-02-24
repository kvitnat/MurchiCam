
enum class motion_state 
{
	Forward, 
	Backward, 
	Still
};

class motion_controller
{
private:

	float left_speed  = 0.f;
	float right_speed = 0.f;

	float prev_x = -1.f;
	float prev_y = -1.f;

public:
	void input(float x, float y);
	
	float get_left() { return left_speed; }
	float get_right() { return right_speed; }
	
	bool is_input_changed(float x, float y);
	
	// not used
	// void input_smooth_turn(float x, float y);
};

class driver_controller
{
private:
	
	// Calculated values for our specific battery and motors

	const unsigned int pwm_frequency = 15000;
	const unsigned char pwm_resolution = 8;
	
	float battery_high = 3.7f; // V
	float battery_low  = 3.3f; // V
	float battery_level = 0.94f; // percent

	float battery_voltage = battery_low + (battery_high - battery_low) * battery_level;

	float min_voltage = 2.f; // V
	float threshold_voltage = 3.f; // V

	float MAX_VALUE = 255;

	float threshold_speed_value = threshold_voltage / battery_voltage * MAX_VALUE; // 3V 206 - 231
	float min_speed_value = min_voltage / battery_voltage * MAX_VALUE; // 2V 140 - 155 
	float max_speed_value = MAX_VALUE;
	float speed_range = max_speed_value - min_speed_value;


	// Speed values (from -1.f to 1.f) passed from motion_controller
	float left_speed;
	float right_speed;

	
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

public:
	void update_wheels_speed(float in_left_speed, float in_right_speed);

	void calculate_driver_values();
	bool needs_starting_voltage();

	void init_driver_pins();
	void write_values_to_driver();
	
	const char* get_debug_info();
	const char* get_debug_voltage_info();

};