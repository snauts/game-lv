#ifndef __PATH_H__
#define __PATH_H__

#pragma warning(disable : 4786)

#include <algorithm>
#include <vector>
#include <map>
#include "game.h"
#include "graph.h"


class WalkMask {
public:
	// Called when game is loaded.
	WalkMask();
	// Called when game starts for first time.
	WalkMask(const string &fileName);
	~WalkMask();

	bool isOk() { return valid; }
	unsigned loadWalkMask(const string& file);

	int saveWalkMask(FILE *fout);
	int readWalkMask(FILE *fin);

	// Called when user enters/exits scene.
	int loadData();
	int freeData();

	int getWidth() { return w; }
	int getHeight() { return h; }

	int startNewPath(CoordinatePoint &currentCoordinate, int x, int y, long timeNow);
	int getNextPoint(CoordinatePoint &currentCoordinate, long timeNow);
	void resetPath(CoordinatePoint &currentCoordinate);
	int getTimeSkip();

	// newSpeed - speed in pixels/sec
	int setSpeed(int newSpeed);
 
	inline bool isOnMask(int x, int y) { return(mask[y * w + x] ? true : false); }

	bool findClosestMaskPoint(int &maskx, int &masky);

	inline void updateTime(int newTime) { lastTime = newTime; }

	int addGraphNode();
	int delGraphNode(int nodeNum);
	int addGraphEdge(int nodeFrom, int noteto);
	int delGraphEdge(int nodeFrom, int nodeTo);

	int setGraphLineState(int lineID, bool enabled) { return graph.setGraphLineState(lineID, enabled); }

	// Debug f-jas
	unsigned char *maska() { return(mask); }
	Graph &wGraph() { return(graph); }
	string &name() { return(walkMaskName); }
		
protected:
	bool valid;
	int w, h;				
	unsigned char *mask;
	Graph graph;
	string walkMaskName;

	// Needed for storing currently found path
	int pathPointCount;
	CoordinatePoint *activePath;

	// Needed to regulate speed
	int speed;				
	int lastTime;

	// Needed for getNextPoint computation
	int edgeNum;
	int edgeProgress;
	CoordinatePoint lastPoint;	
	int lastScript;
	double scaleDelta;

	double slopines, height;
};


#endif

