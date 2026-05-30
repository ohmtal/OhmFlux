//-----------------------------------------------------------------------------
// Ohmtal Game Engine
//-----------------------------------------------------------------------------
/* 
  Box2D Objects / OGE Glue

  @author T.Huehn (XXTH)
  @since  2021-03-24

 Todo: 
    [?] EdgeShape SetOneSided / SetTwoSided ?? seams i've a old box2d version ^^ 

    [X] PreSolve - for ignore collision

       maybe also ?? 
      [?]  Fließband : look at demo: Conveyor Belt 
         ==> contact SetTangentSpeed

    [X] b2PolygonShape Points 

    [ ] missing world methods
        [ ] World QueryAABB => Explosion example: http://www.iforce2d.net/b2dtut/explosions
        [ ] World rayCast

    [X] missing body methods (https://box2d.org/documentation/classb2_body.html)
        ApplyForce(const b2Vec2& force, const b2Vec2& point, bool wake);
        void ApplyForceToCenter(const b2Vec2& force, bool wake);
        void ApplyTorque(float32 torque, bool wake);
        
  

*/




#include "console/console.h"
#include "console/consoleTypes.h"
#include "platform/platform.h"
#include "memory/safeDelete.h"
#include "graphics/dgl.h"
#include "core/Utility.h"
#include "Box2D/Box2D.h"
#include "tom2D.h"
#include "b2Objects.h"



// global ratio for pixel to meters can be changed with World2b::setRatio console method
U32 gB2ratio = 32;


