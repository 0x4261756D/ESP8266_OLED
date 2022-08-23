#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <heltec.h>

struct Vec3
{
	float x, y, z;
};

struct Vec2
{
	float x, y;
};
int cube_line_indices[12][2] = {{0, 1}, {0, 2}, {0, 4}, {1, 3}, {1, 5}, {2, 3}, {2, 6}, {3, 7}, {4, 5}, {4, 6}, {5, 7}, {6, 7}};
int pyramid_line_indices[8][2] = {{0, 1}, {0, 2}, {3, 1}, {3, 2}, {0, 4}, {1, 4}, {2, 4}, {3, 4}};

Vec3 cube[8] = {{1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1}, {-1, 1, 1}, {-1, 1, -1}, {-1, -1, 1}, {-1, -1, -1}};

Vec3 pyramid[5] = {{0, 0, 0}, {1, 0, 0}, {0, 0, 1}, {1, 0, 1}, {.5, 1, .5}};

Vec3 happy[80] =
{
//      0            1            2            3            4            5
	{14, 9, 0}, {14, 1, 0}, {14, 5, 0}, {10, 5, 0}, {10, 9, 0}, {10, 1, 0}, //H
//     6           7           8           9          10
	{8, 1, 0}, {6, 9, 0}, {4, 1, 0}, {7, 5, 0}, {5, 5, 0}, //A
//    11          12         13         14         15         16          17
	{2, 1, 0}, {2, 9, 0}, {-0, 9, 0}, {-2, 8, 0}, {-2, 6, 0}, {-0, 5, 0}, {2, 5, 0}, //P
//   18         19         20         21         22         23         24
	{-4, 1, 0}, {-4, 9, 0}, {-6, 9, 0}, {-8, 8, 0}, {-8, 6, 0}, {-6, 5, 0}, {-4, 5, 0}, //P
//    25          26          27          28
	{-12, 5, 0}, {-10, 9, 0}, {-14, 9, 0}, {-12, 1, 0}, //Y
//      29            30            31            32            33            34            35            36            37            38
	{23, -1, 0}, {23, -9, 0}, {21, -9, 0}, {19, -8, 0}, {19, -6, 0}, {21, -5, 0}, {19, -4, 0}, {19, -2, 0}, {21, -1, 0}, {23, -5, 0}, //B
//      39            40            41            42            43            44
	{16, -1, 0}, {14, -1, 0}, {15, -1, 0}, {15, -9, 0}, {16, -9, 0}, {14, -9, 0}, //I
//      45            46           47           48           49           50           51            52
	{11, -9, 0}, {11, -1, 0}, {9, -1, 0}, {7, -2, 0}, {7, -4, 0}, {9, -5, 0}, {7, -9, 0}, {11, -5, 0}, //R
//     53           54           55           56
	{5, -1, 0}, {1, -1, 0}, {3, -1, 0}, {3, -9, 0}, //T
//    57          58          59          60          61          62
	{-1, -1, 0}, {-1, -9, 0}, {-1, -5, 0}, {-5, -5, 0}, {-5, -1, 0}, {-5, -9, 0}, //H
//    63          64          65           66           67           68           69          70
	{-7, -1, 0}, {-7, -9, 0}, {-9, -9, 0}, {-10, -8, 0}, {-11, -6, 0}, {-11, -4, 0}, {-10, -2, 0}, {-9, -1, 0}, //D
//     71           72           73           74           75
	{-13, -9, 0}, {-15, -1, 0}, {-17, -9, 0}, {-14, -5, 0}, {-16, -5, 0}, //A
//     76           77           78           79
	{-21, -5, 0}, {-19, -1, 0}, {-23, -1, 0}, {-21, -9, 0}, //Y
};
int happy_line_indices[60][2] =
{
	{0, 1}, {2, 3}, {4, 5}, //H
	{6, 7}, {7, 8}, {9, 10}, //A
	{11, 12}, {12, 13}, {13, 14}, {14, 15}, {15, 16}, {16, 17}, //P
	{18, 19}, {19, 20}, {20, 21}, {21, 22}, {22, 23}, {23, 24}, //P
	{25, 26}, {25, 27}, {25, 28}, //Y
	{29, 30}, {30, 31}, {31, 32}, {32, 33}, {33, 34}, {34, 35}, {35, 36}, {36, 37}, {37, 29}, {34, 38}, //B
	{39, 40}, {41, 42}, {43, 44}, //I
	{45, 46}, {46, 47}, {47, 48}, {48, 49}, {49, 50}, {50, 51}, {50, 52}, //R
	{53, 54}, {55, 56}, //T
	{57, 58}, {59, 60}, {61, 62}, //H
	{63, 64}, {64, 65}, {65, 66}, {66, 67}, {67, 68}, {68, 69}, {69, 70}, {70, 63}, //D
	{71, 72}, {72, 73}, {74, 75}, //A
	{76, 77}, {76, 78}, {76, 79}, //Y
};

