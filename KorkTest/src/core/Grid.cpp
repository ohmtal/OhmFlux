// FIXME DEBUG RENDER
// TODO TEST consoleFunctions
#include "Grid.h"
#include <platform/platformString.h>
#include "core/Globals.h"
namespace KorkFlux {

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

        F32 halfSquareSize = mGrid.getSquareSize() / 2;

        for (U32 i = 0; i < replyList.size(); i++)
        {
            char nbuf[64];memset(nbuf,0,64);
            dSprintf(nbuf,20,"node%d",i);
            const char *fieldName = StringTable->insert(nbuf);

            dSprintf(nbuf, 64, "%d %d",
                     (U32)(replyList[i]->getPos().x + halfSquareSize),
                     (U32)(replyList[i]->getPos().y + halfSquareSize)
            );

            pathObject->setDataField( fieldName, NULL, nbuf );

        }
        return pathObject;
    }
    return NULL;
}

// ------------------------ Console -------------------------------

ConsoleMethod(Grid,init, ConsoleBool, 4, 4, "param: area: x y w h, F32 SquareSize")
{
    F32 lSquareSize;
    RectI area;

    dSscanf(argv[2], "%d %d %d %d", &area.x, &area.y, &area.w, &area.h);
    lSquareSize = dAtof(argv[3]);

    if (!area.isValidRect())
        return false;

    object->mGrid.init(area,lSquareSize);

    return true;
}


ConsoleMethod(Grid,getNodeCount, ConsoleInt, 2, 2, "get count of nodes")
{

    return object->mGrid.getNodeCount();
}



ConsoleMethod(Grid, getPos, ConsoleString, 4, 4, "x,y; return pos ")
{


    BasicGridNode *lNode = object->mGrid.findNode(dAtof(argv[2]),dAtof(argv[3]));

    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d %d", (U32)lNode->getPos().x,(U32)lNode->getPos().y);
        return getReturnString(rbuf,vmPtr);
    }

    return "";
}

ConsoleMethod(Grid, getNodeCenter, ConsoleString, 4, 4, "x,y; return centered Point2I pos ")
{

    BasicGridNode* lNode = object->mGrid.findNode(dAtof(argv[2]), dAtof(argv[3]));

    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d %d",
                 (U32)(lNode->getPos().x + object->mGrid.getSquareSize() / 2),
                 (U32)(lNode->getPos().y + object->mGrid.getSquareSize() / 2)
        );
        return getReturnString(rbuf, vmPtr);
    }

    return "";
}


ConsoleMethod(Grid, getNodeCenterbyId, ConsoleString, 3, 3, "return centered Point2I pos ")
{


    BasicGridNode* lNode = object->mGrid.getNodeById(dAtoi(argv[2]));
    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d %d",
                 (U32)(lNode->getPos().x + object->mGrid.getSquareSize() / 2),
                 (U32)(lNode->getPos().y + object->mGrid.getSquareSize() / 2)
        );
        return getReturnString(rbuf, vmPtr);
    }

    return "";
}

ConsoleMethod(Grid, getNodeRectbyId, ConsoleString, 3, 3, "return centered rectI pos / extent ")
{
    BasicGridNode* lNode = object->mGrid.getNodeById(dAtoi(argv[2]));
    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d %d %d %d",
                 (U32)(lNode->getPos().x),
                 (U32)(lNode->getPos().y),
                 (U32)(object->mGrid.getSquareSize()),
                 (U32)(object->mGrid.getSquareSize())
        );
        return getReturnString(rbuf,vmPtr);
    }

    return "";
}



ConsoleMethod(Grid, getFlags, ConsoleString, 4, 4, "x,y; return flags ")
{
    BasicGridNode *lNode = object->mGrid.findNode(dAtof(argv[2]),dAtof(argv[3]));

    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d", (U32)lNode->getFlags());
        return getReturnString(rbuf,vmPtr);
    }

    return "";
}

ConsoleMethod(Grid, getNodeByPos, ConsoleString, 4,4, "x,y; return nodeidx x y flags ")
{

    BasicGridNode *lNode;
    S32 lNodeIndex = object->mGrid.getNodeIndex(dAtof(argv[2]),dAtof(argv[3]), true);
    if (lNodeIndex >= 0)
        lNode = object->mGrid.getNodeById(lNodeIndex);
    else
        return "";

    // getNodeIndex
    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y, (U32)lNode->getFlags());
        return getReturnString(rbuf,vmPtr);
    }

    return "";
}

