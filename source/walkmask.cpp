#include "game.h"

WalkMask::WalkMask() {
	valid = false;
	h = 0;
	w = 0;
	mask = NULL;
	
	pathPointCount = 0;
	activePath = NULL;
	
	speed = 0;
	lastTime = 0;
	
	edgeNum = 0;
	edgeProgress = 0;
	lastScript = -1;

	slopines = 1.0;
	height = 0.0;
}

WalkMask::WalkMask(const string &fileName) {
	valid = false;
	h = 0;
	w = 0;
	mask = NULL;
	
	pathPointCount = 0;
	activePath = NULL;
	
	speed = 0;
	lastTime = 0;
	
	edgeNum = 0;
	edgeProgress = 0;
	lastScript = -1;
	
	slopines = 1.0;
	height = 0.0;

	loadWalkMask(fileName);
}

WalkMask::~WalkMask() {
	if (mask != NULL) {
		delete [] mask;
		mask = NULL;
	}
	if (activePath != NULL) {
		delete [] activePath;
		activePath = NULL;
	}
}


int WalkMask::loadData() {
	mask = loadMask(walkMaskName, w, h);
	if (!mask) return(PATHFINDING_FUNC_ERROR);
	if (graph.isOk()) valid = true;
	return PATHFINDING_FUNC_OK;
}

int WalkMask::freeData() {
	if(valid && mask) {
		delete [] mask;
		mask = NULL;
		valid = false;
		return PATHFINDING_FUNC_OK;
	} 
	if(valid && !mask) {
		valid = false;
		return PATHFINDING_FUNC_ERROR;
	}
	return PATHFINDING_FUNC_ERROR;
}

int WalkMask::saveWalkMask(FILE *fout) {
	/* walk mask info */
	saveInt(fout, (int)valid);
	saveString(fout, walkMaskName);
	saveInt(fout, pathPointCount);
	for(int temp = 0; temp < pathPointCount; temp++)
		CoordinatePoint::saveCoordinatePoint(fout, activePath[temp]);

	saveInt(fout, speed);	
	saveInt(fout, lastTime);	

	saveInt(fout, edgeNum);	
	saveInt(fout, edgeProgress);	
	CoordinatePoint::saveCoordinatePoint(fout, lastPoint);
	saveInt(fout, lastScript);	
	saveDouble(fout, scaleDelta);	
	saveDouble(fout, slopines);	
	saveDouble(fout, height);	

	/* graph info */
	saveInt(fout, (int)graph.isOk());
	saveInt(fout, graph.nodeAmount());

	map <int, GraphNode*> *allNodes = graph.allNodes();

	map<int, GraphNode*>::iterator nodeIterator = allNodes->begin();
	while(nodeIterator != allNodes->end()) {
		saveInt(fout, nodeIterator->first);
		saveInt(fout, nodeIterator->second->x);
		saveInt(fout, nodeIterator->second->y);
		saveInt(fout, nodeIterator->second->extra);
		saveInt(fout, nodeIterator->second->lineCount);
		saveInt(fout, nodeIterator->second->scale);

		for(int draza = 0; draza < nodeIterator->second->lineCount; draza++) {
			saveInt(fout, nodeIterator->second->lines[draza].lineID);
			saveInt(fout, nodeIterator->second->lines[draza].node);
			saveDouble(fout, (double)nodeIterator->second->lines[draza].cost);
			saveInt(fout, nodeIterator->second->lines[draza].startScript);
			saveInt(fout, nodeIterator->second->lines[draza].endScript);
			if(nodeIterator->second->lines[draza].enabled) saveInt(fout, 1); else saveInt(fout, 0);
			if(nodeIterator->second->lines[draza].jumpable) saveInt(fout, 1); else saveInt(fout, 0);
			if(nodeIterator->second->lines[draza].shouldInterpolate) saveInt(fout, 1); else saveInt(fout, 0);
		}

		nodeIterator++; 
	} 

	return PATHFINDING_FUNC_OK;
}

