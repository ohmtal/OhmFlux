//-----------------------------------------------------------------------------
// Copyright (c) 2009 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "basicgrid.h"
#include "core/fluxGlobals.h"
#include "utils/errorlog.h"
#include "tBinaryHeap.h"

#include <algorithm>
//-----------------------------------------------------------------------------
/* Constructor */
BasicGrid::BasicGrid()
{
  
  mSquareSize = 0;
  mInitDone	  = false;
  mNodes      = nullptr;
  mDebugGrid = false;  //unused!
  mClientGame = false; //unused!

  mNodeCount = 0;
  mNodesX = 0;
  mNodesY = 0;

}
//-----------------------------------------------------------------------------
/* Destructor */
BasicGrid::~BasicGrid()
{
	mInitDone	  = false;
	SAFE_DELETE_ARRAY(mNodes);
}
//-----------------------------------------------------------------------------------------------------
/* 
TODO:
bool BasicGrid::save(const char *filename)
{

}
*/

//-----------------------------------------------------------------------------
void BasicGrid::init(RectI lArea, F32 lSquareSize)
{
	mInitDone = false;
	SAFE_DELETE_ARRAY(mNodes);
	mArea    = lArea; 
	
	mSquareSize = lSquareSize;

	mNodesX = std::ceil(mArea.len_x() / mSquareSize);
	mNodesY = std::ceil(mArea.len_y() / mSquareSize);
	mNodeCount = mNodesX * mNodesY;

	if (mNodeCount <= 0)
	{
		Log("BasicGrid: Invalid area!");
		return;
	}

	mNodes = new BasicGridNode[getNodeCount()];

	S32 i,j,n;
	BasicGridNode *lNode;
	n = 0;

	for (i=0; i<mNodesX; i++)
		for (j=0; j<mNodesY; j++)
		{

			lNode = &mNodes[i + j * mNodesX]; 
			//using top left position!
			F32 lz = 0.f;
			Point2F lPos = { mArea.getPoint().x + i * mSquareSize, mArea.getPoint().y + j * mSquareSize };
			lNode->setPos(lPos);
			n++;

		}


    mInitDone = true;
}

//-----------------------------------------------------------------------------------------------------
S32 BasicGrid::getNodeIndex(F32 x, F32 y, bool lPrec)
{
	// 1. Check if the point is within the general area first
	if (!mArea.pointInRect({ static_cast<S32>(x), static_cast<S32>(y) }))
		return -1;

	// 2. Calculate the "relative" grid position
	F32 fx = (x - mArea.getPoint().x) / mSquareSize;
	F32 fy = (y - mArea.getPoint().y) / mSquareSize;

	// 3. Convert to integer and CLAMP
	// This ensures that even if x is exactly at the edge, it stays within valid indices
	S32 xNode = std::clamp(static_cast<S32>(std::floor(fx)), 0, mNodesX - 1);
	S32 yNode = std::clamp(static_cast<S32>(std::floor(fy)), 0, mNodesY - 1);

	// 4. Final safety check on the calculated index
	S32 node = xNode + yNode * mNodesX;
	if (node >= 0 && node < mNodeCount)
		return node;

	return -1;
}
//-----------------------------------------------------------------------------------------------------
BasicGridNode* BasicGrid::findNode(F32 x, F32 y, S32 &nodeIndex)
{
	nodeIndex = getNodeIndex(x, y, true);
	if (nodeIndex >= 0)
		return &mNodes[nodeIndex];
	else
		return nullptr;

}

BasicGridNode* BasicGrid::findNode(F32 x, F32 y)
{
	S32 lNodeIndex = getNodeIndex(x,y, true);
	if (lNodeIndex >= 0)
		return &mNodes[lNodeIndex];
	else
		return nullptr;

}