//=================================================================================================
// World2b
//=================================================================================================
 IMPLEMENT_CONOBJECT(World2b);

 World2b::World2b() {
     mWorld = new b2World(b2Vec2(0.f, 0.f));
     mWorld->SetContactListener(this);
 }
 World2b::~World2b() {
     SAFE_DELETE(mWorld);
 }
 void World2b::setGravity(b2Vec2 lGravitiy) {
     mWorld->SetGravity(lGravitiy);
 }
 b2Vec2 World2b::getGravity() {
     return mWorld->GetGravity(); 
 }


 void World2b::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
 {

     // turn fixtures into  objects
     Body2b* A = static_cast<Body2b*>
         (contact->GetFixtureA()->GetBody()->GetUserData());
     Body2b* B = static_cast<Body2b*>
         (contact->GetFixtureB()->GetBody()->GetUserData());
     bool doIgnore = false;
     if (A->getSendCollision() )
     {
         if (A->isMethod("onCollisionIgnore"))
         {
             doIgnore = dAtob(Con::executef(A, 6, "onCollisionIgnore",
                 Con::getIntArg(B->getId())
             ));
         }
     }
     if (B->getSendCollision())
     {
         if (B->isMethod("onCollisionIgnore"))
         {
             doIgnore = dAtob(Con::executef(B, 6, "onCollisionIgnore",
                 Con::getIntArg(A->getId())
             ));
         }
     }
     if (doIgnore)
         contact->SetEnabled(false);

 }

 void World2b::BeginContact(b2Contact* contact)
 {
     if (contact && contact->IsTouching())
     {
         // turn fixtures into  objects
         Body2b* A = static_cast<Body2b*>
             (contact->GetFixtureA()->GetBody()->GetUserData());
         Body2b* B = static_cast<Body2b*>
             (contact->GetFixtureB()->GetBody()->GetUserData());

         
         U32 lPointCount = 0; 
         b2WorldManifold lWorldManifold;

         /*
         	b2Vec2 normal;								///< world vector pointing from A to B
        	b2Vec2 points[b2_maxManifoldPoints];		///< world contact point (point of intersection)
	        float32 separations[b2_maxManifoldPoints];	///< a negative value indicates overlap, in meters
         */
         contact->GetWorldManifold(&lWorldManifold);
         const b2Vec2& normal = lWorldManifold.normal;
         lPointCount = contact->GetManifold()->pointCount;

         if (A->getSendCollision() && A->isMethod("onCollision"))
         {
             Con::executef(A, 6, "onCollision",
                 Con::getIntArg(B->getId()),
                 Con::getFloatArg(normal.x),
                 Con::getFloatArg(normal.y),
                 Con::getFloatArg(lWorldManifold.points[0].x),
                 Con::getFloatArg(lWorldManifold.points[0].y)
             );
         }
         if (B->getSendCollision() && B->isMethod("onCollision"))
         {
             Con::executef(B, 6, "onCollision",
                 Con::getIntArg(A->getId()),
                 Con::getFloatArg(normal.x),
                 Con::getFloatArg(normal.y),
                 Con::getFloatArg(lWorldManifold.points[0].x),
                 Con::getFloatArg(lWorldManifold.points[0].y)
             );
         }
         

#ifdef B2_DEBUG
         Con::errorf("======== World2b::BeginContact objects =========");

         Con::printf(
                     "objects: %d <-> %d \n"
                     "pointCount %d \n"
                     "normal: %f, %f "
                     
             , A->getId(), B->getId()
             , lPointCount
             , normal.x, normal.y
         );
         for (U32 i = 0; i < lPointCount; i++) {


             Con::printf("point%d: %f, %f"
                 ,i
                 , lWorldManifold.points[i].x
                 , lWorldManifold.points[i].y
             );

         } //for
#endif
     } //contact / istouching
 }
 void World2b::EndContact(b2Contact* contact)
 {
     if (contact)
     {
         // turn fixtures into  objects
         Body2b* A = static_cast<Body2b*>
             (contact->GetFixtureA()->GetBody()->GetUserData());
         Body2b* B = static_cast<Body2b*>
             (contact->GetFixtureB()->GetBody()->GetUserData());

         if (A->getSendCollision() && A->isMethod("onCollisionEnd"))
         {
             Con::executef(A, 2, "onCollisionEnd",
                 Con::getIntArg(B->getId())
             );
         }
         if (B->getSendCollision() && B->isMethod("onCollisionEnd"))
         {
             Con::executef(B, 2, "onCollisionEnd",
                 Con::getIntArg(A->getId())
            );
         }

#ifdef B2_DEBUG
         Con::errorf("======== World2b::EndContact objects =========");

         Con::printf(
             "objects: %d <-> %d \n"
 
             , A->getId(), B->getId()
         );
#endif


     }

 }


 ConsoleMethod(World2b, setGravity, void, 3, 3, "(b2Vec2 gravity)"
     "set the world gravity")
 {
     F32 x, y;
     dSscanf(argv[2], "%g %g", &x, &y);
     object->setGravity(b2Vec2(x, y));
 }




 ConsoleMethod(World2b, setRatio, void, 3, 3, "(U32 ratio pixel/m)")
 {
     gB2ratio = dAtoi(argv[2]);
 }


 ConsoleMethod(World2b, isLocked, bool, 2, 2, "return world is locked")
 {
     return object->mWorld->IsLocked();
 }


 ConsoleMethod(World2b, step, void, 5, 5, "(float timeStep, int velocityIterations, int positionIterations)"
     "example .step(%fdt, 8, 3)")
 {
 
     F32 timeStep = dAtof(argv[2]);
     U32 velocityIterations = dAtoi(argv[3]);
     U32 positionIterations = dAtoi(argv[4]);
     object->mWorld->Step(timeStep, velocityIterations, positionIterations);
 }


 //=================================================================================================
