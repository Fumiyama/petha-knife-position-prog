#pragma once
#include	<iostream>
#include	<string>
#include	<vector>

#include	<GL/glut.h>
#include	<opencv2/opencv.hpp>

#include	"MicronTracker/MicronTracker.h"
#include	"ShMem/ShMem.h"

class metzMARKER : public MARKER {
public:
	auto operator=(MARKER &m);
};

class Position {
public:
	std::string name;
	double x, y, z;
	Position() :x(0.0), y(0.0), z(0.0) {};
	Position(std::string name, double pos[3]) :name(name), x(pos[0]), y(pos[1]), z(pos[2]) {};

	bool operator==(const metzMARKER &m) const;

	void set(double pos[3]);

};

class RelativePosition {
private:
	std::vector<metzMARKER> m;
	std::vector<Position> relPosVecs;

public:

	void setMarker(std::vector<MARKER> &marker);

	std::vector<metzMARKER> find(std::string name);

	metzMARKER getFindTop(std::string name);

	bool relativePosVector(std::string mAName, std::string mBName);

	std::vector<Position> getTips();

};

struct SHMEM_KNIFE {
	double x, y, z;
	double xt, yt, zt;
	double s_pos[3];
};

const int WINDOW_WIDTH_SIZE = 800;
const int WINDOW_HEIGHT_SIZE = 800;
const std::string WINDOW_TITLE = "MicronTracker Knife Tip Pos";

const std::string TIP_MARKER_NAME = "01";
const std::string BODY_MARKER_NAME[] = { "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12" };

const std::string SHM_NAME = "KNIFE";

void idle();
void initGL();
void resize(int w, int h);
void drawAxis(double scale, double len);
void viewMarker(std::vector<MARKER> m, GLfloat col[4]);
void keyboard(unsigned char k, int x, int y);