//-----------------------------------------------------------------------------------------------------
bool BasicGrid::getNodesByRect(const RectF &lRect, std::vector<S32> &lList, bool lCanOverlap)
{
	if (!lRect.isValidRect())
		return false;

	// 1. Initial calculation using floor to ensure we get the correct starting cell
	S32 xB = static_cast<S32>(std::floor((lRect.getPoint().x - mArea.getPoint().x) / mSquareSize));
	S32 yB = static_cast<S32>(std::floor((lRect.getPoint().y - mArea.getPoint().y) / mSquareSize));

	S32 xE = static_cast<S32>(std::floor((lRect.getPoint().x + lRect.getExtent().x - mArea.getPoint().x) / mSquareSize));
	S32 yE = static_cast<S32>(std::floor((lRect.getPoint().y + lRect.getExtent().y - mArea.getPoint().y) / mSquareSize));

	// 2. Bounds handling
	if (lCanOverlap)
	{
		// Clamp to valid index range [0, mNodes-1]
		xB = std::clamp(xB, 0, mNodesX - 1);
		yB = std::clamp(yB, 0, mNodesY - 1);
		xE = std::clamp(xE, 0, mNodesX - 1);
		yE = std::clamp(yE, 0, mNodesY - 1);
	}
	else
	{
		// Strict check: if any part of the rect is outside, fail
		if (xB < 0 || yB < 0 || xE >= mNodesX || yE >= mNodesY)
			return false;
	}

	// 3. Populate list
	// Optimization: reserve space if the rect is large
	size_t estimatedSize = (xE - xB + 1) * (yE - yB + 1);
	lList.reserve(lList.size() + estimatedSize);

	for (S32 i = xB; i <= xE; i++)
	{
		for (S32 j = yB; j <= yE; j++)
		{
			lList.push_back(i + j * mNodesX);
		}
	}

	return true;
}
//-----------------------------------------------------------------------------------------------------
 /*
 Directions:
     1  2  3
	 4  X  5
	 6  7  8
 */

BasicGridNode* BasicGrid::getNeighbour(BasicGridNode* startNode, U8 direction, S32 &nodeIndex)
{
	// Get the current index of the startNode
	S32 startIdx = (startNode - mNodes); // Pointer arithmetic to get index
	S32 sX = startIdx % mNodesX;
	S32 sY = startIdx / mNodesX;

	S32 dX = 0, dY = 0;
	switch (direction) {
		case 1: dX = -1; dY = -1; break;
		case 2:          dY = -1; break;
		case 3: dX =  1; dY = -1; break;
		case 4: dX = -1;          break;
		case 5: dX =  1;          break;
		case 6: dX = -1; dY =  1; break;
		case 7:          dY =  1; break;
		case 8: dX =  1; dY =  1; break;
	}

	S32 nX = sX + dX;
	S32 nY = sY + dY;

	if (nX >= 0 && nX < mNodesX && nY >= 0 && nY < mNodesY) {
		nodeIndex = nX + nY * mNodesX;
		return &mNodes[nodeIndex];
	}

	nodeIndex = -1;
	return nullptr;
}
//-----------------------------------------------------------------------------------------------------
static S32 BINARYHEAP_COMPARE pathNodeFitnessCompare(const void* a, const void* b)
{
	// Use static_cast for better type safety and searchability
	const BasicGridNode* nodeA = static_cast<const BasicGridNode*>(a);
	const BasicGridNode* nodeB = static_cast<const BasicGridNode*>(b);

	F32 aCol = nodeA->mFitness;
	F32 bCol = nodeB->mFitness;

	if (aCol < bCol) return -1;
	if (aCol > bCol) return 1;
	return 0;
}

