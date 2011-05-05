#include "game.h"

const int startKeyValue = 1;
const int endKeyValue = 2;

//*****************************************************

// G r a p h

int Graph::loadGraph(FILE *fin, unsigned char* mask, int w, int h) {
	validGraph = false;

	if(!IsNextString(fin, "graph")) return(1);

	while(!IsNextString(fin, "end_graph")) {
		if(feof(fin)) {
			destroy();
			return(PATHFINDING_FUNC_ERROR);
		}

		if(!IsNextString(fin, "graph_node")) {
			destroy();
			return(PATHFINDING_FUNC_ERROR);
		}

		GraphNode *newNode = new GraphNode;
		int num = LoadInt(fin);
		newNode->x = LoadInt(fin);
		newNode->y = LoadInt(fin);
		newNode->scale = LoadInt(fin);
		if(newNode->scale == -1) newNode->scale =
			mask[newNode->x + newNode->y * w];
			
		if(!IsNextString(fin, "graph_node_lines")) {
			destroy();
			return(PATHFINDING_FUNC_ERROR);
		}

		GraphLine tmpLine;
		while(!IsNextString(fin, "end_graph_node_lines")) {
			if(feof(fin)) {
				destroy();
				return(PATHFINDING_FUNC_ERROR);
			}

			tmpLine.node = LoadInt(fin);
			tmpLine.startScript = LoadInt(fin);
			tmpLine.endScript = LoadInt(fin);
			if(LoadInt(fin) == 0) tmpLine.jumpable = false; else tmpLine.jumpable = true;
			if(LoadInt(fin) == 0) tmpLine.shouldInterpolate = false; else tmpLine.shouldInterpolate = true;

			tmpLine.lineID = -1;
			tmpLine.enabled = true;

			if(IsNextString(fin, "line_enable_tag")) {
				tmpLine.lineID = LoadInt(fin);
				if(LoadInt(fin) == 0) tmpLine.enabled = false; else tmpLine.enabled = true;
				if(!IsNextString(fin, "end_line_enable_tag")) {
					destroy();
					return(PATHFINDING_FUNC_ERROR);
				}
			}

			newNode->lines.push_back(tmpLine);
		}

		newNode->lineCount = newNode->lines.size();

		if(!IsNextString(fin, "end_graph_node")) {
			destroy();
			return(PATHFINDING_FUNC_ERROR);
		}

		nodes.insert(make_pair<int, GraphNode*>(num, newNode));
	}

	nodeCount = nodes.size();

	// Calculate ranges

	map<int, GraphNode*>::iterator nodeIterator = nodes.begin();

	while(nodeIterator != nodes.end()) {
		for(int draza = 0; draza < nodeIterator->second->lineCount; draza++) {
			map<int, GraphNode*>::iterator nextIterator = nodes.find(nodeIterator->second->lines[draza].node);
			if (nextIterator == nodes.end()) {
				nodeIterator->second->lines.erase(nodeIterator->second->lines.begin() + draza);
				nodeIterator->second->lineCount--;
				continue;
			} // deleletion of edge whitch points to noexisting node
			int dx = nodeIterator->second->x - nextIterator->second->x;
			int dy = nodeIterator->second->y - nextIterator->second->y;
			nodeIterator->second->lines[draza].cost = sqrt((float)(dx * dx + dy * dy));
		}
		
		nodeIterator++;
	}

	validGraph = true;
	return(PATHFINDING_FUNC_OK);
}

void Graph::makeEmpty() {
	destroy();
	validGraph = true;
	return;
}

bool Graph::destroy() {
	map<int, GraphNode*>::iterator nodeIterator = nodes.begin();
	
	while(nodeIterator != nodes.end()) {
		delete nodeIterator->second;
		nodeIterator++;
	}

	nodes.clear();
	
	nodeCount = 0;
	return(true);
}

Graph::Graph() {
	destroy();
	validGraph = false;
}

Graph::~Graph() {
	destroy();
	validGraph = false;
}