Vec3 line_x[2] = {{-1, 0, 0}, {1, 0, 0}};
Vec3 line_y[2] = {{0, -1, 0}, {0, 1, 0}};
Vec3 line_z[2] = {{0, 0, -1}, {0, 0, 1}};
int line_line_indices[1][2] = {{0, 1}};

//Vec3 pov = {-2, 2, -2};
#define CAMX (-10)
#define CAMY (2)
#define CAMZ (-25)
float dist = 25;
float t = 0;
float scale = 5;
Vec2 offset = {64, 15};

enum State
{
	DVD,
	THREED,
};

enum Rot
{
	X = 1,
	Y = 2,
	Z = 4,
};

enum Model
{
	BIRTHDAY,
	CUBE,
	PYRAMID,
	LINE_X,
	LINE_Y,
	LINE_Z,
};

uint8_t current_rot = 0;
Model current_model = CUBE;

int8_t pos_x = 0, pos_y = 0;
int8_t vel_x = 1, vel_y = 1;

String text = "Happy Birthday";
ESP8266WebServer server(80);
State current_state = THREED;

void drawLines(const Vec2 coords[], const int line_indices[][2], const int line_index_count)
{
	for(int i = 0; i < line_index_count; i++)
	{
		Heltec.display->drawLine(Heltec.display->width() - floor(coords[line_indices[i][0]].x * scale + offset.x),
			Heltec.display->height() - floor(coords[line_indices[i][0]].y * scale + offset.y),
			Heltec.display->width() - floor(coords[line_indices[i][1]].x * scale + offset.x),
			Heltec.display->height() - floor(coords[line_indices[i][1]].y * scale + offset.y));
	}
}

void proj(/*const Vec3 cam, */const float plane, const Vec3 coords[], const int coord_count, Vec2 *ret)
{
	for(int i = 0; i < coord_count; i++)
	{
		float a = (plane - CAMZ /*cam.z*/) / (coords[i].z - CAMZ /*cam.z*/);
		ret[i].x = CAMX /*cam.x*/ + a * (coords[i].x - CAMX /*cam.x*/);
		ret[i].y = CAMY /*cam.y*/ + a * (coords[i].y - CAMY /*cam.y*/);
	}
}

void draw3d(const Vec3 coords[], const int line_indices[][2], const int coord_count, const int line_index_count)
{
	Vec2 ret[coord_count];
	proj(/*pov,*/ dist, coords, coord_count, ret);
	drawLines(ret, line_indices, line_index_count);
}

/*Vec3 matvecmul(const Vec3 m[3], const Vec3 v)
{
	Vec3 ret;
	ret.x = m[0].x * v.x + m[1].x * v.y + m[2].x * v.z;
	ret.z = m[0].y * v.x + m[1].y * v.y + m[2].y * v.z;
	ret.y = m[0].z * v.x + m[1].z * v.y + m[2].z * v.z;
	return ret;
}*/

void rot3d(const Vec3 rot[], const int point_count, Vec3 *ret)
{
	for(int i = 0; i < point_count; i++)
	{
		Vec3 v = ret[i];
		ret[i].x = rot[0].x * v.x + rot[1].x * v.y + rot[2].x * v.z;
		ret[i].z = rot[0].y * v.x + rot[1].y * v.y + rot[2].y * v.z;
		ret[i].y = rot[0].z * v.x + rot[1].z * v.y + rot[2].z * v.z;
		//ret[i] = matvecmul(rot, ret[i]);
	}
}

