// ***********************
// ***  CONFIGURATION  ***
// ***********************

// Input pin: Infrared receiver
#define INPUT_IR 8

// Output pins: First set of RGB LEDs
#define OUTPUT_RED1 3
#define OUTPUT_GREEN1 5
#define OUTPUT_BLUE1 6

// Output pins: Optional second set of RGB LEDs
//#define OUTPUT_RED2 9
//#define OUTPUT_GREEN2 10
//#define OUTPUT_BLUE2 11

// Output pins: Optional main lights
#define OUTPUT_MAINLIGHTS 2

// Output pins: Does low voltage at RGB pins mean "lights on" (if undefined, high voltage is "on")?
#define RGB_LOW_ACTIVE

// Output pins: Does low voltage at main light pin mean "lights on" (if undefined, high voltage is "on")?
#define MAIN_LOW_ACTIVE

// Remote control buttons: Count, hues, and saturations
#define KEY_COUNT 16
const unsigned char keyHues[KEY_COUNT] = {0, 7, 15, 22, 29, 85, 75, 111, 128, 142, 170, 156, 190, 202, 230, 0};
const unsigned char keySaturations[KEY_COUNT] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0};

// Colors: Default fade sequence: Hues, brightness, speed
#define DEFAULT_FADE_COLOR_COUNT 15
const unsigned char defaultFadeHues[DEFAULT_FADE_COLOR_COUNT] = {0, 7, 16, 24, 33, 65, 82, 103, 108, 128, 160, 190, 205, 215, 235};

// Brightnesses
#define BRIGHTNESS_COUNT 17
const unsigned char brightnesses[BRIGHTNESS_COUNT] = {0, 11, 23, 35, 48, 61, 76, 91, 106, 123, 139, 157, 175, 194, 214, 234, 255};
#define DEFAULT_BRIGHTNESS_INDEX 12

// Speeds
#define SPEED_COUNT 11
const long colorTimes[SPEED_COUNT] = {35000, 20000, 12000, 6000, 3000, 1500, 800, 400, 200, 100, 50};
#define DEFAULT_SPEED_INDEX 4

// Whether to optimize the intensity steps of the RGB LEDs
// Without optimization, each color has 256 steps of intensity (using PWM), which may cause visible steps when fading
// The optimization quickly switches between the steps, increasing the number of steps to 2560.
// However, if timing is bad, this may even introduce flickering.
// Try both, on and off, and use whatever is better for your LEDs.
#define OPTIMIZE_RGB_STEPS

// Color calibration 1: Sometimes, the RGB LED color mix is tinted.
// Using the below parameters, you can adjust the overall level of red, green, and blue.
// Find parameters where all brightness levels of white actually look white.
// Use values from 0.0 to 1.0 only. Keep a parameter undefined if you don't need it.
//#define CALIBRATION_RED_FACTOR 0.8
//#define CALIBRATION_GREEN_FACTOR 0.8
//#define CALIBRATION_BLUE_FACTOR 0.8

// Color calibration 2: If the tint differs from brightness to brightness, you have a more complex calibration to do.
// Using the below parameters, you can control, how much one color affects the intensity of another color.
// Again, find parameters (including those of calibration 1), where all brightness levels of white actually look white.
// Use values from 0.0 to 1.0 only. Keep a parameter undefined if you don't need it.
#define CALIBRATION_GREEN_DAMP_BY_RED 0.5
#define CALIBRATION_BLUE_DAMP_BY_RED 0.8
//#define CALIBRATION_RED_DAMP_BY_GREEN 0.3
//#define CALIBRATION_BLUE_DAMP_BY_GREEN 0.3
//#define CALIBRATION_RED_DAMP_BY_BLUE 0.3
//#define CALIBRATION_GREEN_DAMP_BY_BLUE 0.3

// Test mode 1 (cycles all outputs regardless of IR)
//#define TEST_MODE1

// Test mode 2 (RED, GREEN, BLUE keys and those below to set RGB manually (+16, +1, -1, -16))
//#define TEST_MODE2

// Test mode 3 (RED, GREEN, BLUE keys and those below to set HSB manually (+16, +1, -1, -16))
//#define TEST_MODE3