int Graph::findPath(CoordinatePoint &src, int dstx, int dsty, CoordinatePoint *pointList, unsigned char *mask, int w, int h) {		
	dout << endl;
	if (src.y == dstx && src.y == dsty) return(PATHFINDING_FUNC_OK);
		
//	dout << "--- findPath: saakuma nodes noteikshana." << endl;
	GraphNode *srcNode;
	srcNode = new GraphNode;
	bool jumpable = true;

//	dout << "--- findPath: paarbaudam vai vispaar taa viegli var noiet no shiis skjautes." << endl;
	if (nodes.find(src.nodeFrom) != nodes.end())
		for(int lineCounter = 0; lineCounter < nodes.find(src.nodeFrom)->second->lineCount; lineCounter++)
			if (nodes.find(src.nodeFrom)->second->lines[lineCounter].node == src.nodeTo) {
				jumpable = nodes.find(src.nodeFrom)->second->lines[lineCounter].jumpable;
				break;
			}

	if(!jumpable || mask[src.y * w + src.x] == notWalkableValue && src.nodeFrom >= 0 && src.nodeTo >= 0) {
		if (src.nodeFrom == src.nodeTo) {
//			dout << "--- findPath: saakuma punkts nav uz maskas uz nodes." << endl;
			srcNode->x = nodes.find(src.nodeFrom)->second->x;
			srcNode->y = nodes.find(src.nodeFrom)->second->y;
			srcNode->lineCount = nodes.find(src.nodeFrom)->second->lineCount;
			srcNode->lines = nodes.find(src.nodeFrom)->second->lines;
			srcNode->scale = nodes.find(src.nodeFrom)->second->scale;
			srcNode->extra = 0;
		}
		else {	
//			dout << "--- findPath: saakuma punkts nav uz maskas starp divaam nodeem." << endl;
			srcNode->x = src.x;
			srcNode->y = src.y;
			srcNode->extra = 0;
			srcNode->scale = (int)(src.scaleCoefficient * 100);
			srcNode->lineCount = 0;
			GraphLine tmpLine;
			GraphNode *tmpNode;

//			dout << "--- findPath: skjautnes konstrueeshana uz vienu no nodeem." << endl;
			tmpNode = nodes.find(src.nodeFrom)->second;
			int draza = 0;
			for(draza = 0; draza < tmpNode->lineCount; draza++)
				if (tmpNode->lines[draza].node == src.nodeTo) {
					tmpLine.endScript = tmpNode->lines[draza].endScript;
					tmpLine.jumpable = tmpNode->lines[draza].jumpable;
					tmpLine.enabled = tmpNode->lines[draza].enabled;
					tmpLine.shouldInterpolate = tmpNode->lines[draza].shouldInterpolate;

					tmpNode = nodes.find(src.nodeTo)->second;
					tmpLine.node = src.nodeTo;
					tmpLine.lineID = -1;
					tmpLine.startScript = -1;
					tmpLine.cost = sqrt(pow((float)(src.x - tmpNode->x), 2) + pow((float)(src.y - tmpNode->y), 2)); 
					srcNode->lines.push_back(tmpLine);
					srcNode->lineCount++;
					break;
				}

//			dout << "--- findPath: skjautnes konstrueeshana uz otru no nodeem." << endl;
			tmpNode = nodes.find(src.nodeTo)->second;
			for(draza = 0; draza < tmpNode->lineCount; draza++)
				if (tmpNode->lines[draza].node == src.nodeFrom) {
					tmpLine.endScript = tmpNode->lines[draza].endScript;
					tmpLine.jumpable = tmpNode->lines[draza].jumpable;
					tmpLine.enabled = tmpNode->lines[draza].enabled;
					tmpLine.shouldInterpolate = tmpNode->lines[draza].shouldInterpolate;
					
					tmpNode = nodes.find(src.nodeFrom)->second;
					tmpLine.node = src.nodeFrom;
					tmpLine.lineID = -1;
					tmpLine.startScript = -1; 
					tmpLine.cost = sqrt(pow((float)(src.x - tmpNode->x), 2) + pow((float)(src.y - tmpNode->y), 2)); 
					srcNode->lines.push_back(tmpLine);
					srcNode->lineCount++;
					break;
				}
		}
	} 
	else {
//		dout << "--- findPath: saakuma punkts atrodas uz maskas." << endl;
		srcNode->x = src.x;
		srcNode->y = src.y;
		srcNode->lineCount = 0;
		srcNode->extra = 0;
		srcNode->scale = (int)mask[src.y * w + src.x];
	}

//	dout << "--- findPath: beigu nodes noteikshana." << endl;
	GraphNode *dstNode = new GraphNode;
	dstNode->x = dstx;
	dstNode->y = dsty;
	dstNode->lineCount = 0;
	dstNode->extra = 0;
	dstNode->scale = (int)mask[dsty * w + dstx];

//	dout << "--- findPath: kalkulee saakuma un beigu nodu atsleegas." << endl;
	map<int, GraphNode*>::iterator lastIterator = nodes.end();
	int startKey, endKey;
	if (lastIterator == nodes.begin()) {
		startKey = startKeyValue;
		endKey = endKeyValue;
	}
	else {
		lastIterator--;
		startKey = lastIterator->first + startKeyValue;
		endKey = lastIterator->first + endKeyValue;
	}

//	dout << "--- findPath: pievieno grafam jaunaas saakuma un beigu nodes." << endl;
	nodes.insert(make_pair<int, GraphNode*>(startKey, srcNode));
	map<int, GraphNode*>::iterator srcIterator = nodes.find(startKey);
	nodes.insert(make_pair<int, GraphNode*>(endKey, dstNode));
	map<int, GraphNode*>::iterator dstIterator = nodes.find(endKey);

	GraphLine tmpLine = {0, 0, 0.0, -1, -1}; 
	map<int, GraphNode*>::iterator nodeIterator = nodes.begin();

//	dout << "--- findPath: kalkulee skjautnes no paareejam uz jaunajaam nodeem." << endl;
	while(nodeIterator != nodes.end()) {
		if (shootLine(mask, src.x, src.y, nodeIterator->second->x, nodeIterator->second->y, 0, w, h)) {
			if (jumpable && nodeIterator != srcIterator) {
				tmpLine.lineID = -1;
				tmpLine.startScript = -1;
				tmpLine.endScript = -1;
				tmpLine.jumpable = true;
				tmpLine.enabled = true;
				tmpLine.shouldInterpolate = false;
				tmpLine.cost = sqrt(pow((float)(src.x - nodeIterator->second->x), 2) + pow((float)(src.y - nodeIterator->second->y), 2));
				if (nodeIterator != dstIterator) {
					tmpLine.node = nodeIterator->first;
					srcIterator->second->lines.push_back(tmpLine);
					srcIterator->second->lineCount++;
				}
				tmpLine.node = startKey;
				nodeIterator->second->lines.push_back(tmpLine);
				nodeIterator->second->extra++;
			}
		}
		
		if (shootLine(mask, dstx, dsty, nodeIterator->second->x, nodeIterator->second->y, 0, w, h)) {
			if (nodeIterator != dstIterator) {
				if(!jumpable && nodeIterator == srcIterator) {
					nodeIterator++;
					continue;
				}
				tmpLine.lineID = -1;
				tmpLine.startScript = -1;
				tmpLine.endScript = -1;
				tmpLine.jumpable = true;
				tmpLine.enabled = true;
				tmpLine.shouldInterpolate = false;
				tmpLine.cost = sqrt(pow((float)(dstx - nodeIterator->second->x), 2) + pow((float)(dsty - nodeIterator->second->y), 2));
				if (nodeIterator != srcIterator) {
					tmpLine.node = nodeIterator->first;
					dstIterator->second->lines.push_back(tmpLine);
					dstIterator->second->lineCount++;
				}				
				tmpLine.node = endKey;
				nodeIterator->second->lines.push_back(tmpLine);				
				nodeIterator->second->extra++;
			}
		}
		nodeIterator++;
	}

//	dout << "--- findPath: pathfaindinga inits." << endl;
	MasterNodeList g_nodelist;
	PriorityQueue open;
	GraphNode *tmpNode;

//	dout << "--- findPath: start mekleejamaas nodes izveidoshana." << endl;
	SearchNode *startSNode = new SearchNode;
	startSNode->nodeNum = startKey;
	startSNode->cost = 0;
	startSNode->total = sqrt(pow((float)(src.x - dstx), 2) + pow((float)(src.y - dsty), 2));
	startSNode->onOpen = true;
	startSNode->onClosed = false;
	startSNode->parent = NULL;
	startSNode->startTPScript = -1;
	startSNode->endTPScript = -1;
	startSNode->shouldInterpolate = false;
	startSNode->jumpable = true;

	g_nodelist.putNode(startSNode);
	pushPriorityQueue(open, startSNode);

//	dout << "--- findPath: PATHFINDING." << endl;
	while(!isPriorityQueueEmpty(open)) {
		SearchNode *bestnode = popPriorityQueue(open);

		if (bestnode->nodeNum == endKey) {
//			dout << "--- findPath: celjs atrasts." << endl;
			int pointCount = 0;

//			dout << "--- findPath: saskaitam no cik punktiem sastaav muusu celjs." << endl;
			SearchNode *tmpNode = bestnode;
			do { 
				pointCount++; 
				tmpNode = tmpNode->parent;
			} while(tmpNode != NULL);
			
			int pointCycle = pointCount - 1;
			pointList[pointCycle].inScript = bestnode->endTPScript;
			pointList[pointCycle].outScript = -1;
			pointList[pointCycle].shouldInterpolate = bestnode->shouldInterpolate;
			pointList[pointCycle].jumpable = bestnode->jumpable;

//			dout << "--- findPath: paarkaartojam celju otraadaa seciibaa." << endl;
			do {
				/* nodes infs */
				GraphNode *wNode = nodes.find(bestnode->nodeNum)->second;
				pointList[pointCycle].x = wNode->x;
				pointList[pointCycle].y = wNode->y;
				pointList[pointCycle].scaleCoefficient = (double)wNode->scale / 100;

				/* scripti no dotaas skjautnes */
				if(nodes.find(bestnode->nodeNum) == srcIterator || nodes.find(bestnode->nodeNum) == dstIterator) {
					pointList[pointCycle].nodeFrom = -1;
					pointList[pointCycle].nodeTo = -1;
				}
				else {
					pointList[pointCycle].nodeFrom = nodes.find(bestnode->nodeNum)->first;
					pointList[pointCycle].nodeTo = nodes.find(bestnode->nodeNum)->first;
				}

				/* lenkjis */
				if (pointCycle < pointCount - 1) {
					int deltaX = pointList[pointCycle + 1].x - pointList[pointCycle].x;
					int deltaY = pointList[pointCycle + 1].y - pointList[pointCycle].y;
					pointList[pointCycle].angle = atan2(-(double)deltaY, (double)deltaX);
				} 

				if (pointCycle + 1 == pointCount - 1)
					pointList[pointCycle + 1].angle = pointList[pointCycle].angle;

				pointCycle--;

				/* scripti no naakamaas skjautnes */
				if(pointCycle >= 0) {
					pointList[pointCycle].outScript = bestnode->startTPScript;
					
					if (bestnode->parent)
						pointList[pointCycle].inScript = bestnode->parent->endTPScript;
					else
						pointList[pointCycle].inScript = -1;

					pointList[pointCycle].shouldInterpolate = bestnode->shouldInterpolate;
					pointList[pointCycle].jumpable = bestnode->jumpable;
				}

				bestnode = bestnode->parent;
			} while(bestnode != NULL);
			
			while(!isPriorityQueueEmpty(open)) popPriorityQueue(open);			

//			dout << "--- findPath: ja characteris nav uz maskas un atrodas starp divaam skjautneem." << endl;
			/* lai nesanaaktu shaise, ja grib uzreizaam mekleet citu pathu */
			if(mask[src.y * w + src.x] == notWalkableValue && src.nodeFrom != src.nodeTo && src.nodeFrom >= 0 && src.nodeTo >= 0 || !jumpable)
				if (pointList[NEXT_POINT].nodeFrom == src.nodeFrom) {
					pointList[PATH_START_POINT].nodeFrom = src.nodeTo;
					pointList[PATH_START_POINT].nodeTo = src.nodeFrom;
				}
				else {
					pointList[PATH_START_POINT].nodeFrom = src.nodeFrom;
					pointList[PATH_START_POINT].nodeTo = src.nodeTo;
				}
			
			delete srcIterator->second;
			nodes.erase(srcIterator);
			delete dstIterator->second;
			nodes.erase(dstIterator);

//			dout << "--- findPath: aizvaac info par saakuma un beigu nodeem." << endl;
			nodeIterator = nodes.begin();
			while(nodeIterator != nodes.end()) {
				nodeIterator->second->lines.resize(nodeIterator->second->lineCount);
				nodeIterator->second->extra = 0;
				nodeIterator++;
			}
			
			dout << endl;
			return(pointCount);
		}

		/* cikls kuraa mees apstaigaajam visas dotaas nodes kaiminju nodes */
		tmpNode = nodes.find(bestnode->nodeNum)->second;
		for(int temp = 0; temp < tmpNode->lineCount + tmpNode->extra; temp++) {
			if(!tmpNode->lines[temp].enabled) continue;

			SearchNode newSNode;			
			newSNode.nodeNum = tmpNode->lines[temp].node;

			if (bestnode->parent == NULL || bestnode->parent->nodeNum != newSNode.nodeNum) {
				newSNode.parent = bestnode;

				/* uztaisa jaunu mekleejamo nodi (visdaudzsoloshaako) */
				newSNode.startTPScript = tmpNode->lines[temp].startScript;
				newSNode.endTPScript = tmpNode->lines[temp].endScript;
				newSNode.shouldInterpolate = tmpNode->lines[temp].shouldInterpolate;
				newSNode.jumpable = tmpNode->lines[temp].jumpable;

				newSNode.cost = bestnode->cost + tmpNode->lines[temp].cost;
				newSNode.total = newSNode.cost + 
					sqrt(pow((float)(nodes.find(newSNode.nodeNum)->second->x - dstx), 2) + pow((float)(nodes.find(newSNode.nodeNum)->second->y - dsty), 2));

				/* izveelamies apluukojamo nodi */
				SearchNode *actualnode = g_nodelist.getNode(newSNode.nodeNum);
				if (!actualnode) {
					actualnode = new SearchNode;
					actualnode->nodeNum = newSNode.nodeNum;
					actualnode->onClosed = false;
					actualnode->onOpen = false;
					g_nodelist.putNode(actualnode);
				}

				/* paarbauda apluukojamaas nodes statusu */
				if ((!actualnode->onOpen && !actualnode->onClosed) || !(newSNode.total > actualnode->total)) {
					actualnode->startTPScript = newSNode.startTPScript;
					actualnode->endTPScript = newSNode.endTPScript;
					actualnode->shouldInterpolate = newSNode.shouldInterpolate;
					actualnode->jumpable = newSNode.jumpable;
					actualnode->parent = newSNode.parent;
					actualnode->cost = newSNode.cost;
					actualnode->total = newSNode.total;

					if (!actualnode->onOpen) {
						pushPriorityQueue(open, actualnode);
						actualnode->onOpen = true;
					} else updateNodeOnPriorityQueue(open, actualnode);				
				}
			}
		}
		bestnode->onClosed = true;
	}

//	dout << "--- findPath: celjs nav ticis atrasts :(" << endl;
	delete srcIterator->second;
	nodes.erase(srcIterator);
	delete dstIterator->second;
	nodes.erase(dstIterator);
	
//	dout << "--- findPath: aizvaac info par saakuma un beigu nodeem" << endl;
	nodeIterator = nodes.begin();
	while(nodeIterator != nodes.end()) {
		nodeIterator->second->lines.resize(nodeIterator->second->lineCount);
		nodeIterator->second->extra = 0;
		nodeIterator++;
	}

	dout << endl;
	return(-1);
}