int WalkMask::readWalkMask(FILE *fin) {
	/* walk mask info */
	if (readInt(fin)) valid = true; else valid = false;
	walkMaskName = readString(fin);
	
	if (valid) loadData();

	pathPointCount = readInt(fin);
	if(activePath) delete [] activePath;
	activePath = new CoordinatePoint[pathPointCount];
	int temp;
	for(temp = 0; temp < pathPointCount; temp++)
		CoordinatePoint::readCoordinatePoint(fin, activePath[temp]);
	
	speed = readInt(fin);	
	lastTime = readInt(fin);	

	edgeNum = readInt(fin);	
	edgeProgress = readInt(fin);	
	CoordinatePoint::readCoordinatePoint(fin, lastPoint);
	lastScript = readInt(fin);	
	scaleDelta = readDouble(fin);
	slopines = readDouble(fin);
	height = readDouble(fin);

	/* graph info */
	graph.destroy();

	if (readInt(fin)) graph.isOk(true); else graph.isOk(false); 
	graph.nodeAmount(readInt(fin));
	
	map <int, GraphNode*> *allNodes = graph.allNodes();
	
	for(temp = 0; temp < graph.nodeAmount(); temp++) {
		GraphNode *newLoadNode = new GraphNode;
		int nodeNum = readInt(fin);
		newLoadNode->x = readInt(fin);
		newLoadNode->y = readInt(fin);
		newLoadNode->extra = readInt(fin);
		newLoadNode->lineCount = readInt(fin);
		newLoadNode->scale = readInt(fin);
		
		for(int draza = 0; draza < newLoadNode->lineCount; draza++) {
			GraphLine newLoadLine;
			newLoadLine.lineID = readInt(fin);
			newLoadLine.node = readInt(fin);
			newLoadLine.cost = readDouble(fin);
			newLoadLine.startScript = readInt(fin);
			newLoadLine.endScript = readInt(fin);
			if(readInt(fin) == 0) newLoadLine.enabled = false; else newLoadLine.enabled = true;
			if(readInt(fin) == 0) newLoadLine.jumpable = false; else newLoadLine.jumpable = true;
			if(readInt(fin) == 0) newLoadLine.shouldInterpolate = false; else newLoadLine.shouldInterpolate = true;
			newLoadNode->lines.push_back(newLoadLine);
		}

		allNodes->insert(make_pair<int, GraphNode*>(nodeNum, newLoadNode));
	} 
	
	return PATHFINDING_FUNC_OK;
}

int CoordinatePoint::saveCoordinatePoint(FILE *fout, CoordinatePoint &val) {
	saveInt(fout, val.x);
	saveInt(fout, val.y);
	saveInt(fout, val.inScript);
	saveInt(fout, val.outScript);
	saveDouble(fout, val.scaleCoefficient);
	saveInt(fout, val.nodeFrom);
	saveInt(fout, val.nodeTo);
	saveDouble(fout, val.angle);
	if (val.jumpable) saveInt(fout, 1); else saveInt(fout, 0);
	if (val.shouldInterpolate) saveInt(fout, 1); else saveInt(fout, 0);
	return PATHFINDING_FUNC_OK;
}

int CoordinatePoint::readCoordinatePoint(FILE *fin, CoordinatePoint &val) {
	val.x = readInt(fin);
	val.y = readInt(fin);
	val.inScript = readInt(fin);
	val.outScript = readInt(fin);
	val.scaleCoefficient = readDouble(fin);
	val.nodeFrom = readInt(fin);
	val.nodeTo = readInt(fin);
	val.angle = readDouble(fin);
	if (readInt(fin) == 0) val.jumpable = false; else val.jumpable = true;
	if (readInt(fin) == 0) val.shouldInterpolate = false; else val.shouldInterpolate = true;
	return PATHFINDING_FUNC_OK;
}