// Whether to log the RGB output to the Serial port
//#define LOG_RGB


// *******************
// ***  CONSTANTS  ***
// *******************

#define KEY_COLOR0 0
#define KEY_COLOR1 1
#define KEY_COLOR2 2
#define KEY_COLOR3 3
#define KEY_COLOR4 4
#define KEY_COLOR5 5
#define KEY_COLOR6 6
#define KEY_COLOR7 7
#define KEY_COLOR8 8
#define KEY_COLOR9 9
#define KEY_COLOR10 10
#define KEY_COLOR11 11
#define KEY_COLOR12 12
#define KEY_COLOR13 13
#define KEY_COLOR14 14
#define KEY_COLOR15 15
#define KEY_OFF 16
#define KEY_MAINLIGHTS 17
#define KEY_BRIGHT_PLUS 18
#define KEY_BRIGHT_MINUS 19
#define KEY_SPEED_PLUS 20
#define KEY_SPEED_MINUS 21
#define KEY_FADE 22
#define KEY_PROGRAM 23


// **************************
// ***  GLOBAL VARIABLES  ***
// **************************

bool mainLights;
double colorPosition;
short int colors;
unsigned char hues[256];
unsigned char saturations[256];
short int brightnessIndex;
short int speedIndex;
bool program;
bool fade;
long lastFadeTime;

#ifdef TEST_MODE1
	short int testStep;
#endif

#if defined(TEST_MODE2) || defined(TEST_MODE3)
	unsigned char testVal1;
	unsigned char testVal2;
	unsigned char testVal3;
#endif


// **************************
// ***  HELPER FUNCTIONS  ***
// **************************

unsigned char convertRgbDoubleToInteger(double doubleValue, unsigned char fractionCycle)
{
	if (doubleValue <= 0.0)
		return 0;
	else if (doubleValue >= 0.999)
		return 255;

	doubleValue = doubleValue * (255.0 * 10.0);
	int intValue = (int) doubleValue;
	unsigned char intPart = intValue / 10;

	#ifdef OPTIMIZE_RGB_STEPS
		if (fade)
		{
			unsigned char fraction = intValue - intPart * 10;
			if (fraction > fractionCycle)
				intPart++;
		}
	#endif
	
	return intPart;
}

