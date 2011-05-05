#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <vector>
#include <map>

using namespace std;

// ***************************************************************************

// C o n s t s

const int extraNodes = 2;
const int notWalkableValue = 0;
const int miliseconds = 100;
const float pi = (float) 3.141592;
const float sq = (float) 1.414213;

const float searchSpeedConst = (float) 0.4; // (for findClosestMaskPoint)

const int CHARACTER_STILL_WALKING = 0;
const int CHARACTER_ENDPOINT_REACHED = 1;
const int CHARACTER_PATH_BLOCKED = 2;

const int PATH_FOUND = 0;
const int PATH_NOT_FOUND = 1;

const int PATH_START_POINT = 0;
const int NEXT_POINT = 1;

const int PATHFINDING_FUNC_OK = 0;
const int PATHFINDING_FUNC_ERROR = 1;

// 0.40 - search all
// 1.00 - faster, do not search all
// 3.14 - (pi) if search is closer than 2 pixels

// * Structs for advanced shoot line *****************************************

const int patternSize = 9;

struct LinePattern {
	int x, y;
} const pattern[patternSize] = {	
	{-1, -1},	{0, -1},	{1, -1},
	{-1, 0},	{0, 0},		{1, 0},
	{-1, 1},	{0, 1},		{1, 1}};

// ***************************************************************************

// P r i o r i t y Q u e u e

struct SearchNode {
	int nodeNum;

	SearchNode *parent;
	float cost;
	float total;
	int startTPScript; // start to parent script
	int endTPScript; // end to parent script
	int scaleCoefficient; // in percents, if negative - no scale
	bool shouldInterpolate;
	bool jumpable;
	bool onOpen;
	bool onClosed;
};

typedef vector<SearchNode*> PriorityQueue;

class NodeTotalGreater {
public:
	bool operator()(SearchNode *first, SearchNode *second) const {
		return(first->total > second->total);
	}
};

SearchNode *popPriorityQueue(PriorityQueue &pqueue);
void pushPriorityQueue(PriorityQueue &pqueue, SearchNode *snode);
bool isPriorityQueueEmpty(PriorityQueue &pqueue);
void updateNodeOnPriorityQueue(PriorityQueue &pqueue, SearchNode *snode); //?

// ***************************************************************************

// G r a p h

struct GraphLine {
	int node;	// node To
	int lineID;
	float cost;
	int startScript;
	int endScript;
	bool enabled;
	bool jumpable;
	bool shouldInterpolate;
};

struct GraphNode {
	int x, y;
	int extra;
	int lineCount;
	int scale;
	vector <GraphLine> lines;
	GraphNode() {
		lineCount = 0;
		extra = 0;
		scale = 100;
	}
};

struct CoordinatePoint {
	int x;
	int y;
	int inScript;
	int outScript;
	double scaleCoefficient;

	int nodeFrom;
	int nodeTo;

	double angle;
	bool jumpable;
	bool shouldInterpolate;

	CoordinatePoint() : x(0), y(0), inScript(-1), outScript(-1), scaleCoefficient(1.0), nodeFrom(-1), nodeTo(-1), angle(0.0), jumpable(true), shouldInterpolate(true) { }
	static int saveCoordinatePoint(FILE *fout, CoordinatePoint &val);
	static int readCoordinatePoint(FILE *fin, CoordinatePoint &val);
};


class Graph {
protected:
	int nodeCount;
	map <int, GraphNode*> nodes;
	bool validGraph;
public:
	Graph();
	~Graph();

	bool destroy();

	void makeEmpty();

	int loadGraph(FILE *fin, unsigned char* mask, int w, int h);

	inline bool isOk() { return(validGraph); }
	inline void isOk(bool force) { validGraph = force; }

	int findPath(CoordinatePoint &src, int dstx, int dsty, CoordinatePoint *pointList, unsigned char *mask, int w, int h);
			
	inline int nodeAmount() { return (nodeCount); }
	inline void nodeAmount(int force) { nodeCount = force; }

	map <int, GraphNode*> *allNodes() { return &nodes; }

	int setGraphLineState(int lineID, bool enabled);
	
	// Debug f-jas (ps. Es arii vinjas lietoju. Kovacs) //
	GraphNode *node_pk(int nodeNum);
	int node_nr(int nodeNum);
	GraphNode *node(int nodeNum);
};

// ***************************************************************************

// N o d e B a n k

struct MasterNodeList {
	map <int, SearchNode*> table;
	
	MasterNodeList() { table.clear(); }
	
	~MasterNodeList() { 		
		map<int, SearchNode*>::iterator tableIterator = table.begin();
		
		while(tableIterator != table.end()) {
			delete tableIterator->second;
			tableIterator++;
		}
		
		table.clear();
	}
	
	SearchNode *getNode(int num);
	bool putNode(SearchNode *snode);
};

// ***************************************************************************

// M i s c

//int min(int a, int b);
//int max(int a, int b);
int signum(int a);
void swap(int &a, int &b);
bool shootLine(unsigned char *mask, int srcx, int srcy, int dstx, int dsty, char notWalkAble, int w, int h);
bool moveLine(int &srcx, int &srcy, int dstx, int dsty, int steps);

// ***************************************************************************


#endif