/**
  PolygonShape2b
     void Set(const b2Vec2* points, int32 count);
     void SetAsBox(float32 hx, float32 hy);
     void SetAsBox(float32 hx, float32 hy, const b2Vec2& center, float32 angle);
     int32 GetVertexCount() const { return m_count; }
     const b2Vec2& GetVertex(int32 index) const;
     bool Validate() const;
*/
//=================================================================================================
 IMPLEMENT_CONOBJECT(Shape2b);


 void Shape2b::initPersistFields()
 {
     Parent::initPersistFields();
     addField("shapeType", TypeS32, Offset(mShapeType, Shape2b));

 }

 b2Shape* Shape2b::getShape()
 {
     switch (mShapeType)
     {
     case PolygonShape:
         return &mPoligonShape;
         break;
     case CircleShape:
         return &mCircleShape;
         break;
     case EdgeShape:
         return &mEdgeShape;
         break;
     case ChainShape:
         return &mChainShape;
     }

     return NULL; //??
 }

 ConsoleMethod(Shape2b, setAsBox, bool, 3, 3, "(b2Vec2 x y)"
     "set as box by x and y" )
 {
     if (object->mShapeType != PolygonShape)
         return false;
     F32 x, y;
     dSscanf(argv[2], "%g %g", &x, &y);
     object->mPoligonShape.SetAsBox(x / gB2ratio, y / gB2ratio);

     return true;
 }

 ConsoleMethod(Shape2b, setPoints, bool, 3, 3, "(b2Vec2 x y SPC b2Vec2 x y SPC ...) closed automaticly!!"
     "set as box by x and y")
 {
     if (object->mShapeType != PolygonShape)
         return false;
    const U32 pointElements = Utility::mGetStringElementCount(argv[2]);

    // Check even number of elements and at least 6 (3 points) exist.
    if ((pointElements % 2) != 0 || pointElements < 6 || pointElements >(2 * b2_maxPolygonVertices))
    {
        Con::warnf("SceneObject::createPolygonCollisionShape() - Invalid number of parameters!");
        return false;
    }

    b2Vec2 localPoints[b2_maxPolygonVertices];
    Point2F tmpPoint;
    for (U32 elementIndex = 0, pointIndex = 0; elementIndex < pointElements; elementIndex += 2, pointIndex++)
    {
        tmpPoint = Utility::mGetStringElementVector(argv[2], elementIndex);
        localPoints[pointIndex].x = tmpPoint.x / gB2ratio;
        localPoints[pointIndex].y = tmpPoint.y / gB2ratio;
    }

    const U32 pointCount = pointElements / 2;
    object->mPoligonShape.Set(localPoints, pointCount);
    return true;
 }

 ConsoleMethod(Shape2b, setRadius, bool, 3, 3, "(F32 r)"
     "set circle radius")
 {
     if (object->mShapeType != CircleShape)
         return false;
     F32 rad = dAtof(argv[2]) / gB2ratio;
     object->mCircleShape.m_radius = rad;
     return true;

 }

 ConsoleMethod(Shape2b, setEdge, bool, 3, 3, "(x y w h)"
     "set as box by x and y")
 {
     if (object->mShapeType != EdgeShape)
         return false;
     F32 x, y, w, h;
     dSscanf(argv[2], "%g %g %g %g", &x, &y, &w, &h );
     object->mEdgeShape.Set(b2Vec2(x / gB2ratio, y / gB2ratio),b2Vec2(w / gB2ratio,h / gB2ratio));

     return true;
 }


//=================================================================================================
// BodyDef2b ==>  b2BodyDef
//=================================================================================================
 IMPLEMENT_CONOBJECT(BodyDef2b);

 void BodyDef2b::initPersistFields()
 {
     Parent::initPersistFields();
     //   addField("TextLayer", TypeS32, Offset(mTextLayer, tom2DCtrl));
     addField("type", TypeS32, Offset(mBodyDef.type, BodyDef2b), "type of body: $staticBody= 0;$kinematicBody = 1;$dynamicBody = 2;");
     addField("active", TypeBool, Offset(mBodyDef.active, BodyDef2b), "is active");
     addField("allowSleep", TypeBool, Offset(mBodyDef.allowSleep, BodyDef2b), "is active");
     addField("linearDamping", TypeF32, Offset(mBodyDef.linearDamping, BodyDef2b), "reduce the linear velocity");
     addField("angularDamping", TypeF32, Offset(mBodyDef.angularDamping, BodyDef2b), "reduce the angular velocity");
     addField("angularVelocity", TypeF32, Offset(mBodyDef.angularVelocity, BodyDef2b), "The angular velocity of the body.");
     addField("gravityScale", TypeF32, Offset(mBodyDef.gravityScale, BodyDef2b), "Scale the gravity applied to this body. ");
     addField("linearVelocityX", TypeF32, Offset(mBodyDef.linearVelocity.x, BodyDef2b), "The linear X velocity of the body.");
     addField("linearVelocityY", TypeF32, Offset(mBodyDef.linearVelocity.y, BodyDef2b), "The linear Y velocity of the body.");


 }

 ConsoleMethod(BodyDef2b, setPosition, void, 3, 3, "(b2Vec2 x y)"
     "set position by x and y")
 {
     F32 x, y;
     dSscanf(argv[2], "%g %g", &x, &y);
     object->mBodyDef.position.Set(x / gB2ratio, y / gB2ratio);
     
 }

 //=================================================================================================