void handleKey(unsigned char key)
{
	#if defined(TEST_MODE2) || defined(TEST_MODE3)
		if (key == KEY_COLOR0)
			testVal1 = testVal1 <= (255-16) ? testVal1+16 : 255;
		else if (key == KEY_COLOR1 && testVal1 < 255)
			testVal1++;
		else if (key == KEY_COLOR2 && testVal1 > 0)
			testVal1--;
		else if (key == KEY_COLOR3)
			testVal1 = testVal1 >= 16 ? testVal1-16 : 0;
		else if (key == KEY_COLOR5)
			testVal2 = testVal2 <= (255-16) ? testVal2+16 : 255;
		else if (key == KEY_COLOR6 && testVal2 < 255)
			testVal2++;
		else if (key == KEY_COLOR7 && testVal2 > 0)
			testVal2--;
		else if (key == KEY_COLOR8)
			testVal2 = testVal2 >= 16 ? testVal2-16 : 0;
		else if (key == KEY_COLOR10)
			testVal3 = testVal3 <= (255-16) ? testVal3+16 : 255;
		else if (key == KEY_COLOR11 && testVal3 < 255)
			testVal3++;
		else if (key == KEY_COLOR12 && testVal3 > 0)
			testVal3--;
		else if (key == KEY_COLOR13)
			testVal3 = testVal3 >= 16 ? testVal3-16 : 0;

		#ifdef TEST_MODE2
			setRgb(testVal1 / 255.0, testVal2 / 255.0, testVal3 / 255.0);
		#else
			setHsb(testVal1, testVal2, testVal3);
		#endif

		if (true) return;
	#endif
	
	if (key == KEY_BRIGHT_PLUS && !mainLights && brightnessIndex < BRIGHTNESS_COUNT-1)
		brightnessIndex++;
	else if (key == KEY_BRIGHT_MINUS && !mainLights && brightnessIndex > 1)
		brightnessIndex--;

	if (program)
	{
		if (key == KEY_PROGRAM || key == KEY_FADE)
		{
			if (colors <= 0)
			{
				if (key == KEY_FADE)
					programDefaultFade();
				else
					programWhite();
			}

			program = 0;
			lastFadeTime = millis();
			colorPosition = 0.0;
			speedIndex = DEFAULT_SPEED_INDEX;
			fade = key == KEY_FADE ? 1 : 0;
		}
		else if (key >= KEY_COLOR0 && key <= KEY_COLOR15 || key == KEY_OFF)
		{
			if (colors < 256)
			{
				setRgb(0, 0, 0);
				delay(50);

				if (key == KEY_OFF)
				{
					hues[colors] = 255;
					saturations[colors] = 0;
				}
				else
				{
					hues[colors] = keyHues[key];
					saturations[colors] = keySaturations[key];
					setHsb(hues[colors], saturations[colors], brightnesses[brightnessIndex]);
				}

				delay(300);
				setRgb(0, 0, 0);
				delay(50);

				colors++;
			}
		}
	}
	else
	{
		if ((key >= KEY_COLOR0 && key <= KEY_COLOR15 || key == KEY_OFF
			|| key == KEY_FADE || key == KEY_PROGRAM) && mainLights)
		{
			mainLights = 0;
			programWhite();
		}
		#ifdef OUTPUT_MAINLIGHTS
			else if (key == KEY_MAINLIGHTS && !mainLights)
				mainLights = 1;
		#endif

		if (mainLights)
			return;

		if (key >= KEY_COLOR0 && key <= KEY_COLOR15)
		{
			colors = 1;
			hues[0] = keyHues[key];
			saturations[0] = keySaturations[key];
			if (brightnessIndex <= 0)
				brightnessIndex = DEFAULT_BRIGHTNESS_INDEX;
		}
		else if (key == KEY_OFF)
		{
			programWhite();
			brightnessIndex = 0;
		}
		else if (key == KEY_SPEED_PLUS && speedIndex < SPEED_COUNT-1)
		{
			speedIndex++;
			setRgb(0, 0, 0);
			delay(70);
		}
		else if (key == KEY_SPEED_MINUS && speedIndex > 0)
		{
			speedIndex--;
			setRgb(0, 0, 0);
			delay(70);
		}
		else if (key == KEY_FADE)
		{
			if (colors == 1 || brightnessIndex <= 0)
				programDefaultFade();
			else
				fade = fade ? 0 : 1;
		}
		else if (key == KEY_PROGRAM)
		{
			program = 1;
			colors = 0;
		}
	}
}

double mixHues(int hue1, int hue2, double blend)
{
	if (hue1 - hue2 > 128.0)
		hue2 += 256.0;

	if (hue2 - hue1 > 128.0)
		hue1 += 256.0;

	double result = hue1 * (1.0 - blend) + hue2 * blend;
	if (result >= 256.0)
		result -= 256.0;

	return result;
}

double mixSaturations(unsigned char saturation1, unsigned char saturation2, double blend)
{
	double result = saturation1 * (1.0 - blend) + saturation2 * blend + 0.5;
	if (result < 0.0)
		return 0.0;
	else if (result >= 255.0)
		return 255.0;
	else
		return result;
}

double mixBrightnesses(unsigned char brightness1, unsigned char brightness2, double blend)
{
	double result = brightness1 * (1.0 - blend) + brightness2 * blend + 0.5;
	if (result < 0.0)
		return 0.0;
	else if (result >= 255.0)
		return 255.0;
	else
		return result;
}

void programWhite()
{
	program = 0;

	colors = 1;
	for (short int i=0; i<256; i++)
	{
		hues[i] = 0;
		saturations[i] = 0;
	}

	brightnessIndex = DEFAULT_BRIGHTNESS_INDEX;
	speedIndex = DEFAULT_SPEED_INDEX;
	fade = 0;

	lastFadeTime = millis();
	colorPosition = 0.0;
}

