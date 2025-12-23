//-----------------------------------------------------------------------------
// Copyright (c) 2009 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// BasicGrid
//-----------------------------------------------------------------------------

#pragma once
#ifndef _BASICGRID_H_
#define _BASICGRID_H_

#include "fluxGlobals.h"
#include <vector>
#include <cassert>

//-------------------------------------------

const U32 BASIC_GRID_NODE_INTVALUES_COUNT = 10;

class BasicGridNode
{
private:
   Point2F mPos;
   // Flag 0 = unwalkable!
   U32  mFlags;
   U8  mWeight; 
   S32 mIntValues[BASIC_GRID_NODE_INTVALUES_COUNT];

public:
	//pathfinding stuff:
	bool mOpen;
	bool mClosed;
	F32 mFitness;
	F32 mLowestCostFromStart;
	F32 mHeuristicCostToGoal;
	BasicGridNode* mParent;


public:
   void setPos(Point2F lPos) { mPos = lPos; }
   Point2F getPos() { return mPos;}
   Point2F getPos2F() { return { mPos.x, mPos.y };}

   void setIntValue(S32 idx,S32 lValue)
   {
       assert(idx >= 0);
	   if ( idx < BASIC_GRID_NODE_INTVALUES_COUNT )
           mIntValues[idx] = lValue;
   }
   S32 getIntValue(S32 idx)
   {
      assert(idx >= 0);
	   if ( idx < BASIC_GRID_NODE_INTVALUES_COUNT )
		   return mIntValues[idx]; 
	   else 
		   return 0; 
   }

   void setWeight (S32 lValue) { mWeight = (U8)lValue; }
   S32 getWeight () { return (S32)mWeight; }

   void setFlags (U32 lFlags) { mFlags = lFlags; }
   U32 getFlags () { return mFlags; }

   bool isFlagOn(U8 lFlag)  { 
	   return (mFlags & BIT(lFlag)) == BIT(lFlag); 
   }
   void toggleFlag(U8 lFlag) { 
	   if (!isFlagOn(lFlag)) 
		   mFlags |= BIT(lFlag); 
	   else
		   mFlags ^= BIT(lFlag); 
   }

   void addFlag(U8 lFlag) { 
	   if (!isFlagOn(lFlag)) 
		   mFlags |= BIT(lFlag); 
   }
   void rmvFlag(U8 lFlag) { 
	   if (isFlagOn(lFlag))  
			mFlags ^= BIT(lFlag); 
   }



   U8 getPathWeight()
   {
	   if (isFlagOn(0))
		   return 255;
		return mWeight;
   }



   BasicGridNode() { 
	   mPos={ 0.f,0.f };
	   mFlags=0; 

	   mOpen    = false; 
	   mClosed  = false;
	   mFitness = 0.f;
	   mWeight = 0;
	   mLowestCostFromStart = 0.f;
	   mHeuristicCostToGoal = 0.f;
	   mParent = nullptr;
	   for (S32 i=0;i<10; i++)
		   mIntValues[i]=0; 
   }

   ~BasicGridNode() {
   }
};


class BasicGrid
{

public:
	BasicGrid();
	~BasicGrid();

private:
	F32 mSquareSize;

protected:
	RectI mArea;
	bool mInitDone;
	bool mDebugGrid;
    bool mClientGame;

	S32 mNodesX,mNodesY, mNodeCount;
	BasicGridNode* mNodes;
	

	void resetNodeVariables(std::vector<BasicGridNode*> &affectedList);
	F32 estimateCostToGoal(BasicGridNode* from, BasicGridNode* goal);


public:

   bool generatePath(BasicGridNode* startNode,
                     BasicGridNode* goalNode,
                     std::vector<BasicGridNode*> &replyList,
                     const bool smoothPath);

   void smoothPath(std::vector<BasicGridNode*>& path);
   bool checkLineOfSight(Point2F start, Point2F end);
   std::vector<Point2F> getPath(Point2F start, Point2F end, bool smoothPath);

   S32 getPathCosts(Point2F start, Point2F end);

   //this will hold a pre-calculated lookup table of the cost to travel from
	//one node to any other.

   std::vector<std::vector<U32> >  mPathCosts;
   std::vector<std::vector<U32> >  CreateAllPairsCostsTable();
   S32 getNodeToNodeCosts(Point2F from, Point2F to);


   S32  getNodeIndex(F32 x, F32 y, bool lPrec = false); //lPrec = more precise by float params, but slower
   BasicGridNode* getNeighbour(BasicGridNode* startNode, U8 direction, S32 &nodeIndex);

   bool getNodesByRect(const RectF &lRect, std::vector<S32> &lList, bool lCanOverlap = false);

   bool getClientGame() { return mClientGame; }
   void setClientGame(bool lValue) { mClientGame= lValue; }

   F32 getSquareSize() { return mSquareSize; }
   F32 getHalfSquareSize() { return mSquareSize / 2.f; }

   RectI getArea() { return mArea; }

   bool getDebugGrid() { return mDebugGrid; }
   void setDebugGrid(bool lValue) { mDebugGrid = lValue; }
   BasicGridNode* findNode(F32 x, F32 y);
   BasicGridNode* findNode(F32 x, F32 y, S32 &nodeIndex);

   BasicGridNode* getNodeById(U32 lId) { 
	   if (lId >=0 && lId < getNodeCount())
			return &mNodes[lId]; 
	   else
		   return nullptr;
   }

   U32 getNodeCount() { return mNodeCount; }

   S32 getNodesX() const { return mNodesX; }
   S32 getNodesY() const { return mNodesY; }

   virtual void init(RectI lArea, F32 lSquareSize = 8);
   


   
};

#endif //_BASICGRID_H_