int Graph::setGraphLineState(int lineID, bool enabled) {
	map<int, GraphNode*>::iterator nodeIterator = nodes.begin();

	while(nodeIterator != nodes.end()) {
		vector <GraphLine>::iterator lineIterator = nodeIterator->second->lines.begin();
		
		while(lineIterator != nodeIterator->second->lines.end()) {
			if (lineIterator->lineID == lineID) lineIterator->enabled = enabled;
			lineIterator++;
		}
		nodeIterator++; 
	}
	
	return PATHFINDING_FUNC_OK;
}

//*****************************************************

// P r i o r i t y Q u e u e

SearchNode *popPriorityQueue(PriorityQueue &pqueue) {
	SearchNode *node = pqueue.front();

	pop_heap(pqueue.begin(), pqueue.end(), NodeTotalGreater());
	pqueue.pop_back();

	return(node);
}

void pushPriorityQueue(PriorityQueue &pqueue, SearchNode *snode) {
	pqueue.push_back(snode);
	push_heap(pqueue.begin(), pqueue.end(), NodeTotalGreater());
}

bool isPriorityQueueEmpty(PriorityQueue &pqueue) {
	return(pqueue.empty());
}

void updateNodeOnPriorityQueue(PriorityQueue &pqueue, SearchNode *snode) {
	push_heap(pqueue.begin(), pqueue.end(), NodeTotalGreater());
}

