//-----------------------------------------------------------------------------
// Ohmtal Game Engine

//-----------------------------------------------------------------------------
/**
 * Box2D Test 
 * @author T.Huehn
 * @since  2021-03-18
 * 2026-05-30 ported to ElfFlux

 $testWorld = new TestWorld(){x=300; y=300;};

 $testWorld.delete();
 */

#include "b2btest.h"

#include "console/engineAPI.h"
#include "appMain.h"
#include "Box2D/Box2D.h"

namespace b2bTest
{
    IMPLEMENT_CONOBJECT(TestWorld);

    // ------------------------------------------------------------------------.
    bool TestWorld::onAdd(){
        if (!ElfFlux::gMain)  return false;

        // ----------------------------
        //Creating a World
        mGravity = b2Vec2(0.f, mGroundGravity);
        mWorld = new b2World(mGravity);
        // Create edge around entire screen
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(0, 0);

        b2Body* groundBody = mWorld->CreateBody(&groundBodyDef);
        b2EdgeShape groundEdge;
        b2FixtureDef boxShapeDef;
        boxShapeDef.shape = &groundEdge;

        // Wall definitions

        groundEdge.Set(b2Vec2(0, 0), b2Vec2(mW, 0));
        groundBody->CreateFixture(&boxShapeDef);

        groundEdge.Set(b2Vec2(0, 0), b2Vec2(0, mH));
        groundBody->CreateFixture(&boxShapeDef);

        groundEdge.Set(b2Vec2(0, mH), b2Vec2(mW, mH));
        groundBody->CreateFixture(&boxShapeDef);

        groundEdge.Set(b2Vec2(mW, mH), b2Vec2(mW, 0));
        groundBody->CreateFixture(&boxShapeDef);

        //Creating a Dynamic Body
        mBodyDef.type = b2_dynamicBody;
        mBodyDef.position.Set(50.0f, 50.0f);
        mBody = mWorld->CreateBody(&mBodyDef);
        mDynamicBox.SetAsBox(3.0f, 3.0f);
        mFixtureDef.shape = &mDynamicBox;
        mFixtureDef.density = 1.0f;
        mFixtureDef.friction = 0.01f;
        //power of bouncing back:
        mFixtureDef.restitution = 12.f; //0.8

        mBody->CreateFixture(&mFixtureDef);

        //Creating a Dynamic Bouncing Ball => restitution is the answer

        b2BodyDef ballBodyDef;
        ballBodyDef.type = b2_dynamicBody;
        ballBodyDef.position.Set(100.f,  100.f);



        // link to rendered object:  ballBodyDef.userData = _ball;
        mBallBody = mWorld->CreateBody(&ballBodyDef);

        b2CircleShape mBallShape;
        mBallShape.m_radius = 5.0;

        b2FixtureDef ballShapeDef;
        ballShapeDef.shape = &mBallShape;
        ballShapeDef.density = 1.0f;
        ballShapeDef.friction = 0.2f;
        //power of bouncing back:
        ballShapeDef.restitution = 10.8f; //0.8
        mBallBody->CreateFixture(&ballShapeDef);
        // ----------------------------



        ElfFlux::gMain->queueObject(this);
        Log("[info] TestWorld %d queued.", getId());

        return Parent::onAdd();
    }
    // ------------------------------------------------------------------------.
    void TestWorld::onRemove() {
        ElfFlux::gMain->unQueueObject(this);
        SAFE_DELETE(mWorld);
        mWorld = nullptr;
        Parent::onRemove();
    }
    // ------------------------------------------------------------------------.
    void TestWorld::initPersistFields()
    {
        Parent::initPersistFields();
        addField("x", TypeF32, Offset(mX, TestWorld));
        addField("y", TypeF32, Offset(mY, TestWorld));
        addField("w", TypeF32, Offset(mW, TestWorld));
        addField("h", TypeF32, Offset(mH, TestWorld));
        addField("gravity", TypeF32, Offset(mGroundGravity, TestWorld));

    }



    void TestWorld::Update(const double& dt)
    {
        U32 velocityIterations = 8;
        U32 positionIterations = 2;

        mWorld->Step(dt, velocityIterations, positionIterations);
    }

    void TestWorld::Draw()
    {

        // dglDrawText(mScreen->mProfile->mFont, Point2I(mX, mY), "Box2D Test!!");

        b2Vec2 position = mBody->GetPosition();
        //dglDrawLine(Point2I(mX, mY), Point2I(mRayPos.x, mRayPos.y), ColorF(1.0f, 0.20f, 0.20f, 0.75f));

        //mDynamicBox
        F32 lRadius = 4; //mDynamicBox.m_radius * 1000;
        RectF lBox = RectF(
               (mX + position.x - lRadius), (mY + position.y - lRadius),
               ( lRadius * 2.f), (lRadius * 2.f)
            );
        Render2D.drawRect(lBox, cl_NeonPink, true);
         
        // float angle = mBody->GetAngle();
        // Con::printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);


        position = mBallBody->GetPosition();
        //mDynamicBox
        lRadius = 5; // mBallShape.m_radius; //FIXME!! mBallShape.m_radius * 1000;
        lBox = RectF(
            (mX + position.x - lRadius), (mY + position.y - lRadius),
            (lRadius * 2.f), (lRadius * 2.f)
        );
        Render2D.drawRect(lBox, cl_AcidGreen, true);

        // angle = mBallBody->GetAngle();

        Render2D.drawLine(mX, mY, mW + mX,mY, Color4F(1.0f, 1.0f, 1.0f, 1.f));
        Render2D.drawLine(mW + mX, mY, mW + mX, mH+ mY, cl_White);
        Render2D.drawLine(mW + mX, mH+ mY, mX, mH+ mY, cl_White);
        Render2D.drawLine(mX, mH+ mY, mX, mY,cl_White);

    }


    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    // https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html
    void hello()
    {
        //Creating a World
        b2Vec2 gravity(0.0f, -10.0f);
        b2World world(gravity);

        //Creating a Ground Box
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(0.0f, -10.0f);
        b2Body* groundBody = world.CreateBody(&groundBodyDef);
        b2PolygonShape groundBox;
        groundBox.SetAsBox(50.0f, 10.0f);
        groundBody->CreateFixture(&groundBox, 0.0f);
        
        //Creating a Dynamic Body
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(0.0f, 4.0f);
        b2Body* body = world.CreateBody(&bodyDef);

        b2PolygonShape dynamicBox;
        dynamicBox.SetAsBox(1.0f, 1.0f);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;
        body->CreateFixture(&fixtureDef);

        // Simulating the World
        F32 timeStep = 1.0f / 60.0f;
        U32 velocityIterations = 6;
        U32 positionIterations = 2;
        for (U32 i = 0; i < 60; ++i)
        {
            world.Step(timeStep, velocityIterations, positionIterations);
            b2Vec2 position = body->GetPosition();
            float angle = body->GetAngle();
            Con::printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
        }



    }

} //namespace b2bTest



/*

Example:

function fooDings()
{
  return 47;
}
*/

// ConsoleFunction(b2bTest_ochjo, ConsoleVoid, 1, 1, "")
// {
//     KorkApi::ConsoleValue dings = Con::executef( "fooDings" );
//     Con::errorf("DINGS IS: %s", dings).;
//     F32 fdings = dAtof(dings.getFloat());
//     Con::errorf("DINGS IS: %s float is: %f", dings, fdings);
// }
