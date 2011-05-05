#ifndef __SPLINE_H__
#define __SPLINE_H__

#include "game.h"

struct PointStruct {
	float x, y;
	float length();
	PointStruct operator* (const float &rhs);
	PointStruct operator/ (const float &rhs);
	PointStruct operator+ (const PointStruct &rhs);
	PointStruct operator- (const PointStruct &rhs);
};

class Spline {
public:
	bool pushBackPoint(PointStruct point);
	bool getPath(vector <PointStruct> &pathPoints, int subPointCount);
	bool getPoints(vector <PointStruct> &points);
	bool setPoints(vector <PointStruct> &points);
	bool flush();

private:
	PointStruct algorithm(PointStruct a, PointStruct aPrim, PointStruct bPrim, PointStruct b, float step);
	PointStruct normalize(PointStruct a, float n);
	vector <PointStruct> splinePoints;
};

#endif