//*****************************************************

// N o d e B a n k

SearchNode *MasterNodeList::getNode(int num) {
	map<int, SearchNode*>::iterator tableIterator = table.find(num);
	if (tableIterator == table.end())
		return(NULL);
	else
		return(tableIterator->second);
}

bool MasterNodeList::putNode(SearchNode *snode) {
	map<int, SearchNode*>::iterator tableIterator = table.find(snode->nodeNum);

	if (tableIterator == table.end()) {
		table.insert(make_pair<int, SearchNode*>(snode->nodeNum, snode));
		return(true);
	} 
	else {
		if (tableIterator->second != snode) {
			delete tableIterator->second;
			tableIterator->second = snode;
		}
		return(false);
	}
}

//*****************************************************

// M i s c
/*
int min(int a, int b) {
	return(a > b ? b : a);
}		

int max(int a, int b) {
	return(a > b ? a : b);
}		
*/
int signum(int a) {
	if (a > 0) return(+1);
	if (a < 0) return(-1);
	return(0);
}

void swap(int &a, int &b) {
	int swaper = a;
	a = b;
	b = swaper;
}

bool shootLine(unsigned char *mask, int srcx, int srcy, int dstx, int dsty, char notWalkAble, int w, int h) {
	int deltaX = dstx - srcx, deltaY = dsty - srcy;
	int px = abs(deltaX), PY = abs(deltaY), half;
	int d1X = signum(deltaX), d1Y = signum(deltaY);
	int d2X = signum(deltaX), d2Y = 0;
		
	if (srcx == dstx && srcy == dsty) 
		if(mask[srcy * w + srcx] == notWalkAble)
			return(false);
		else
			return(true);
	
	if (px <= PY) {
		d2X = 0 ;
		d2Y = signum(deltaY);
		px = abs(deltaY);
		PY = abs(deltaX);
	}
	
	half = px / 2;
	bool boolPattern[patternSize] =
		{true, true, true, true, true, true, true, true, true};
	bool shouldCancel;

	for(int Temp = 0; Temp < px + 1; Temp++) {

		shouldCancel = true;
		for(int loop = 0; loop < patternSize; loop++) {
			int index = (srcy + pattern[loop].y) * w + srcx + pattern[loop].x;

			if (index >= w * h || index < 0) {
				boolPattern[loop] = false;
				continue;
			}
			
			if (mask[index] == notWalkAble) 
				boolPattern[loop] = false;

			if (boolPattern[loop]) shouldCancel = false;
		}
		if (shouldCancel) return false;
		
		half += PY;
		if (! (half < px)) {
			half -= px;
			srcx += d1X;
			srcy += d1Y;
		} else {
			srcx += d2X;
			srcy += d2Y;
		}
	}

	return(true);
}

