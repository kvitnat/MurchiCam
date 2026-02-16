
enum class motion_state 
{
	Forward, 
	Backward, 
	Still
};

class motion_controller
{
private:

	int MAX_SPEED = 255;

	float left_speed  = 0.f;
	float right_speed = 0.f;

	float prev_x = -1.f;
	float prev_y = -1.f;

public:
	void input(float x, float y);
	// not used
	// void input_smooth_turn(float x, float y);

	float get_left() { return left_speed; }
	float get_right() { return right_speed; }

	bool is_input_changed(float x, float y);

};

class driver_controller
{
private:
	const unsigned int pwm_frequency = 15000;
	const unsigned char pwm_resolution = 8;

	float threshold_speed_value = 200;

	float left_speed;
	float right_speed;

	int L_in1_speed = 0;
	int L_in2_speed = 0;

	int R_in1_speed = 0;
	int R_in2_speed = 0;

	motion_state prev_state_left;
	motion_state prev_state_right;

	bool left_needs_starting_voltage = false;
	bool right_needs_starting_voltage = false;

	motion_state get_state_from_speed(float speed);

public:
	void update_wheels_speed(float in_left_speed, float in_right_speed);

	void calculate_driver_values();
	void write_values_to_driver();
	
	bool needs_starting_voltage();
	
	void init_driver_pins();

	const char* get_debug_info();

};