void programDefaultFade()
{
	program = 0;

	colors = DEFAULT_FADE_COLOR_COUNT;
	for (short int i=0; i<DEFAULT_FADE_COLOR_COUNT; i++)
	{
		hues[i] = defaultFadeHues[i];
		saturations[i] = 255;
	}

	brightnessIndex = DEFAULT_BRIGHTNESS_INDEX;
	speedIndex = DEFAULT_SPEED_INDEX;
	fade = 1;

	lastFadeTime = millis();
	colorPosition = 0.0;
}

void readInput()
{
	if (digitalRead(INPUT_IR) == 1)
		return;

	long start = micros();
	long current = start;
	long last = current;
	long diff = 0;

	char rawCode[26];
	short int i;
	for (i=0; i<25 && current - start < 100000L; i++)
	{
		while (digitalRead(INPUT_IR) == 0 && current - start < 100000L)
			current = micros();
		if (current - start >= 100000L)
			break;

		last = current;

		while (digitalRead(INPUT_IR) == 1 && current - start < 100000L)
			current = micros();
		if (current - start >= 100000L)
			break;

		diff = current - last;
		last = current;
		rawCode[i] = diff > 1000 ? '#' : '.';
	}
	rawCode[i] = 0;

	char * code = strstr(rawCode, "........########");
	if (!code)
		return;
	code = &code[16];

	if (strlen(code) > 7)
		code[7] = 0;

	unsigned char key;
	if (strcmp(code, "#..##..") == 0) key = KEY_COLOR0;
	else if (strcmp(code, "###.#..") == 0) key = KEY_COLOR1;
	else if (strcmp(code, "......#") == 0) key = KEY_COLOR2;
	else if (strcmp(code, ".#.#...") == 0) key = KEY_COLOR3;
	else if (strcmp(code, "..###..") == 0) key = KEY_COLOR4;
	else if (strcmp(code, "##.##..") == 0) key = KEY_COLOR5;
	else if (strcmp(code, ".#..#..") == 0) key = KEY_COLOR6;
	else if (strcmp(code, "..##..#") == 0) key = KEY_COLOR7;
	else if (strcmp(code, ".####..") == 0) key = KEY_COLOR8;
	else if (strcmp(code, "..#.#..") == 0) key = KEY_COLOR9;
	else if (strcmp(code, "#...#..") == 0) key = KEY_COLOR10;
	else if (strcmp(code, ".##.#..") == 0) key = KEY_COLOR11;
	else if (strcmp(code, "..#....") == 0) key = KEY_COLOR12;
	else if (strcmp(code, ".###...") == 0) key = KEY_COLOR13;
	else if (strcmp(code, "####...") == 0) key = KEY_COLOR14;
	else if (strcmp(code, "#.#.#..") == 0) key = KEY_COLOR15;
	else if (strcmp(code, "#####..") == 0) key = KEY_OFF;
	else if (strcmp(code, "#.##...") == 0) key = KEY_MAINLIGHTS;
	else if (strcmp(code, "#..#...") == 0) key = KEY_BRIGHT_PLUS;
	else if (strcmp(code, "#.###..") == 0) key = KEY_BRIGHT_MINUS;
	else if (strcmp(code, "#.##..#") == 0) key = KEY_SPEED_PLUS;
	else if (strcmp(code, ".......") == 0) key = KEY_SPEED_MINUS;
	else if (strcmp(code, ".#.##..") == 0) key = KEY_FADE;
	else if (strcmp(code, "..##...") == 0) key = KEY_PROGRAM;
	else return;

	handleKey(key);
}