// FixtureDef2b ==>  b2FixtureDef
//=================================================================================================
 IMPLEMENT_CONOBJECT(FixtureDef2b);
 void FixtureDef2b::initPersistFields()
 {
     Parent::initPersistFields();

     addField("density", TypeF32, Offset(mFixtureDef.density, FixtureDef2b));
     addField("friction", TypeF32, Offset(mFixtureDef.friction, FixtureDef2b));
     addField("restitution", TypeF32, Offset(mFixtureDef.restitution, FixtureDef2b));
 }


 ConsoleMethod(FixtureDef2b, setShape, bool, 3, 3, "(shapeobject)"
     "")
 {

//funzt so net !!!! 
     Shape2b* lShape = (Shape2b*)Sim::findObject(dAtoi(argv[2]));
     if (lShape) {
         object->mFixtureDef.shape = lShape->getShape();
         return true;
     }
     return false;
 }


 //=================================================================================================
// Body2b 
//=================================================================================================
IMPLEMENT_CONOBJECT(Body2b);

Body2b::Body2b()
{
    mAxisVector = Point2F(0.f, 0.f);
    mAxisVectorAngle = 0.f;

}

Body2b::~Body2b() 
{

    

    if (mBody)
    {
        
        b2World* lWorld = mBody->GetWorld();
        if (lWorld)
        {
            lWorld->DestroyBody(mBody);
        }
        mBody = NULL;
    }
    
}

void Body2b::onUpdate(F32 fDt)
{

    if (mBody->GetType() != b2_staticBody)
    {

        b2Transform transform = mBody->GetTransform();
        b2Vec2 axis = b2Mul(transform.q, b2Vec2(1.0f, 0.0f));
        mAxisVector = Point2F(axis.x, axis.y);
        mAxisVectorAngle = mRadToDeg(mAtan(axis.x, axis.y)); //mAtan

        mX = transform.p.x * gB2ratio;
        mY = transform.p.y * gB2ratio;
        mRotation = mAxisVectorAngle; // +90.f;
    }

    updateAnimation(fDt);

    Con::executef(this, 2, "onUpdate", Con::getFloatArg(fDt));
   

}

