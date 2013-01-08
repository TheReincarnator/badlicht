// *************************
// ** Remote control keys **
// *************************

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
#define KEY_ON 17
#define KEY_BRIGHT_PLUS 18
#define KEY_BRIGHT_MINUS 19
#define KEY_SPEED_PLUS 20
#define KEY_SPEED_MINUS 21
#define KEY_FADE 22
#define KEY_PROGRAM 23


// *************************************
// ** Color, brightness, speed tables **
// *************************************

int keyHues[16] = {0, 13, 24, 43, 53, 85, 111, 128, 139, 146, 170, 156, 173, 198, 222, 0};
//int keySaturations[16] = {255, 252, 237, 200, 200, 255, 232, 255, 255, 255, 255, 191, 176, 166, 120, 0};
int keySaturations[16] = {255, 255, 255, 255, 255, 255, 245, 255, 255, 255, 255, 245, 230, 230, 220, 0};

int brightnesses[16] = {11, 23, 35, 48, 61, 76, 91, 106, 123, 139, 157, 175, 194, 214, 234, 255};
long colorTimes[10] = {20000, 12000, 6000, 3000, 1500, 1000, 600, 350, 200, 100};

// **********************
// ** Global variables **
// **********************

int on = 1;
int colors = 0;
int hues[256];
int saturations[256];
int brightnessIndex = 8;
int speedIndex = 4;
int program = 0;
int fade = 1;
long lastFadeTime = 0;
double colorPosition = 0.0;


// **********************
// ** Helper functions **
// **********************