void setHsb(double hue, double sat, double bri)
{
	double r, g, b;
	if (hue < 85.0)
	{
		r = (85.0 - hue) / 85.0;
		g = hue / 85.0;
		b = 0;
	}
	else if (hue < 170.0)
	{
		r = 0;
		g = (170.0 - hue) / 85.0;
		b = (hue - 85.0) / 85.0;
	}
	else
	{
		r = (hue - 170.0) / 85.0;
		g = 0;
		b = (255.0 - hue) / 85.0;
	}

	double satFactor = sat / 255.0;
	satFactor *= satFactor;
	double negSatFactor = 1 - satFactor;
	r *= satFactor; g *= satFactor; b *= satFactor;
	r += negSatFactor; g += negSatFactor; b += negSatFactor;

	// Color calibrations
	double rc = r, gc = g, bc = b;
	#ifdef CALIBRATION_RED_FACTOR
		rc *= CALIBRATION_RED_FACTOR;
	#endif
	#ifdef CALIBRATION_GREEN_FACTOR
		gc *= CALIBRATION_GREEN_FACTOR;
	#endif
	#ifdef CALIBRATION_BLUE_FACTOR
		bc *= CALIBRATION_BLUE_FACTOR;
	#endif
	#ifdef CALIBRATION_GREEN_DAMP_BY_RED
		gc *= 1 - r * CALIBRATION_GREEN_DAMP_BY_RED;
	#endif
	#ifdef CALIBRATION_BLUE_DAMP_BY_RED
		bc *= 1 - r * CALIBRATION_BLUE_DAMP_BY_RED;
	#endif
	#ifdef CALIBRATION_RED_DAMP_BY_GREEN
		rc *= 1 - g * CALIBRATION_RED_DAMP_BY_GREEN;
	#endif
	#ifdef CALIBRATION_BLUE_DAMP_BY_GREEN
		bc *= 1 - g * CALIBRATION_BLUE_DAMP_BY_GREEN;
	#endif
	#ifdef CALIBRATION_RED_DAMP_BY_BLUE
		rc *= 1 - b * CALIBRATION_RED_DAMP_BY_BLUE;
	#endif
	#ifdef CALIBRATION_GREEN_DAMP_BY_BLUE
		gc *= 1 - b * CALIBRATION_GREEN_DAMP_BY_BLUE;
	#endif

	double briFactor = bri / 255.0;
	rc *= briFactor; gc *= briFactor; bc *= briFactor;

	setRgb(rc, gc, bc);
}

void setRgb(double rDouble, double gDouble, double bDouble)
{
	unsigned char fractionCycle = millis() % 5 * 2;
	unsigned char r = convertRgbDoubleToInteger(rDouble, fractionCycle);
	unsigned char g = convertRgbDoubleToInteger(gDouble, fractionCycle);
	unsigned char b = convertRgbDoubleToInteger(bDouble, fractionCycle);
	
	#ifdef LOG_RGB
		char message[256];
		sprintf(message, "R %03d  G %03d  B %03d\n", r, g, b);
		Serial.print(message);
	#endif

	#ifdef RGB_LOW_ACTIVE
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
	#endif

	// First set of lights

	#ifdef OUTPUT_RED1
		analogWrite(OUTPUT_RED1, r);
	#endif

	#ifdef OUTPUT_GREEN1
		analogWrite(OUTPUT_GREEN1, g);
	#endif

	#ifdef OUTPUT_BLUE1
		analogWrite(OUTPUT_BLUE1, b);
	#endif

	// Second set of lights

	#ifdef OUTPUT_RED2
		analogWrite(OUTPUT_RED2, r);
	#endif

	#ifdef OUTPUT_GREEN2
		analogWrite(OUTPUT_GREEN2, g);
	#endif

	#ifdef OUTPUT_BLUE2
		analogWrite(OUTPUT_BLUE2, b);
	#endif
}

#ifdef TEST_MODE1
	void testCycle()
	{
		digitalWrite(13, 1);
		delay(200);
		digitalWrite(13, 0);
		delay(800);
		
		switch (testStep)
		{
			case 0: handleKey(KEY_COLOR0); break;
			case 1: handleKey(KEY_COLOR5); break;
			case 2: handleKey(KEY_COLOR10); break;
			case 3: handleKey(KEY_OFF); break;
			case 4: handleKey(KEY_MAINLIGHTS); break;
			case 5: handleKey(KEY_MAINLIGHTS); break;
			case 6: handleKey(KEY_OFF); break;
		}
		
		testStep = (testStep+1) % 7;
	}
#endif