//------------------------------------------------------------------------------
void Body2b::debugRender(U32 dt, Point3F lOffset)
{
    if (!mDebugRender)
        return;

    b2Body* b = mBody;
    const b2Transform& xf = b->GetTransform();
    for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
    {
        if (b->IsActive() == false)
        {
            debugRenderShape(f, xf, b2Color(0.5f, 0.5f, 0.3f));
        }
        else if (b->GetType() == b2_staticBody)
        {
            debugRenderShape(f, xf, b2Color(0.5f, 0.9f, 0.5f));
        }
        else if (b->GetType() == b2_kinematicBody)
        {
            debugRenderShape(f, xf, b2Color(0.5f, 0.5f, 0.9f));
        }
        else if (b->IsAwake() == false)
        {
            debugRenderShape(f, xf, b2Color(0.6f, 0.6f, 0.6f));
        }
        else
        {
            debugRenderShape(f, xf, b2Color(0.5f, 1.f, 0.5f));
        }
    }


}
void Body2b::debugRenderShape(b2Fixture* fixture, const b2Transform& xf, const b2Color& color)
{


    switch (fixture->GetType())
    {
    case b2Shape::e_circle:
    {
        b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();

        b2Vec2 center = b2Mul(xf, circle->m_p);
        center *= gB2ratio;
        float radius = circle->m_radius * gB2ratio;
        b2Vec2 axis = b2Mul(xf.q, b2Vec2(1.0f, 0.0f));
        dglDrawSolidCircle(
            Point2F(center.x, center.y), 
            radius, 
            Point2F(axis.x,axis.y),
            ColorF(color.r, color.g, color.b)
        );
        
    }
    break;

    case b2Shape::e_edge:
    {
        b2EdgeShape* edge = (b2EdgeShape*)fixture->GetShape();
        b2Vec2 v1 = b2Mul(xf, edge->m_vertex1);
        b2Vec2 v2 = b2Mul(xf, edge->m_vertex2);
        //m_debugDraw->DrawSegment(v1, v2, color);
        v1 *= gB2ratio;
        v2 *= gB2ratio;

        //dglDrawRect(Point2I(v1.x, v1.y), Point2I(v2.x, v2.y), ColorF(color.r,color.g,color.b));

        dglDrawLine(Point2I(v1.x, v1.y), Point2I(v2.x, v2.y), ColorF(color.r, color.g, color.b));


        /* 
        if (edge->m_oneSided == false)
        {
            m_debugDraw->DrawPoint(v1, 4.0f, color);
            m_debugDraw->DrawPoint(v2, 4.0f, color);
        }
        */
    }
    break;

    case b2Shape::e_chain:
    {
        b2ChainShape* chain = (b2ChainShape*)fixture->GetShape();
        int32 count = chain->m_count;
        const b2Vec2* vertices = chain->m_vertices;

        b2Vec2 v1 = b2Mul(xf, vertices[0]);
        v1 *= gB2ratio;
        for (int32 i = 1; i < count; ++i)
        {
            b2Vec2 v2 = b2Mul(xf, vertices[i]);
            //m_debugDraw->DrawSegment(v1, v2, color);
            v2 *= gB2ratio;
            dglDrawRect(Point2I(v1.x, v1.y), Point2I(v2.x, v2.y), ColorF(color.r, color.g, color.b));
            v1 = v2;
        }
    }
    break;

    case b2Shape::e_polygon:
    {
        b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
        int32 vertexCount = poly->m_count;

        b2Assert(vertexCount <= b2_maxPolygonVertices);
        if (vertexCount > 1)
        {
            b2Vec2 vertices[b2_maxPolygonVertices];
            b2Vec2 tmp = b2Mul(xf, poly->m_vertices[0]);
            Point2I first = Point2I(tmp.x * gB2ratio, tmp.y * gB2ratio);
            Point2I cur;
            Point2I last = first;

            for (int32 i = 1; i < vertexCount; ++i)
            {
                tmp = b2Mul(xf, poly->m_vertices[i]);
                cur = Point2I(tmp.x * gB2ratio, tmp.y * gB2ratio);
                dglDrawLine(last ,cur , ColorF(color.r, color.g, color.b));
                last = cur;
            }
            dglDrawLine(last , first , ColorF(color.r, color.g, color.b));

        }


        
    }
    break;

    default:
        break;
    }


}


/*
void Body2b::copyFrom(Body2b* object, bool copyDynamicFields )
{
    Parent::copyFrom(object);

    //bodies

    there id not fuction to clone the body!!!!

    object->mBody->
    object->mBody->GetWorld
    
    //fixture
    for (b2Fixture* pFixture = object->mBody->GetFixtureList(); pFixture; pFixture = pFixture->GetNext())
    {
        b2FixtureDef fixtureDef;
        
        fixtureDef.density = pFixture->GetDensity();
        fixtureDef.friction = pFixture->GetFriction();
        fixtureDef.restitution = pFixture->GetRestitution();
        fixtureDef.isSensor = pFixture->IsSensor();
        fixtureDef.shape = pFixture->GetShape();
    
        mBody->CreateFixture(&fixtureDef);
    }

    


}

Body2b* Body2b::clone(bool attachToScreen, bool copyDynamicFields)
{
    Body2b* result = new Body2b();

    result->copyFrom(this);

    if (attachToScreen && mScreen)
        mScreen->addObject(result);

    return result;
}
*/