void handleKey(int key)
{
	if (key == KEY_BRIGHT_PLUS && on && brightnessIndex < 15)
	{
		brightnessIndex++;
		setRgb(0, 0, 0);
		delay(100);
	}
	else if (key == KEY_BRIGHT_MINUS && on && brightnessIndex > 0)
	{
		brightnessIndex--;
		setRgb(0, 0, 0);
		delay(100);
	}

	if (program)
	{
		if (key == KEY_PROGRAM || key == KEY_FADE)
		{
			if (colors <= 0)
			{
				if (key == KEY_FADE)
					programRainbow();
				else
					programGray();
			}

			program = 0;
			lastFadeTime = millis();
			colorPosition = 0.0;
			speedIndex = 4;
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
		if (key == KEY_ON && !on)
		{
			on = 1;
			lastFadeTime = millis();
			colorPosition = 0.0;
		}
		else if (key == KEY_OFF && on)
			on = 0;

		if (!on)
			return;

		if (key >= KEY_COLOR0 && key <= KEY_COLOR15)
		{
			colors = 1;
			hues[0] = keyHues[key];
			saturations[0] = keySaturations[key];
		}
		else if (key == KEY_SPEED_PLUS && speedIndex < 9)
		{
			speedIndex++;
			setRgb(0, 0, 0);
			delay(100);
		}
		else if (key == KEY_SPEED_MINUS && speedIndex > 0)
		{
			speedIndex--;
			setRgb(0, 0, 0);
			delay(100);
		}
		else if (key == KEY_FADE)
		{
			if (colors == 1)
			{
				programRainbow();
				lastFadeTime = millis();
				colorPosition = 0.0;
				speedIndex = 3;
				fade = 1;
			}
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

int mixHues(int hue1, int hue2, double blend)
{
	if (hue1 - hue2 > 128)
		hue2 += 256;

	if (hue2 - hue1 > 128)
		hue1 += 256;

	return ((int) (hue1 * (1.0 - blend) + hue2 * blend + 0.5)) % 256;
}

int mixSaturations(int saturation1, int saturation2, double blend)
{
	return (int) (saturation1 * (1.0 - blend) + saturation2 * blend + 0.5);
}

int mixBrightnesses(int brightness1, int brightness2, double blend)
{
	return (int) (brightness1 * (1.0 - blend) + brightness2 * blend + 0.5);
}

void programGray()
{
	colors = 1;
	for (int i=0; i<256; i++)
	{
		hues[i] = 0;
		saturations[i] = 0;
	}
}

void programRainbow()
{
	colors = 15;
	for (int i=0; i<15; i++)
	{
		hues[i] = i*17;
		saturations[i] = 255;
	}
}

void readInput()
{
	if (digitalRead(2) == 1)
		return;

	long start = micros();
	long current = start;
	long last = current;
	long diff = 0;

	char rawCode[26];
	int i;
	for (i=0; i<25 && current - start < 100000L; i++)
	{
		while (digitalRead(2) == 0 && current - start < 100000L)
			current = micros();
		if (current - start >= 100000L)
			break;

		last = current;

		while (digitalRead(2) == 1 && current - start < 100000L)
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

	if (strlen(code) > 23)
		code[23] = 0;

	int key;
	if (strcmp(code, "........#########..##..") == 0)
		key = KEY_COLOR0;
	else if (strcmp(code, "........###########.#..") == 0)
		key = KEY_COLOR1;
	else if (strcmp(code, "........########......#") == 0)
		key = KEY_COLOR2;
	else if (strcmp(code, "........########.#.#...") == 0)
		key = KEY_COLOR3;
	else if (strcmp(code, "........########..###..") == 0)
		key = KEY_COLOR4;
	else if (strcmp(code, "........##########.##..") == 0)
		key = KEY_COLOR5;
	else if (strcmp(code, "........########.#..#..") == 0)
		key = KEY_COLOR6;
	else if (strcmp(code, "........########..##..#") == 0)
		key = KEY_COLOR7;
	else if (strcmp(code, "........########.####..") == 0)
		key = KEY_COLOR8;
	else if (strcmp(code, "........########..#.#..") == 0)
		key = KEY_COLOR9;
	else if (strcmp(code, "........#########...#..") == 0)
		key = KEY_COLOR10;
	else if (strcmp(code, "........########.##.#..") == 0)
		key = KEY_COLOR11;
	else if (strcmp(code, "........########..#....") == 0)
		key = KEY_COLOR12;
	else if (strcmp(code, "........########.###...") == 0)
		key = KEY_COLOR13;
	else if (strcmp(code, "........############...") == 0)
		key = KEY_COLOR14;
	else if (strcmp(code, "........#########.#.#..") == 0)
		key = KEY_COLOR15;
	else if (strcmp(code, "........#############..") == 0)
		key = KEY_OFF;
	else if (strcmp(code, "........#########.##...") == 0)
		key = KEY_ON;
	else if (strcmp(code, "........#########..#...") == 0)
		key = KEY_BRIGHT_PLUS;
	else if (strcmp(code, "........#########.###..") == 0)
		key = KEY_BRIGHT_MINUS;
	else if (strcmp(code, "........#########.##..#") == 0)
		key = KEY_SPEED_PLUS;
	else if (strcmp(code, "........########.......") == 0)
		key = KEY_SPEED_MINUS;
	else if (strcmp(code, "........########.#.##..") == 0)
		key = KEY_FADE;
	else if (strcmp(code, "........########..##...") == 0)
		key = KEY_PROGRAM;
	else
		return;

	handleKey(key);
}


void setHsb(int hue, int sat, int bri)
{
	hue %= 256;
	if (sat < 0) sat = 0;
	if (sat > 255) sat = 255;
	if (bri < 0) bri = 0;
	if (bri > 255) bri = 255;

	double r, g, b;
	if (hue < 85)
	{
		r = (85 - hue) / 85.0;
		g = hue / 85.0;
		b = 0;
	}
	else if (hue < 170)
	{
		r = 0;
		g = (170 - hue) / 85.0;
		b = (hue - 85) / 85.0;
	}
	else
	{
		r = (hue - 170) / 85.0;
		g = 0;
		b = (255 - hue) / 85.0;
	}

	double amp = 255.0 * (sat / 255.0);
	r *= amp; g *= amp; b *= amp;
	r += (255 - sat); g += (255 - sat); b += (255 - sat);
	amp = (bri / 255.0);
	r *= amp; g *= amp; b *= amp;

	setRgb(r, g, b);
}

void setRgb(int r, int g, int b)
{
	analogWrite(3, r);
	analogWrite(5, g);
	analogWrite(6, b);
	analogWrite(9, r);
	analogWrite(10, g);
	analogWrite(11, b);
}

void updateOutput()
{
	if (!on)
		setRgb(0, 0, 0);
	else if (program)
	{
		int blinkDivider = millis() % 800 < 400 ? 8 : 4;
		int brightness = brightnesses[brightnessIndex] / blinkDivider;
		setRgb(brightness, brightness, brightness);
	}
	else if (colors > 0)
	{
		long currentTime = millis();
		long elapsedTime = currentTime - lastFadeTime;
		lastFadeTime = currentTime;
		if (elapsedTime > 0)
			colorPosition += elapsedTime * 1.0 / colorTimes[speedIndex];

		int currentColor = (int) colorPosition;
		if (currentColor >= colors)
		{
			colorPosition -= colors;
			currentColor = 0;
		}

		int hue;
		int saturation;
		int brightness;
		if (fade)
		{
			int nextColor = (currentColor + 1) % colors;
			double blend = colorPosition - currentColor;

			int hue1 = hues[currentColor];
			int hue2 = hues[nextColor];
			int saturation1 = saturations[currentColor];
			int saturation2 = saturations[nextColor];
			int brightness1 = hue1 == 255 && saturation1 == 0 ? 0 : brightnesses[brightnessIndex];
			int brightness2 = hue2 == 255 && saturation2 == 0 ? 0 : brightnesses[brightnessIndex];

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


// **************************
// ** Initializer and loop **
// **************************

void setup()
{
	pinMode(2, INPUT);
	pinMode(3, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);

	programGray();
	lastFadeTime = millis();
	colorPosition = 0.0;

	setRgb(32, 32, 32);
	delay(200);
	setRgb(0, 0, 0);
	delay(400);
}

void loop()
{
	readInput();
	updateOutput();
}