void updateOutput()
{
	#ifdef OUTPUT_MAINLIGHTS
		#ifdef MAIN_LOW_ACTIVE
			digitalWrite(OUTPUT_MAINLIGHTS, !mainLights);
		#else
			digitalWrite(OUTPUT_MAINLIGHTS, mainLights);
		#endif
	#endif

	if (mainLights)
		setRgb(0, 0, 0);
	else if (program)
	{
		double blinkValue = millis() % 800 < 400 ? 0.2 : 0.3;
		setRgb(blinkValue, blinkValue, blinkValue);
	}
	else if (colors > 0)
	{
		long currentTime = millis();
		long elapsedTime = currentTime - lastFadeTime;
		lastFadeTime = currentTime;
		if (elapsedTime > 0)
			colorPosition += elapsedTime * 1.0 / colorTimes[speedIndex];

		short int currentColor = (short int) colorPosition;
		if (currentColor >= colors)
		{
			colorPosition -= colors;
			currentColor = 0;
		}

		double hue;
		double saturation;
		double brightness;
		if (fade)
		{
			short int nextColor = (currentColor + 1) % colors;
			double blend = colorPosition - currentColor;

			unsigned char hue1 = hues[currentColor];
			unsigned char hue2 = hues[nextColor];
			unsigned char saturation1 = saturations[currentColor];
			unsigned char saturation2 = saturations[nextColor];
			unsigned char brightness1 = hue1 == 255 && saturation1 == 0 ? 0 : brightnesses[brightnessIndex];
			unsigned char brightness2 = hue2 == 255 && saturation2 == 0 ? 0 : brightnesses[brightnessIndex];

			if (brightness1 == 0 && brightness2 > 0)
			{
				hue1 = hue2;
				saturation1 = saturation2;
			}
			else if (brightness1 > 0 && brightness2 == 0)
			{
				hue2 = hue1;
				saturation2 = saturation1;
			}

			hue = mixHues(hue1, hue2, blend);
			saturation = mixSaturations(saturation1, saturation2, blend);
			brightness = mixBrightnesses(brightness1, brightness2, blend);
		}
		else
		{
			hue = hues[currentColor];
			saturation = saturations[currentColor];
			brightness = hue == 255 && saturation == 0 ? 0 : brightnesses[brightnessIndex];
		}

		setHsb(hue, saturation, brightness);
	}
}


// ******************************
// ***  INITIALIZER AND LOOP  ***
// ******************************

void setup()
{
	pinMode(INPUT_IR, INPUT);
	pinMode(13, OUTPUT);

	// First set of lights

	#ifdef OUTPUT_RED1
		pinMode(OUTPUT_RED1, OUTPUT);
	#endif

	#ifdef OUTPUT_GREEN1
		pinMode(OUTPUT_GREEN1, OUTPUT);
	#endif

	#ifdef OUTPUT_BLUE1
		pinMode(OUTPUT_BLUE1, OUTPUT);
	#endif

	// Second set of lights

	#ifdef OUTPUT_RED2
		pinMode(OUTPUT_RED2, OUTPUT);
	#endif

	#ifdef OUTPUT_GREEN2
		pinMode(OUTPUT_GREEN2, OUTPUT);
	#endif

	#ifdef OUTPUT_BLUE2
		pinMode(OUTPUT_BLUE2, OUTPUT);
	#endif

	// Change the PWM output counters to work the same
	// This ensures that all RGB outputs are in sync (not out of phase)
	TCCR2A |= 3;

	#ifdef OUTPUT_MAINLIGHTS
		pinMode(OUTPUT_MAINLIGHTS, OUTPUT);
	#endif

	programWhite();
	#ifdef OUTPUT_MAINLIGHTS
		mainLights = 1;
	#else
		mainLights = 0;
	#endif
	
	#ifdef TEST_MODE1
		testStep = 0;
	#endif

	#if defined(TEST_MODE2) || defined(TEST_MODE3)
		mainLights = 0;
		updateOutput();

		testVal1 = 0;
		testVal2 = 0;
		testVal3 = 0;

		#ifdef TEST_MODE2
			setRgb(testVal1, testVal2, testVal3);
		#else
			setHsb(testVal1, testVal2, testVal3);
		#endif
	#endif

	#ifdef LOG_RGB
		Serial.begin(9600);
	#endif
}

void loop()
{
	#if !defined(TEST_MODE2) && !defined(TEST_MODE3)
		updateOutput();
	#endif

	#ifdef TEST_MODE1
		testCycle();
	#else
		readInput();
	#endif
}