ConsoleMethod(Body2b, CreateBody, bool, 4, 4, "(World2b world, BodyDef2b bodydef)"
    "")
{

    World2b* lWorld = (World2b*)Sim::findObject(dAtoi(argv[2]));
    BodyDef2b* lBodydef = (BodyDef2b*)Sim::findObject(dAtoi(argv[3]));
    //fixme vector of boddies ? 
    object->mBody = lWorld->mWorld->CreateBody(& lBodydef->mBodyDef);
    
    //userdata for collisions 
    object->mBody->SetUserData(object);

    return true;
    
}






ConsoleMethod(Body2b, CreateFixture, bool, 3, 3, "(FixtureDef2b fixtureDef)"
    "")
{

    FixtureDef2b* lFixtureDef2b = (FixtureDef2b*)Sim::findObject(dAtoi(argv[2]));

    b2FixtureDef lFixtureDef = lFixtureDef2b->mFixtureDef;

    if (!lFixtureDef2b->mFixtureDef.shape)
        return false;

    //FIXME vector of Fixture !!!! look at edges!!
    object->mFixture = object->mBody->CreateFixture( &lFixtureDef);

    return true;
}



ConsoleMethod(Body2b, SetActive, void, 3, 3, "params: bool active"
    "default true")
{
    object->mBody->SetActive(dAtob(argv[2]));
}



ConsoleMethod(Body2b, getTransform, const char *, 2, 2, "return x y angle"
"")
{
    char* rbuf = Con::getReturnBuffer(256);
    //b2Vec2 pos = object->mBody->GetPosition() * gB2ratio;
    //F32 angle = object->mBody->GetAngle();
    Point2F pos = object->get2DPosition();
    dSprintf(rbuf, 256, "%f %f %f", pos.x, pos.y, object->getAxisVectorAngle());
    return rbuf;
}

ConsoleMethod(Body2b, getPosition, const char*, 2, 2, "return x y"
    "")
{
    char* rbuf = Con::getReturnBuffer(256);
    b2Vec2 pos = object->mBody->GetPosition() * gB2ratio;
    dSprintf(rbuf, 256, "%f %f", pos.x, pos.y);
    return rbuf;
}

ConsoleMethod(Body2b, GetLinearVelocity, const char*, 2, 2, "return x y"
    "")
{
    char* rbuf = Con::getReturnBuffer(256);
    b2Vec2 pos = object->mBody->GetLinearVelocity() ;
    dSprintf(rbuf, 256, "%f %f", pos.x, pos.y);
    return rbuf;
}

ConsoleMethod(Body2b, GetActive, bool, 2, 2, "return isActive"
    "")
{
    
    return object->mBody->IsActive();
}


ConsoleMethod(Body2b, setPosition, void, 3, 3, "params: x y"
    "")
{
    F32 x, y, angle = 0.f;
    dSscanf(argv[2], "%g %g %g", &x, &y);

    object->mBody->SetTransform(b2Vec2(x / gB2ratio, y / gB2ratio), mDegToRad(angle));
}


ConsoleMethod(Body2b, setTransform, void, 3, 3, "params: x y angle"
    "")
{
    F32 x, y, angle;
    dSscanf(argv[2], "%g %g %g", &x, &y, &angle);
     
    object->mBody->SetTransform(b2Vec2(x / gB2ratio, y / gB2ratio), mDegToRad(angle));
}