// Simple greedy path smoothing
void BasicGrid::smoothPath(std::vector<BasicGridNode*>& path) {
	if (path.size() < 3) return;

	for (size_t i = 0; i + 2 < path.size(); ) {
		BasicGridNode* startNode = path[i];
		BasicGridNode* endNode = path[i + 2];

		// If you can walk straight from 'i' to 'i+2', remove 'i+1'
		if (checkLineOfSight(startNode->getPos(), endNode->getPos())) {
			path.erase(path.begin() + i + 1);
			// Check again from the same 'i' with the new next node
		} else {
			++i;
		}
	}
}
//-----------------------------------------------------------------------------
// Basic implementation of line-of-sight
bool BasicGrid::checkLineOfSight(Point2F start, Point2F end) {
	// Simple version: check if any grid square between points is unwalkable (Flag 0)
	// For production, use a Bresenham's line algorithm or DDA
	F32 dist = start.dist(end);
	Point2F dir = (end - start) / dist;
	F32 stepSize = mSquareSize / 2.0f;

	for (F32 d = 0.0f; d < dist; d += stepSize) {
		Point2F checkPos = start + dir * d;
		BasicGridNode* node = findNode(checkPos.x, checkPos.y);
		if (!node || node->isFlagOn(0)) {
			return false; // Obstacle detected
		}
	}
	return true;
}
//-----------------------------------------------------------------------------
// Change return type to a vector of points
std::vector<Point2F> BasicGrid::getPath(Point2F start, Point2F end, bool smoothPath)
{
	std::vector<Point2F> path;
	std::vector<BasicGridNode*> replyList;

	BasicGridNode* startNode = findNode(start.x, start.y);
	BasicGridNode* goalNode = findNode(end.x, end.y);

	if (startNode && goalNode && generatePath(startNode, goalNode, replyList, smoothPath))
	{
		path.reserve(replyList.size());
		F32 halfSize = getSquareSize() / 2.0f;

		for (auto* node : replyList)
		{
			// Center the point in the grid square
			path.push_back({
				node->getPos().x + halfSize,
						   node->getPos().y + halfSize
			});
		}
	}
	return path;
}
//-----------------------------------------------------------------------------
S32 BasicGrid::getPathCosts(Point2F start, Point2F end)
{
	S32 startIndex, goalIndex;
	BasicGridNode* startNode = findNode(start.x, start.y, startIndex);
	BasicGridNode* goalNode = findNode(end.x, end.y, goalIndex);

	if (!startNode || !goalNode) return -1;
	if (startIndex == goalIndex) return 0;

	// Check if the node is unwalkable (Flag 0)
	if (startNode->isFlagOn(0) || goalNode->isFlagOn(0)) return -1;

	std::vector<BasicGridNode*> replyList;
	if (generatePath(startNode, goalNode, replyList, false))
	{
		return static_cast<S32>(replyList.size());
	}

	return -1;
}
//-----------------------------------------------------------------------------------------------------
// BasicGrid::CreateAllPairsCostsTable()
// precompile ALL possible pathes 
//-----------------------------------------------------------------------------------------------------
std::vector<std::vector<U32>> BasicGrid::CreateAllPairsCostsTable()
{
	U32 numNodes = getNodeCount();
	// Initialize matrix with a large value representing "infinity"
	// Use M_MAXS32/2 to prevent overflow during addition
	const U32 INF = 1000000;
	std::vector<std::vector<U32>> pathCosts(numNodes, std::vector<U32>(numNodes, INF));

	// Initialize direct edges from neighbors
	for (U32 i = 0; i < numNodes; ++i) {
		pathCosts[i][i] = 0; // Distance to self is zero

		// Check all 8 directions for direct neighbors
		for (U8 dir = 1; dir < 9; ++dir) {
			S32 neighborIdx;
			BasicGridNode* neighbor = getNeighbour(&mNodes[i], dir, neighborIdx);
			if (neighbor && neighbor->getPathWeight() != 255) {
				pathCosts[i][neighborIdx] = 1 + neighbor->getPathWeight();
			}
		}
	}

	// Floyd-Warshall Algorithm: 3 nested loops
	for (U32 k = 0; k < numNodes; ++k) {
		for (U32 i = 0; i < numNodes; ++i) {
			for (U32 j = 0; j < numNodes; ++j) {
				if (pathCosts[i][k] + pathCosts[k][j] < pathCosts[i][j]) {
					pathCosts[i][j] = pathCosts[i][k] + pathCosts[k][j];
				}
			}
		}
	}

	return pathCosts;
}
//-----------------------------------------------------------------------------------------------------
S32 BasicGrid::getNodeToNodeCosts(Point2F from, Point2F to)
{
	// Ensure table exists
	if (mPathCosts.empty()) {
		return -1;
	}

	S32 startNodeIdx = getNodeIndex(from.x, from.y);
	S32 endNodeIdx = getNodeIndex(to.x, to.y);

	// Bounds check for indices
	if (startNodeIdx < 0 || endNodeIdx < 0 ||
		startNodeIdx >= (S32)mPathCosts.size() ||
		endNodeIdx >= (S32)mPathCosts[startNodeIdx].size()) {
		return -1;
		}

		U32 cost = mPathCosts[startNodeIdx][endNodeIdx];

	// Check if path is unreachable (matches INF constant above)
	if (cost >= 1000000) {
		return -1;
	}

	return static_cast<S32>(cost);
}


//-----------------------------------------------------------------------------------------------------
// BasicGrid::generatePath create a path between 2 nodes =>  Astar
// need some tuning the node information are cleared after the path is generated
// so the must rebuild every time which is costy 