void handleRoot()
{
	if(server.method() == HTTP_POST)
	{
		const String t = server.arg("plain");
		if(t.startsWith("dvd="))
		{
			current_state = DVD;
			text = t;
			text = server.urlDecode(text.substring(4));
			text.remove(text.length() - 2);
			pos_x = 2;
			pos_y = 2;
			vel_x = 1;
			vel_y = 1;
		}
		else if(t.startsWith("threed="))
		{
			current_state = THREED;
			current_rot = 0;
			if(t.indexOf("threed=bday") >= 0)
			{
				current_model = BIRTHDAY;
			}
			else if(t.indexOf("threed=cube") >= 0)
			{
				current_model = CUBE;
			}
			else if(t.indexOf("threed=pyramid") >= 0)
			{
				current_model = PYRAMID;
			}
			else if(t.indexOf("threed=linex") >= 0)
			{
				current_model = LINE_X;
			}
			else if(t.indexOf("threed=liney") >= 0)
			{
				current_model = LINE_Y;
			}
			else if(t.indexOf("threed=linez") >= 0)
			{
				current_model = LINE_Z;
			}
			if(t.indexOf("x=X") >= 0)
			{
				current_rot += X;
			}
			if(t.indexOf("y=Y") >= 0)
			{
				current_rot += Y;
			}
			if(t.indexOf("z=Z") >= 0)
			{
				current_rot += Z;
			}
		}
		else
		{
			server.send(418, "text/html", "I'm a teapot");
		}
	}
	server.send(200, "text/html",
		"<head><title>Geburtstag</title></head><body><h1>" + text +
		"</h1><h2>DVD</h2><form method=\"post\" enctype=\"text/plain\" action=\"/\"><input type=\"text\" name=\"dvd\" value=\"" + text +
		"\"/><br><input type=\"submit\" value=\"Submit\"/><h2>3D</h2></form><form method=\"post\" enctype=\"text/plain\" action=\"/\"><select name=\"threed\" id=\"model_select\"><option value=\"bday\" " + 
		((current_model == BIRTHDAY) ? "selected" : "") +
		">Happy Birthday</option><option value=\"cube\" " + ((current_model == CUBE) ? "selected" : "") +
		">Cube</option><option value=\"pyramid\" " + ((current_model == PYRAMID) ? "selected" : "") +
		">Pyramid</option><option value=\"linex\" " + ((current_model == LINE_X) ? "selected" : "") +
		">Line X</option><option value=\"liney\" " + ((current_model == LINE_Y) ? "selected" : "") +
		">Line Y</option><option value=\"linez\" " + ((current_model == LINE_Z) ? "selected" : "") +
		">Line Z</option></select><br/><input " + (((current_rot & X) == X) ? "checked" : "") +
		" id=\"x_check\" type=\"checkbox\" name=\"x\" value=\"X\"/><label for=\"x_check\">X</label><input " + (((current_rot & Y) == Y) ? "checked" : "") +
		" id=\"y_check\" type=\"checkbox\" name=\"y\" value=\"Y\"/><label for=\"y_check\">Y</label><input " + (((current_rot & Z) == Z) ? "checked" : "") +
		" id=\"z_check\" type=\"checkbox\" name=\"z\" value=\"Z\"/><label for=\"z_check\">Z</label></br><input type=\"submit\" value=\"Submit\"/></form>" +
		"</br>The (admittedly incredibly ugly) source code can be found <a href=\"https://github.com/0x4261756D/ESP8266_OLED\">here</a>. " +
		"It can be hard to find correct documentation for this, apart from the surprisingly good information the Arduino IDE (v.2, not v.1) provides " +
		"<a href=\"https://heltec-automation-docs.readthedocs.io/en/latest/esp8266%2Barduino/quick_start.html#via-arduino-board-manager\">this</a> " +
		"is helpful to get started (but beware, they get the location of the examples wrong...) and <a href=\"https://github.com/HelTecAutomation/Heltec_ESP8266\">the OLED library</a> even has examples.</body>");
}