ConsoleMethod(Body2b, SetLinearVelocity, void, 3, 3, "params: x y"
    "")
{
    F32 x, y;
    dSscanf(argv[2], "%g %g", &x, &y);
    object->mBody->SetLinearVelocity(b2Vec2(x, y));

    
}

/// Set the angular velocity.
/// @param omega the new angular velocity in radians/second.
/// void SetAngularVelocity(float32 omega);
ConsoleMethod(Body2b, SetAngularVelocity, void, 3, 3, "params: F32 velocity in radians/second"
    "")
{
    object->mBody->SetAngularVelocity(dAtof(argv[2]));
}



ConsoleMethod(Body2b, ApplyLinearImpulse, void, 5, 5, "params: Point2I impulse, Point2I point, bool wake"
    "")
{
    b2Vec2 impulse;
    b2Vec2 point;
    bool wake;
    dSscanf(argv[2], "%g %g", &impulse.x, & impulse.y);
    dSscanf(argv[3], "%g %g", &point.x, &point.y);
    point = point / gB2ratio;
    wake = dAtob(argv[4]);
    object->mBody->ApplyLinearImpulse(impulse, point, wake);
}

ConsoleMethod(Body2b, ApplyLinearImpulseToCenter, void, 4, 4, "params: Point2I impulse,  bool wake"
    "")
{
    b2Vec2 impulse;
    b2Vec2 point;
    bool wake;
    dSscanf(argv[2], "%g %g", &impulse.x, &impulse.y);
    point = point / gB2ratio;
    wake = dAtob(argv[3]);
    object->mBody->ApplyLinearImpulseToCenter(impulse,  wake);
}


ConsoleMethod(Body2b, ApplyForce, void, 5, 5, "params: Point2I force, Point2I point, bool wake"
    "")
{
    b2Vec2 force;
    b2Vec2 point;
    bool wake;
    dSscanf(argv[2], "%g %g", &force.x, &force.y);
    dSscanf(argv[3], "%g %g", &point.x, &point.y);
    point = point / gB2ratio;
    wake = dAtob(argv[4]);
    object->mBody->ApplyForce(force, point, wake);
}

ConsoleMethod(Body2b, ApplyForceToCenter, void, 4, 4, "params: Point2I force,  bool wake"
    "")
{
    b2Vec2 force;
    bool wake;
    dSscanf(argv[2], "%g %g", &force.x, &force.y);
    wake = dAtob(argv[3]);
    object->mBody->ApplyForceToCenter(force, wake);
}


ConsoleMethod(Body2b, ApplyAngularImpulse, void, 4, 4, "params: F32 impulse, bool wake"
    "")
{
    F32 impulse = dAtof(argv[2]);
    bool  wake = dAtob(argv[3]);
    object->mBody->ApplyAngularImpulse(impulse, wake);
}



ConsoleMethod(Body2b, SetGravityScale, void, 3, 3, "params: F32 scale"
    "default 1.0, 0=ignore gravity")
{
    object->mBody->SetGravityScale(dAtof(argv[2]));
}
ConsoleMethod(Body2b, SetLinearDamping, void, 3, 3, "params: F32 value"
    "default 0, 0..1")
{
    object->mBody->SetLinearDamping(dAtof(argv[2]));
}
ConsoleMethod(Body2b, SetAngularDamping, void, 3, 3, "params: F32 value"
    "default 0, 0..1")
{
    object->mBody->SetAngularDamping(dAtof(argv[2]));
}

ConsoleMethod(Body2b, SetFixedRotation, void, 3, 3, "params: bool fixedRotation"
    "default false")
{
    object->mBody->SetFixedRotation(dAtob(argv[2]));
}

//------------------------------------------------------------------------------
ConsoleMethod(Body2b, getAxisVector, const char*, 2, 2, "(return Point2F)"
    "return the normalized axis Vector")
{
    char* returnBuffer = Con::getReturnBuffer(256);
    const Point2F& vec = object->getAxisVector();
    dSprintf(returnBuffer, 256, "%g %g", vec.x, vec.y);
    return returnBuffer;
}