unsigned WalkMask::loadWalkMask(const string &file) {
	if(valid) {
		if (mask != NULL) {
			delete [] mask;
			mask = NULL;
		}
		graph.destroy();
		valid = false;
	}
	
	FILE* fin = fopen(convertPath(file).c_str(), "r");
	if (!fin) return(false);
	
	if(!IsNextString(fin, "walkmask")) {
		fclose(fin);
		return(PATHFINDING_FUNC_ERROR);
	}

	walkMaskName = convertPath(LoadString(fin));
	slopines = atof(LoadString(fin).c_str());
	height = atof(LoadString(fin).c_str());

	mask = loadMask(walkMaskName, w, h);
	if (!mask) return(PATHFINDING_FUNC_ERROR);

	if (graph.loadGraph(fin, mask, w, h)) graph.makeEmpty();

	delete [] mask;
	mask = NULL;

	if(!IsNextString(fin, "end_walkmask")) {
		if (mask != NULL) {
			delete [] mask;
			mask = NULL;
		}
		fclose(fin);
		return(PATHFINDING_FUNC_ERROR);
	}

	fclose(fin);
	return(PATHFINDING_FUNC_OK);
}

int WalkMask::setSpeed(int newSpeed) {
	speed = newSpeed;
	return PATHFINDING_FUNC_OK;
}

bool WalkMask::findClosestMaskPoint(int &maskx, int &masky) {
	int dx, dy;
	for(int dist = 1; dist < w + h; dist++) {
		for(float around = 0; around <= 2 * pi; around += searchSpeedConst / dist) {
			dx = (int)(maskx + dist * sin(around));
			dy = (int)(masky + dist * cos(around));
			if(dx >= 0 && dx < w && dy >=0 && dy < h && mask[dy * w + dx]) {
				maskx = dx;
				masky = dy;
				return(true);				
			}
		} 
	}
	return(false);
}

int WalkMask::startNewPath(CoordinatePoint &currentCoordinate, int x, int y, long timeNow) {
	dout << "--- PathFinding: function startNewPath - start" << endl;
	
	if(!isOk()) {
		dout << "--- PathFinding: function startNewPath - end, FAILED!!!" << endl;
		return PATH_NOT_FOUND;
	}

	lastTime = timeNow;

	if (activePath) delete [] activePath;
	
	activePath = new CoordinatePoint[graph.nodeAmount() + extraNodes];

	if (mask[currentCoordinate.y * w + currentCoordinate.x] == notWalkableValue)
		if(lastPoint.nodeFrom == -1 || lastPoint.nodeTo == -1)
			if (!findClosestMaskPoint(currentCoordinate.x, currentCoordinate.y)) {
				dout << "--- PathFinding: function startNewPath - end, FAILED!!!" << endl;
				return PATH_NOT_FOUND;
			}

	if (mask[y * w + x] == notWalkableValue)
		if (!findClosestMaskPoint(x, y)) {
			dout << "--- PathFinding: function startNewPath - end, FAILED!!!" << endl;
			return PATH_NOT_FOUND;
		}

	pathPointCount = graph.findPath(currentCoordinate, x, y, activePath, mask, w, h);
	
	if (pathPointCount <= 0) {
		dout << "--- PathFinding: function startNewPath - end, FAILED!!!" << endl;
		return PATH_NOT_FOUND;
	}
	
	edgeNum = 0;
	edgeProgress = 0;
	lastPoint = activePath[edgeNum]; 

	scaleDelta = activePath[edgeNum + NEXT_POINT].scaleCoefficient - activePath[edgeNum].scaleCoefficient;

	dout << "--- PathFinding: function startNewPath - end success" << endl;

	return PATH_FOUND;
}