void setup()
{
	// put your setup code here, to run once:
	Heltec.begin(true, true);
	Heltec.display->init();
	Heltec.display->setFont(ArialMT_Plain_10);
	// PLEASE CHANGE THIS IF YOU WANT TO USE THIS FOR YOURSELF
	WiFi.softAP("Alles Gute zum Geburtstag", "12345678" /*If everybody uses this password it can't be bad, can it? .)*/);
	server.on("/", handleRoot);
	server.begin();
}

void loop()
{
	server.handleClient();
	// put your main code here, to run repeatedly:
	Heltec.display->clear();
	if(current_state == DVD)
	{
		Heltec.display->drawString(pos_x, pos_y, text);

		pos_x += vel_x;
		pos_y += vel_y;

		if(pos_x >= Heltec.display->width() - Heltec.display->getStringWidth(text.c_str(), text.length()) || pos_x < 0)
		{
			vel_x *= -1;
			pos_x += 2 * vel_x;
		}
		if(pos_y >= Heltec.display->height() - 10 || pos_y < 0)
		{
			vel_y *= -1;
			pos_y += 2 * vel_y;
		}
		delay(50);
	}
	else if(current_state == THREED)
	{
		int vertex_count = 0;
		int line_count = 0;
		if(current_model == PYRAMID)
		{
			vertex_count = 5;
			line_count = 8;
		}
		else if(current_model == CUBE)
		{
			vertex_count = 8;
			line_count = 12;
		}
		else if(current_model == BIRTHDAY)
		{
			vertex_count = 80;
			line_count = 60;
		}
		else
		{
			vertex_count = 2;
			line_count = 1;
		}
		Vec3 rotated_model[vertex_count];
		int line_indices[line_count][2];
		// {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}};
		Vec3 rot_around_x[3] = {{1, 0, 0}, {0, cos(t), -sin(t)}, {0, sin(t), cos(t)}};
		Vec3 rot_around_y[3] = {{cos(t), -sin(t), 0}, {0, 0, 1}, {sin(t), cos(t), 0}};
		Vec3 rot_around_z[3] = {{cos(t), 0, sin(t)}, {-sin(t), 0, cos(t)}, {0, 1, 0}};
		if(current_model == PYRAMID)
		{
			std::copy(pyramid, pyramid + vertex_count, rotated_model);
			std::copy(pyramid_line_indices, pyramid_line_indices + line_count, line_indices);
			scale = 8;
			offset = {-10, 25};
		}
		else if(current_model == CUBE)
		{
			std::copy(cube, cube + vertex_count, rotated_model);
			std::copy(cube_line_indices, cube_line_indices + line_count, line_indices);
			scale = 5;
			offset = {0, 25};
		}
		else if(current_model == BIRTHDAY)
		{
			std::copy(happy, happy + vertex_count, rotated_model);
			std::copy(happy_line_indices, happy_line_indices + line_count, line_indices);
			scale = .5;
			offset = {64, 16};
		}
		else if(current_model == LINE_X)
		{
			std::copy(line_x, line_x + vertex_count, rotated_model);
			std::copy(line_line_indices, line_line_indices + line_count, line_indices);
			scale = 8;
			offset = {0, 25};
		}
		else if(current_model == LINE_Y)
		{
			std::copy(line_y, line_y + vertex_count, rotated_model);
			std::copy(line_line_indices, line_line_indices + line_count, line_indices);
			scale = 8;
			offset = {0, 25};
		}
		else if(current_model == LINE_Z)
		{
			std::copy(line_z, line_z + vertex_count, rotated_model);
			std::copy(line_line_indices, line_line_indices + line_count, line_indices);
			scale = 8;
			offset = {0, 25};
		}
		if((current_rot & X) == X)
		{
			rot3d(rot_around_x, vertex_count, rotated_model);
		}
		if((current_rot & Y) == Y)
		{
			rot3d(rot_around_y, vertex_count, rotated_model);
		}
		if((current_rot & Z) == Z)
		{
			rot3d(rot_around_z, vertex_count, rotated_model);
		}
		draw3d(rotated_model, line_indices, vertex_count, line_count);
		t += .01;
	}
	Heltec.display->display();
}






