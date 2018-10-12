#pragma once
#ifndef MICRONTRACKER_20161028_150100
#define MICRONTRACKER_20161028_150100

#pragma	comment(lib, "winmm.lib")

#include	<windows.h>
#include	<iostream>
#include	<vector>

#ifdef	_WIN64
	#pragma	comment(lib, "MicronTracker/x64/Dist64/MTC.lib")
	#include	"MicronTracker/x64/Dist64/MTC.h"
	#pragma comment(lib, "MicronTracker/x64/MicronTracker.lib")
#else
	#pragma	comment(lib, "MicronTracker/x86/Dist/MTC.lib")
	#include	"MicronTracker/x86/Dist/MTC.h"
	#pragma comment(lib, "MicronTracker/x86/MicronTracker.lib")
#endif

class MARKER {
public:
	bool	isIdentified;	// åüèo
	std::string	name;		// ìoò^ñºèÃ
	double	pos[3];			// à íuèÓïÒ
	double	ang[3];			// äpìx
	double	rot[9];			// âÒì]çsóÒ
	mtMeasurementHazardCode Hazard;

	MARKER() :isIdentified(false) {};

	MARKER(const MARKER &source) {
		isIdentified = source.isIdentified;
		name = source.name;
		for (auto i = 0; i<3; i++) pos[i] = source.pos[i];
		for (auto i = 0; i<3; i++) ang[i] = source.ang[i];
		for (auto i = 0; i<9; i++) rot[i] = source.rot[i];
		Hazard = source.Hazard;
	}

	bool operator==(const MARKER &m) const {
		return this->name == m.name;
	}

};

class __declspec(dllexport)MicronTracker
{
public:
	MicronTracker();
	~MicronTracker();

	void mtcCheck(int result) const;
	void loadMT_Files() const;

	int initMT();
	std::vector<MARKER> getMicronTrackerMarker();

private:
	mtHandle IdentifiedMarkers;
	mtHandle PoseXf;
	mtHandle CurrCamera;
	mtHandle IdentifyingCamera;
};

#endif // MICRONTRACKER_20161028_150100