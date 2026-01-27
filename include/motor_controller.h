class motor_controller
{
public:
	int LIn1_speed = 0;
	int LIn2_speed = 0;

	int RIn1_speed = 0;
	int RIn2_speed = 0;

	float prev_x = -1;
	float prev_y = -1;

public:
	void input(float x, float y);
	void input2(float x, float y);

	bool is_input_changed(float x, float y);

};