int WalkMask::getNextPoint(CoordinatePoint &currentCoordinate, long timeNow) {
	float lastMaskPointScale = currentCoordinate.scaleCoefficient;

	if(edgeNum + NEXT_POINT >= pathPointCount) {
		if(activePath) delete [] activePath;
		activePath = NULL;
		pathPointCount = 0;
		return CHARACTER_ENDPOINT_REACHED;
	} else {
		if(pathPointCount <= 0) {
			return CHARACTER_PATH_BLOCKED;
		} 
	}
		
	float currentSpeed = slopines * (speed * currentCoordinate.scaleCoefficient) + height;
	int jumpPixels = (int)((timeNow - lastTime) * currentSpeed / miliseconds);
	
	if(jumpPixels < 1) {
		lastPoint.inScript = -1;
		lastPoint.outScript = -1;
		currentCoordinate = lastPoint;
		moveLine(lastPoint.x, lastPoint.y, activePath[edgeNum + NEXT_POINT].x, activePath[edgeNum + NEXT_POINT].y, edgeProgress);
		swap(lastPoint.x, currentCoordinate.x);
		swap(lastPoint.y, currentCoordinate.y);
		return CHARACTER_STILL_WALKING;
	}

	float timeDelta = (float)(timeNow - lastTime);

	lastTime = timeNow;
	
	while (timeDelta > 0.0) {
		int lineLength = max(abs(activePath[edgeNum].x - activePath[edgeNum + NEXT_POINT].x), abs(activePath[edgeNum].y - activePath[edgeNum + NEXT_POINT].y));
		if (edgeProgress >= lineLength) {
			lastPoint = activePath[edgeNum + NEXT_POINT];
			lastScript = activePath[edgeNum + NEXT_POINT].outScript;
			activePath[edgeNum + NEXT_POINT].outScript = -1;
			currentCoordinate = activePath[edgeNum + NEXT_POINT];
			edgeProgress = 0;
			edgeNum++;
	
			scaleDelta = activePath[edgeNum + NEXT_POINT].scaleCoefficient - activePath[edgeNum].scaleCoefficient;
			lineLength = max(abs(activePath[edgeNum].x - activePath[edgeNum + NEXT_POINT].x), abs(activePath[edgeNum].y - activePath[edgeNum + NEXT_POINT].y));
		}
		else {
			lastPoint.inScript = -1;
			lastPoint.outScript = -1;
			if (lastScript >= 0) {
				lastPoint.outScript = lastScript;
				lastScript = -1;
			}
			
			currentCoordinate = lastPoint;
			currentCoordinate.nodeTo = activePath[edgeNum + NEXT_POINT].nodeTo;
			moveLine(lastPoint.x, lastPoint.y, activePath[edgeNum + NEXT_POINT].x, activePath[edgeNum + NEXT_POINT].y, edgeProgress);
			swap(lastPoint.x, currentCoordinate.x);
			swap(lastPoint.y, currentCoordinate.y);

			if(lastPoint.shouldInterpolate) {
				lastPoint.scaleCoefficient = activePath[edgeNum].scaleCoefficient + scaleDelta * (double) edgeProgress / (double) lineLength;
				currentCoordinate.scaleCoefficient = lastPoint.scaleCoefficient;
			}
			else {
				if(mask[currentCoordinate.x + currentCoordinate.y * w] == 0)
					lastPoint.scaleCoefficient = lastMaskPointScale;
				else {
					lastPoint.scaleCoefficient = mask[currentCoordinate.x + currentCoordinate.y * w] / 100.0;
					lastMaskPointScale = lastPoint.scaleCoefficient;
				}
				currentCoordinate.scaleCoefficient = lastPoint.scaleCoefficient;
			}
		}
	
		if(edgeNum + NEXT_POINT >= pathPointCount) {
			if(activePath) delete [] activePath;
			activePath = NULL;
			pathPointCount = 0;
			return CHARACTER_ENDPOINT_REACHED;
		}

		if(currentCoordinate.inScript != -1 || currentCoordinate.outScript != -1)
			break;

		if(lastPoint.inScript != -1 || lastPoint.outScript != -1)
			continue;

		edgeProgress += 1;	
		timeDelta -= miliseconds / (slopines * (speed * currentCoordinate.scaleCoefficient) + height);
	}

	return CHARACTER_STILL_WALKING;
}