bool moveLine(int &srcx, int &srcy, int dstx, int dsty, int steps) {
	int deltaX = dstx - srcx, deltaY = dsty - srcy;
	int px = abs(deltaX), PY = abs(deltaY), half;
	int d1X = signum(deltaX), d1Y = signum(deltaY);
	int d2X = signum(deltaX), d2Y = 0;
		
	if (srcx == dstx && srcy == dsty) return(true);
	
	if (px <= PY) {
		d2X = 0 ;
		d2Y = signum(deltaY);
		px = abs(deltaY);
		PY = abs(deltaX);
	}
	
	half = px / 2;
	int Temp = 0;
	while(Temp < min(steps, px + 1)) {
		half += PY;
		if (! (half < px)) {
			half -= px;
			srcx += d1X;
			srcy += d1Y;
		} else {
			srcx += d2X;
			srcy += d2Y;
		}
		Temp++;
	}

	return(true);
}

//***********************************************************

// D e b u g

GraphNode *Graph::node_pk(int nodeNum) { 
	map<int, GraphNode*>::iterator nodeIterator = nodes.begin();
	int count = 0;
	while(nodeIterator != nodes.end() && count < nodeNum) {
		nodeIterator++; 
		count++;
	} 
	return(nodeIterator->second); 
} 

int Graph::node_nr(int nodeNum) { 
	map<int, GraphNode*>::iterator nodeIterator = nodes.begin();
	int count = 0;
	while(nodeIterator != nodes.end() && count < nodeNum) {
		nodeIterator++; 
		count++; 
	}
	return(nodeIterator->first); 
} 
GraphNode *Graph::node(int nodeNum) { 
	map<int, GraphNode*>::iterator nodeIterator = nodes.find(nodeNum);
	return(nodeIterator->second); 
} 

//***********************************************************
