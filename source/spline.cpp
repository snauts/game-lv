#include "game.h"

// Basic PointStruct functions & operators

float PointStruct::length() {
	return(sqrt(pow(x, 2) + pow(y, 2)));
}

PointStruct PointStruct::operator* (const float &rhs) {
	PointStruct returnValue;
	returnValue.x = this->x * rhs;
	returnValue.y = this->y * rhs;
	return returnValue;
}

PointStruct PointStruct::operator/ (const float &rhs) {
	PointStruct returnValue;
	returnValue.x = this->x / rhs;
	returnValue.y = this->y / rhs;
	return returnValue;
}

PointStruct PointStruct::operator+ (const PointStruct &rhs) {
	PointStruct returnValue;
	returnValue.x = this->x + rhs.x;
	returnValue.y = this->y + rhs.y;
	return returnValue;
}

PointStruct PointStruct::operator- (const PointStruct &rhs) {
	PointStruct returnValue;
	returnValue.x = this->x - rhs.x;
	returnValue.y = this->y - rhs.y;
	return returnValue;
}

// Spline class member functions

bool Spline::pushBackPoint(PointStruct point) {
	splinePoints.push_back(point);
	return true;
}

bool Spline::getPoints(vector <PointStruct> &points) {
	points.clear();

	for(int temp = 0; temp < splinePoints.size(); temp++)
		points.push_back(splinePoints[temp]);

	return true;
}

bool Spline::setPoints(vector <PointStruct> &points) {
	splinePoints.clear();

	for(int temp = 0; temp < points.size(); temp++)
		splinePoints.push_back(points[temp]);

	return true;
}

bool Spline::flush() {
	splinePoints.clear();
	return true;
}

PointStruct Spline::normalize(PointStruct a, float n) {
	a = a * n / a.length();
	return a;
}

PointStruct Spline::algorithm(PointStruct a, PointStruct aPrim, PointStruct bPrim, PointStruct b, float step) {
	PointStruct returnPoint;
	
	returnPoint = a * pow(1 - step, 3) + aPrim * 3.0 * pow(1 - step, 2) * step + bPrim * 3.0 * pow(step, 2) * (1 - step) + b * pow(step, 3);
	
	return returnPoint;
}

bool Spline::getPath(vector <PointStruct> &pathPoints, int subPointCount) {
	if(splinePoints.size() <= 1) return false;
	
	pathPoints.clear();

	PointStruct a = splinePoints[0], b;
	PointStruct aPrim, bPrim, tempPoint, tempPrim;
	
	for(int pointIterator = 1; pointIterator < splinePoints.size(); pointIterator++) {
		b = splinePoints[pointIterator];

		if(pointIterator == 1)
			aPrim = a;
		else
			aPrim = tempPrim;
		 
		if(pointIterator < splinePoints.size() - 1) {
			PointStruct c = splinePoints[pointIterator + 1];
			PointStruct z = a + (c - a) / 2.0;
			bPrim = b - normalize(z - a, (b - a).length() / 2.0);
			tempPrim = b + normalize(c - z, (c - b).length() / 2.0);
		}
		else
			bPrim = b;
		
		for(int temp = 0; temp < subPointCount; temp++) {
			float step = (float)temp / (float)subPointCount;
			tempPoint = algorithm(a, aPrim, bPrim, b, step);
			pathPoints.push_back(tempPoint);
		}

		a = b;
	}

	pathPoints.push_back(splinePoints[splinePoints.size() - 1]);

	return true;
}