ConsoleMethod(Grid, getNodeIdByPos, ConsoleInt, 4, 4, "x,y; return S32 nodeidx ")
{
    BasicGridNode* lNode;
    S32 lNodeIndex = object->mGrid.getNodeIndex(dAtof(argv[2]), dAtof(argv[3]), true);
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



ConsoleMethod(Grid, getNode, ConsoleString, 3,3, "S32 NodeIndex,  return nodeidx x y flags ")
{
    BasicGridNode *lNode;
    S32 lNodeIndex = dAtof(argv[2]);
    if (lNodeIndex >= 0)
        lNode = object->mGrid.getNodeById(lNodeIndex);
    else
        return "";

    // getNodeIndex
    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d %f %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y, (U32)lNode->getFlags());
        return getReturnString(rbuf,vmPtr);

    }

    return "";
}

//BasicGridNode * BasicGrid::getNeighbour(BasicGridNode * startNode, U8 direction)

ConsoleMethod(Grid, getNeighbour, ConsoleString, 4, 4, "S32 NodeIndex, S32 Direction,  return nodeidx x y z flags "
"Directions:"
"1  2  3"
"4  X  5"
"6  7  8"
)
{
    BasicGridNode* lstartNode;
    BasicGridNode* lNode;
    S32 lNodeIndex = 0;
    S32 lStartNodeIndex = dAtof(argv[2]);
    S32 lDirection = dAtof(argv[3]);
    if (lNodeIndex >= 0 && (lDirection>0 && lDirection < 9))
    {
        lstartNode = object->mGrid.getNodeById(lStartNodeIndex);
        lNode = object->mGrid.getNeighbour(lstartNode, lDirection, lNodeIndex);
    }
    else
        return "";

    // getNodeIndex
    if (lNode)
    {
        char rbuf[256] = {0};
        dSprintf(rbuf, 256, "%d %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y, (U32)lNode->getFlags());
        return getReturnString(rbuf,vmPtr);

    }

    return "";
}


ConsoleMethod(Grid, getNodesByRect ,ConsoleString, 3,3, "x y w h,  return nodeidx nodeidx .. ")
{
    RectF area;

    //
    if(argc == 3)
        dSscanf(argv[2], "%f %f %f %f", &area.x, &area.y, &area.w, &area.h);
    else if(argc == 6)
    {
        area.x = dAtof(argv[2]);
        area.y = dAtof(argv[3]);
        area.w = dAtof(argv[4]);
        area.h = dAtof(argv[5]);
    }
    std::vector<S32> lVisRadiusList;

    object->mGrid.getNodesByRect(area,lVisRadiusList,true);


    if (lVisRadiusList.size() == 0)
        return "";

    char rbuf[1024] = {0};
    for (S32 i = 0; i < lVisRadiusList.size() ; i++)
    {
        if (i == 0)
            dSprintf(rbuf, 1024, "%d",lVisRadiusList[i]);
        else
            dSprintf(rbuf, 1024, "%s %d", rbuf,lVisRadiusList[i]);
    }
    return getReturnString(rbuf,vmPtr);
}




ConsoleMethod(Grid, setFlags,ConsoleBool, 5, 5, "x,y; set flags ")
{
    BasicGridNode *lNode = object->mGrid.findNode(dAtof(argv[2]),dAtof(argv[3]));

    if (lNode)
    {
        lNode->setFlags(dAtoi(argv[4]));
        return true;
    }

    return false;
}

ConsoleMethod(Grid, setIntValue,ConsoleBool, 6, 6, "x,y, idx[0..9], Value; set flags ")
{
    S32 idx = dAtoi(argv[4]);
    if (idx > 9)
        return false;
    BasicGridNode *lNode = object->mGrid.findNode(dAtof(argv[2]),dAtof(argv[3]));

    if (lNode)
    {
        lNode->setIntValue(idx,dAtoi(argv[5]));
        return true;
    }

    return false;
}

ConsoleMethod(Grid, setWeight,ConsoleBool, 5, 5, "x,y, U8 weight")
{
    BasicGridNode *lNode = object->mGrid.findNode(dAtof(argv[2]),dAtof(argv[3]));

    if (lNode)
    {
        lNode->setWeight(dAtoi(argv[4]));
        return true;
    }

    return false;
}


ConsoleMethod(Grid, getWeightByNodeId, ConsoleInt, 3, 3, "nodeId")
{

    BasicGridNode* lNode = object->mGrid.getNodeById(dAtoi(argv[2]));

    if (lNode)
    {
        return lNode->getWeight();

    }

    return -1;
}

ConsoleMethod(Grid, setWeightByNodeId, ConsoleBool, 4,4, "nodeId, U8 weight")
{

    BasicGridNode* lNode = object->mGrid.getNodeById(dAtoi(argv[2]));

    if (lNode)
    {
        lNode->setWeight(dAtoi(argv[3]));
        return true;
    }

    return false;
}


ConsoleMethod(Grid, getIntValue,ConsoleInt, 5, 5, "x,y, idx[0..9]")
{
    S32 idx = dAtoi(argv[4]);
    if (idx > 9)
        return 0;
    BasicGridNode *lNode = object->mGrid.findNode(dAtof(argv[2]),dAtof(argv[3]));

    if (lNode)
    {
        return lNode->getIntValue(idx);

    }

    return 0;
}

ConsoleMethod(Grid, setIntValueByNodeId, ConsoleBool, 5, 5, "nodeId, idx[0..9], S32 Value; store an integer value on a grid block ")
{
    S32 idx = dAtoi(argv[3]);
    if (idx > 9)
        return false;
    BasicGridNode* lNode = object->mGrid.getNodeById(dAtoi(argv[2]));

    if (lNode)
    {
        lNode->setIntValue(idx, dAtoi(argv[4]));
        return true;
    }

    return false;
}

ConsoleMethod(Grid, getIntValueByNodeId, ConsoleInt, 4, 4, "nodeId, idx[0..9]")
{
    S32 idx = dAtoi(argv[3]);
    if (idx > 9)
        return 0;
    BasicGridNode* lNode = object->mGrid.getNodeById(dAtoi(argv[2]));

    if (lNode)
    {
        return lNode->getIntValue(idx);

    }

    return 0;
}





ConsoleMethod(Grid,getinfo,ConsoleVoid,2,2,"Display Infos on Console")
{
    Con::printf("BASIC Grid - id:%d, Area: %d,%d %d,%d NodeCount:%d SquareSize:%f",
                object->getId(),
                object->mGrid.getArea().x, object->mGrid.getArea().y,
                object->mGrid.getArea().w, object->mGrid.getArea().h,
                object->mGrid.getNodeCount(),
                object->mGrid.getSquareSize());

}



ConsoleMethod(Grid,findPath,ConsoleInt,4,5,"findPath (Point2F start, Point2F goal, bool smoothPath = true) - Create a path between the two points and Return the ID of path")
{


    if ((dStrlen(argv[2]) != 0) && (dStrlen(argv[3]) != 0))
    {
        Point2F start;
        Point2F goal;
        bool smooth = false;
        dSscanf(argv[2], "%f %f", &start.x, &start.y);
        dSscanf(argv[3], "%f %f", &goal.x, &goal.y);
        if (argc>5)
            smooth = dAtob(argv[4]);
        SimObject * result = object->createPath(start,goal,smooth);
        if (result)
        {
            result->registerObject();
            return result->getId();
        } else
            return 0;

    } else {
        return 0;
    }
}

ConsoleMethod(Grid, getPathCost, ConsoleInt, 4, 5, "getPathCost (Point2F start, Point2F goal) - Create a path between the two points and Return the lenth of the path")
{


    if ((dStrlen(argv[2]) != 0) && (dStrlen(argv[3]) != 0))
    {
        Point2F start;
        Point2F goal;
        bool smooth = false;
        dSscanf(argv[2], "%f %f", &start.x, &start.y);
        dSscanf(argv[3], "%f %f", &goal.x, &goal.y);
        if (argc > 5)
            smooth = dAtob(argv[4]);
        S32 result = object->mGrid.getPathCosts(start, goal);
        if (result)
        {
            return result;
        }
        else
            return 0;

    }
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------------------------------------------

ConsoleMethod(Grid, compilePathCosts, ConsoleVoid, 2, 2, "Fill the table with all path costs - expensive!")
{
    object->mGrid.mPathCosts = object->mGrid.CreateAllPairsCostsTable();
}

ConsoleMethod(Grid, getNodeToNodeCosts, ConsoleInt, 4, 4, "(param Point2F from,  Point2F to;	return F32 distance)"
"return nodecount of to points to calculated closed path it -1 then it failed")
{

    Point2F lFrom;
    Point2F lTo;
    dSscanf(argv[2], "%g %g", &lFrom.x, &lFrom.y);
    dSscanf(argv[2], "%g %g", &lTo.x, &lTo.y);
    S32 result = object->mGrid.getNodeToNodeCosts(lFrom, lTo);

    return result;
}




}