//-----------------------------------------------------------------------------------------------------
// bool BasicGrid::generatePath(BasicGridNode* startNode, BasicGridNode* goalNode, std::deque<BasicGridNode*> &replyList, const bool smoothPath)
bool BasicGrid::generatePath(BasicGridNode* startNode,
				  BasicGridNode* goalNode,
				  std::vector<BasicGridNode*> &replyList,
				  const bool smoothPath)
{
	
	//F32 lCostToGoal = 0.f;

	//ignore unwalkable or same node
	if (goalNode->getPathWeight() == 255 || startNode == goalNode)
		return false;

	// openList is a binary heap
	BinaryHeap<BasicGridNode*> openList(getNodeCount(), pathNodeFitnessCompare);

	// list of all nodes which were affected during this pathfinding
	std::vector<BasicGridNode*> affectedList;
	affectedList.clear();


	// add start node to open list
	startNode->mHeuristicCostToGoal = estimateCostToGoal(startNode, goalNode);
	startNode->mFitness = startNode->mLowestCostFromStart + startNode->mHeuristicCostToGoal;
	startNode->mOpen = true;
	openList.push(startNode);



	// keep searching while nodes in open list
	while (openList.size() > 0)
	{
		// first element is the lowest cost
		BasicGridNode* currentNode = openList.front();

		// remove the first element from the openList
		openList.pop();

		// add the current node to affected list
		affectedList.push_back(currentNode);

		// set it as closed
		currentNode->mOpen = false;
		currentNode->mClosed = true;

		// iterate over all its neighbours

  		for (U32 i = 1; i < 9; ++i)
		{
			S32 lNodeIndex;
			BasicGridNode* currentNeighbour = getNeighbour(currentNode,i, lNodeIndex);

			if (!currentNeighbour)
				continue;
			// add current neighbour to affected list
			affectedList.push_back(currentNeighbour);

			// its closed, or not walkable, ignore the neighbour
			if ( (currentNeighbour->mClosed)  || (currentNeighbour->getPathWeight() == 255) )
				continue;

			// if its not open, add it
			if (!currentNeighbour->mOpen)
			{
				// set this neighbours parent as the current node
				currentNeighbour->mParent = currentNode;

				// set the lowest cost, heuristic and fitness
				currentNeighbour->mLowestCostFromStart = currentNeighbour->mParent->mLowestCostFromStart + estimateCostToGoal(currentNeighbour, currentNeighbour->mParent);
				currentNeighbour->mHeuristicCostToGoal = estimateCostToGoal(currentNeighbour, goalNode);
				currentNeighbour->mFitness = currentNeighbour->mLowestCostFromStart + currentNeighbour->mHeuristicCostToGoal + currentNeighbour->getPathWeight();

				// set as open node
				currentNeighbour->mOpen = true;

				// add to open list
				openList.push(currentNeighbour);
			} else
			{
				// see neighbour already has the current node as its parent
				if (currentNeighbour->mParent != currentNode)
				{
					// see if this neighbour is a quicker path
					F32 costFromThisNode = currentNode->mLowestCostFromStart 
						                   + estimateCostToGoal(currentNeighbour, currentNode) + currentNeighbour->getPathWeight();
					if (currentNeighbour->mLowestCostFromStart > costFromThisNode)
					{
						// neighbour is better
						currentNeighbour->mLowestCostFromStart = costFromThisNode;
						currentNeighbour->mParent = currentNode;
					}
				}

				// close the node
				currentNeighbour->mClosed = false;
			}

			// see if we have reached the end yet
			if (currentNeighbour == goalNode)
			{
				// go back over all the nodes parents and construct the path
				BasicGridNode* currentTraceNode = goalNode;
				while (currentTraceNode) {
					replyList.push_back(currentTraceNode); // O(1)
					currentTraceNode = currentTraceNode->mParent;
				}

				// Reverse to get Start -> Goal order
				std::reverse(replyList.begin(), replyList.end());

				// smooth the path
				if (smoothPath) {
					this->smoothPath(replyList);
				}

				// reset the affected node pathfinding variables
				this->resetNodeVariables(affectedList);
				affectedList.clear();

				// found a path - return happy
				// Con::errorf("BASIC GRID TEST COSTS: %5.3f PATHLEN:%d", lCostToGoal, replyList.size());
				
				return true;
			}
		}
	}

	// reset the affected node pathfinding variables
	this->resetNodeVariables(affectedList);
	affectedList.clear();

	// couldn't find a path!
	return false;
}
//-----------------------------------------------------------------------------
void BasicGrid::resetNodeVariables(std::vector<BasicGridNode*> &affectedList)
{
	// iterate over the affected list and reset the path finding variables
	for (U32 i = 0; i < affectedList.size(); i++)
	{
		affectedList[i]->mFitness = 0.0f;
		affectedList[i]->mLowestCostFromStart = 0.0f;
		affectedList[i]->mHeuristicCostToGoal = 0.0f;
		affectedList[i]->mParent = 0;
		affectedList[i]->mOpen = false;
		affectedList[i]->mClosed = false;
	}
}
//-----------------------------------------------------------------------------
F32 BasicGrid::estimateCostToGoal(BasicGridNode* from, BasicGridNode* goal)
{
	return (goal->getPos() - from->getPos()).len();
}
