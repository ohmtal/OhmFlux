// FIXME DEBUG RENDER / Tilemap like rendering

#include "Grid.h"
#include "console/engineAPI.h"
#include "core/Globals.h"
namespace ElfFlux {

IMPLEMENT_CONOBJECT(Grid);

// void Grid::Update(const double& dt) {
//     if (!mGrid.isInitialized()) return;
// }
//
// void Grid::Draw()
// {
//     //FIXME debug render
//     if (!mGrid.isInitialized()) return;
// }

bool Grid::onAdd()
{
    if (!Parent::onAdd()) return false;
    return true;
}
void Grid::onRemove()
{
    Parent::onRemove();
}

SimObject * Grid::createPath(Point2F start, Point2F end, const bool smoothPath )
{
    if (!mGrid.isInitialized()) return nullptr;
    bool result = false;
    std::vector<BasicGridNode*> replyList;

    BasicGridNode* startNode = mGrid.findNode(start.x, start.y);
    BasicGridNode* goalNode = mGrid.findNode(end.x, end.y);
    if (startNode && goalNode) {
        result = mGrid.generatePath(startNode, goalNode, replyList, smoothPath);
    }

    if (result)
    {

        SimObject * pathObject = new SimObject();
        F32 halfSquareSize = mGrid.getHalfSquareSize();

        for (U32 i = 0; i < replyList.size(); i++)
        {
            char nbuf[64];dMemset(nbuf,0,64);
            dSprintf(nbuf,20,"node%d",i);
            const char *fieldName = StringTable->insert(nbuf);


            //XXTH 2021-04-24 / OGE3D 2024-01-04 changed to center and Point2I
            dSprintf(nbuf, 64, "%d %d",
                     (U32)(replyList[i]->getPos().x + halfSquareSize),
                     (U32)(replyList[i]->getPos().y + halfSquareSize)
            );
            /*
             *	dSprintf(nbuf,64,"%f %f %f",
             *          replyList[i]->getPos().x,
             *          replyList[i]->getPos().y,
             *          replyList[i]->getPos().z);
             */
            pathObject->setDataField( fieldName, NULL, nbuf );

        }
        return pathObject;
    }

    return NULL;
}
// ----------------------------------------------------------------
// ------------------------ Console -------------------------------
// ----------------------------------------------------------------
DefineEngineMethod(Grid, init, bool, (RectI area, F32 squareSize), , "param: area: x y w h, F32 SquareSize")
{
    if (!area.isValidRect())
        return false;
    object->mGrid.init(area, squareSize);

    return true;
}


DefineEngineMethod(Grid, getNodeCount, S32, (), , "get count of nodes")
{
    return object->mGrid.getNodeCount();
}

DefineEngineMethod(Grid, getNodeCountX, S32, (), , "get count of horizontal nodes ")
{
    return object->mGrid.getNodesX();
}
DefineEngineMethod(Grid, getNodeCountY, S32, (), , "get count of vertical nodes ")
{
    return object->mGrid.getNodesY();
}


DefineEngineMethod(Grid, getPos, Point2F, (F32 x, F32 y), , "x,y; return top left pos of a node by the world values ")
{
    BasicGridNode *lNode = object->mGrid.findNode(x,y);
    Point2F lResult = {0.f, 0.f};

    if (lNode)
    {
        lResult.x = (U32)lNode->getPos().x;
        lResult.y = (U32)lNode->getPos().y;
    }
    return lResult;
}

DefineEngineMethod(Grid, getFlags, S32, (F32 x, F32 y), , "x,y; return flags ") {

    BasicGridNode* lNode = object->mGrid.findNode(x, y);
    U32 lResult = 0;

    if (lNode)
    {
        lResult = (U32)lNode->getFlags();
    }

    return lResult;
}

DefineEngineMethod(Grid, getNodeByPos, const char*, (F32 x, F32 y), , "x,y; return nodeidx x y z flags ")
{
    char* rbuf = Con::getReturnBuffer(256);
    BasicGridNode *lNode;
    S32 lNodeIndex = object->mGrid.getNodeIndex(x,y);
    if (lNodeIndex >= 0)
        lNode = object->mGrid.getNodeById(lNodeIndex);
    else
        return "";

    // getNodeIndex
    if (lNode)
    {
        dSprintf(rbuf, 256, "%d %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y,  (U32)lNode->getFlags());
        return rbuf;
    }

    return "";
}

DefineEngineMethod(Grid, getNodeIdByPos, S32, (F32 x, F32 y), ,  "x,y; return S32 nodeidx ")
{
    BasicGridNode* lNode;
    S32 lNodeIndex = object->mGrid.getNodeIndex(x,y);
    if (lNodeIndex >= 0)
        lNode = object->mGrid.getNodeById(lNodeIndex);
    else
        return -1;

    // getNodeIndex
    if (lNode)
    {
        return  lNodeIndex;
    }

    return -1;
}

DefineEngineMethod(Grid, getNode, const char*, (S32 nodeIndex), , "S32 NodeIndex,  return nodeidx x y z flags ")
{
    char* rbuf = Con::getReturnBuffer(256);
    BasicGridNode *lNode;
    S32 lNodeIndex = nodeIndex;
    if (lNodeIndex >= 0)
        lNode = object->mGrid.getNodeById(lNodeIndex);
    else
        return "";

    // getNodeIndex
    if (lNode)
    {
        dSprintf(rbuf, 256, "%d %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y, (U32)lNode->getFlags());
        return rbuf;
    }

    return "";
}

DefineEngineMethod(Grid, getNodesByRect, const char*, (RectF area), , "x y w h,  return nodeidx nodeidx .. ")
{
    std::vector<S32> lVisRadiusList;

    object->mGrid.getNodesByRect(area,lVisRadiusList,true);

    if (lVisRadiusList.size() == 0)
        return "";

    char* rbuf = Con::getReturnBuffer(1024);
    for (S32 i = 0; i < lVisRadiusList.size() ; i++)
    {
        if (i == 0)
            dSprintf(rbuf, 1024, "%d",lVisRadiusList[i]);
        else
            dSprintf(rbuf, 1024, "%s %d", rbuf,lVisRadiusList[i]);
    }

    return rbuf;

}

DefineEngineMethod(Grid, getNeighbour, const char*, (S32 nodeIndex, S32 direction), , "S32 NodeIndex, S32 Direction,  return nodeidx x y z flags "
"Directions:"
"1  2  3"
"4  X  5"
"6  7  8"
)
{
    char* rbuf = Con::getReturnBuffer(256);
    BasicGridNode* lstartNode;
    BasicGridNode* lNode;
    S32 lNodeIndex = 0;
    S32 lStartNodeIndex = nodeIndex;
    S32 lDirection = direction;
    if (lNodeIndex >= 0 && (lDirection > 0 && lDirection < 9))
    {
        lstartNode = object->mGrid.getNodeById(lStartNodeIndex);
        lNode = object->mGrid.getNeighbour(lstartNode, lDirection, lNodeIndex);
    }
    else
        return "";

    // getNodeIndex
    if (lNode)
    {
        dSprintf(rbuf, 256, "%d %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y,  (U32)lNode->getFlags());
        return rbuf;
    }

    return "";
}

DefineEngineMethod(Grid, setFlags, bool, (F32 x, F32 y, U32 flags), , "x,y; set flags ")
{
    BasicGridNode *lNode = object->mGrid.findNode(x,y);

    if (lNode)
    {
        //done at the node object->mGrid.setDirty();
        lNode->setFlags(flags);
        return true;
    }

    return false;
}

DefineEngineMethod(Grid, setIntValue, bool, (F32 x, F32 y, S32 idx, S32 value), , "x,y, idx[0..9], Value; set flags ")
{
    if (idx > 9 || idx < 0)
        return false;
    BasicGridNode *lNode = object->mGrid.findNode(x,y);

    if (lNode)
    {
        //done at the node ..not atm but when used it will ...object->mGrid.setDirty();
        lNode->setIntValue(idx,value);
        return true;
    }

    return false;
}

DefineEngineMethod(Grid, setWeight, bool, (F32 x, F32 y, S32 weigth), , "x,y, U8 weight")
{
    BasicGridNode *lNode = object->mGrid.findNode(x,y);

    if (lNode)
    {
        //done at the node ..not atm but when used it will ...object->mGrid.setDirty();
        lNode->setWeight(weigth);
        return true;
    }

    return false;
}

DefineEngineMethod(Grid, getIntValue,S32, (F32 x,F32 y,S32 idx), , "x,y, idx[0..9]")
{
    if (idx > 9 || idx < 0)
        return 0;
    BasicGridNode *lNode = object->mGrid.findNode(x,y);

    if (lNode)
    {
        return lNode->getIntValue(idx);

    }

    return 0;
}

DefineEngineMethod(Grid, setIntValueByNodeId, bool, (S32 nodeId, S32 idx, S32 value), , "x,y, U8 value")
{
    if (idx > 9)
        return false;
    BasicGridNode* lNode = object->mGrid.getNodeById(nodeId);

    if (lNode)
    {
        //done at the node ..not atm but when used it will ...object->mGrid.setDirty();
        lNode->setIntValue(idx, value);
        return true;
    }

    return false;
}

DefineEngineMethod(Grid, getIntValueByNodeId, S32, (S32 nodeId, S32 idx), , "nodeId, idx[0..9]")
{
    if (idx > 9)
        return 0;
    BasicGridNode* lNode = object->mGrid.getNodeById(nodeId);

    if (lNode)
    {
        return lNode->getIntValue(idx);

    }

    return 0;
}

DefineEngineMethod(Grid, getinfo, void, (bool listNodes), (false), "Display Infos on Console")
{

    Con::printf("Grid - id:%d, Area: %d,%d %d,%d NodeCount:%d SquareSize:%f",
                object->getId(),
                object->mGrid.getArea().x, object->mGrid.getArea().y,
                object->mGrid.getArea().w, object->mGrid.getArea().h,
                object->mGrid.getNodeCount(),
                object->mGrid.getSquareSize());


    if (listNodes)
    {
        BasicGridNode* curNode;
        for (U32 i = 0; i < object->mGrid.getNodeCount(); ++i)
        {
            curNode = object->mGrid.getNodeById(i);
            Con::printf("node %d at %f,%f", i,
                        curNode->getPos().x, curNode->getPos().y
            );
        }
    }

}

DefineEngineMethod(Grid, findPath, S32, (Point2F start, Point2F goal, bool smoothPath), ( false)
, "findPath (Point2F start, Point2F goal, bool smoothPath = false) - Create a path between the two points."
  "Warning smoothPath use line of sight and ignore the weight !!!!!"
)
{

    SimObject * result = object->createPath(start,goal, smoothPath);
    if (result)
    {
        result->registerObject();
        return result->getId();
    }

    return 0;
}



DefineEngineMethod(Grid, getWeightByNodeId, S32, (S32 nodeId), , "")
{
    BasicGridNode* lNode = object->mGrid.getNodeById(nodeId);
    if (lNode)
    {
        return lNode->getWeight();

    }
    return -1;
}


DefineEngineMethod(Grid, setWeightByNodeId, bool, (S32 nodeId, S32 weight), , "nodeId, U8 weight")
{
    BasicGridNode* lNode = object->mGrid.getNodeById(nodeId);
    if (lNode)
    {
        lNode->setWeight(weight);
        return true;
    }
    return false;
}


// DefineEngineMethod(Grid, getNodeCenter, Point2I, (F32 x, F32 y), , "x,y; return centerPos Point2I of a node by the world values (see also getCenterPos)")
// {
//     BasicGridNode* lNode = object->mGrid.findNode(x, y);
//
//     Point2I lResult = Point2I(0, 0);
//
//     if (lNode)
//     {
//         Point3F lCenterPos = lNode->getCenterPos();
//         lResult.x = (U32)lCenterPos.x;
//         lResult.y = (U32)lCenterPos.y;
//     }
//
//     return lResult;
// }


// DefineEngineMethod(Grid, getNodeCenterbyId, Point2I, (S32 nodeId), , "return centered Point2I pos  ")
// {
//     BasicGridNode* lNode = object->mGrid.getNodeById(nodeId);
//     if (lNode)
//     {
//         return lNode->getPos2I();
//     }
//     return Point2I(0,0);
// }


DefineEngineMethod(Grid, getNodeRectbyId, RectI, (S32 nodeId), , "return centered rectI pos / extent ")
{
    BasicGridNode* lNode = object->mGrid.getNodeById(nodeId);
    if (lNode)
    {
        return RectI(
            (U32)(lNode->getPos().x),
                     (U32)(lNode->getPos().y),
                     (U32)(object->mGrid.getSquareSize()),
                     (U32)(object->mGrid.getSquareSize())
        );
    }

    return RectI(0,0,0,0);
}

// ----------------------------------------------------------------------------
DefineEngineMethod(Grid, getPathCost, S32, (Point2F start, Point2F goal, bool smoothPath), ( true)
, "getPathCost (Point2F start, Point2F goal) - Create a path between the two points and Return the lenth of the path")
{
    return object->mGrid.getPathCosts(start, goal);
}
//------------------------------------------------------------------------------------------------------------------
DefineEngineMethod(Grid, compilePathCosts, void,() , , "Fill the table with all path costs - expensive!")
{
    object->mGrid.mPathCosts = object->mGrid.CreateAllPairsCostsTable();
}

DefineEngineMethod(Grid, getNodeToNodeCosts, S32, (Point2F start, Point2F goal, bool smoothPath), ( true)
, "(param Point2F from,  Point2F to;	return F32 distance)"
  "return nodecount of to points to calculated closed path it -1 then it failed") {
    return object->mGrid.getNodeToNodeCosts(start, goal);
}


} //namespace
