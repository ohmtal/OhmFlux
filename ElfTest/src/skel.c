// --------- Torque Kork Script Test (https://github.com/jamesu/korkscript)
#include "platform/platform.h"
#include "console/console.h"
#include "sim/simBase.h"
#include "sim/dynamicTypes.h"
#include "core/fileStream.h"

class Player : public SimObject
{
    typedef SimObject Parent;

public:

    Vector2 mPosition;

    Player()
    {
        mPosition = {};
    }

    static void initPersistFields()
    {
        Parent::initPersistFields();
        // Does not have the default types like TypeS32 :(
        // addField("position", TypeReqUInt, Offset(mPosition, Player));
    }

    DECLARE_CONOBJECT(Player);
};

IMPLEMENT_CONOBJECT(Player);

ConsoleMethod(Player, jump, void, 2, 2, "")
{
    Log("[info] Player jump :D");
}

//------------------------------------------------------------------------------
void MyLogger(U32 level, const char *consoleLine, void*)
{
    Log("[%d] %s", level, consoleLine);
}


    // korkscript >>>
    Con::init();
    Sim::init();
    Con::addConsumer(MyLogger, NULL);
    Con::evaluatef("echo(\"Testing kork script... ...\");");
    // <<<<