int WalkMask::getTimeSkip() {
	if(pathPointCount <= 0 || edgeNum + NEXT_POINT >= pathPointCount || !activePath) {
		return 0;
	}
	else {
		float timeSkip = 0.0, localEdgeProgress = edgeProgress;
		CoordinatePoint iterationPoint = lastPoint;
		float lastMaskPointScale = iterationPoint.scaleCoefficient;
		int lineLength = max(abs(activePath[edgeNum].x - activePath[edgeNum + NEXT_POINT].x), abs(activePath[edgeNum].y - activePath[edgeNum + NEXT_POINT].y));

		while(iterationPoint.x != activePath[edgeNum + NEXT_POINT].x || iterationPoint.y != activePath[edgeNum + NEXT_POINT].y) {			
			moveLine(iterationPoint.x, iterationPoint.y, activePath[edgeNum + NEXT_POINT].x, activePath[edgeNum + NEXT_POINT].y, 1);
			if(iterationPoint.shouldInterpolate) {
				iterationPoint.scaleCoefficient = activePath[edgeNum].scaleCoefficient + scaleDelta * (double) localEdgeProgress / (double) lineLength;
			} else {
				if(mask[iterationPoint.x + iterationPoint.y * w] == 0)
					iterationPoint.scaleCoefficient = lastMaskPointScale;
				else {
					iterationPoint.scaleCoefficient = mask[iterationPoint.x + iterationPoint.y * w] / 100.0;
					lastMaskPointScale = iterationPoint.scaleCoefficient;
				}
			}
			timeSkip += miliseconds / (slopines * (speed * iterationPoint.scaleCoefficient) + height); 
			localEdgeProgress++;
		}
		
		for(int temp = edgeNum + NEXT_POINT; temp < pathPointCount - NEXT_POINT; temp++) {
			if(activePath[temp].inScript != -1 || activePath[temp].outScript != -1)	break;

			iterationPoint = activePath[temp];
			localEdgeProgress = 0;

			while(iterationPoint.x != activePath[temp + NEXT_POINT].x || iterationPoint.y != activePath[temp + NEXT_POINT].y) {			
				moveLine(iterationPoint.x, iterationPoint.y, activePath[temp + NEXT_POINT].x, activePath[temp + NEXT_POINT].y, 1);
				if(iterationPoint.shouldInterpolate) {
					iterationPoint.scaleCoefficient = activePath[temp].scaleCoefficient + scaleDelta * (double) localEdgeProgress / (double) lineLength;
				} else {
					if(mask[iterationPoint.x + iterationPoint.y * w] == 0)
						iterationPoint.scaleCoefficient = lastMaskPointScale;
					else {
						iterationPoint.scaleCoefficient = mask[iterationPoint.x + iterationPoint.y * w] / 100.0;
						lastMaskPointScale = iterationPoint.scaleCoefficient;
					}
				}
				timeSkip += miliseconds / (slopines * (speed * iterationPoint.scaleCoefficient) + height); 
				localEdgeProgress++;
			}		
		}
				
		return (int) timeSkip;
	}
}

void WalkMask::resetPath(CoordinatePoint &currentCoordinate) {
	if(activePath) delete [] activePath;
	activePath = NULL;
	pathPointCount = 0;
	
	if(valid && mask[currentCoordinate.y * w + currentCoordinate.x])
		currentCoordinate.scaleCoefficient = 
			(double)mask[currentCoordinate.y * w + currentCoordinate.x] / 100.0;
	
	currentCoordinate.inScript = -1;
	currentCoordinate.outScript = -1;
	currentCoordinate.nodeFrom = -1;
	currentCoordinate.nodeTo = -1;
	currentCoordinate.jumpable = true;	
}

