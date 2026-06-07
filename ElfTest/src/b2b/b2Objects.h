#include <core/fluxGlobals.h>
//-----------------------------------------------------------------------------
// Ohmtal Game Engine
//-----------------------------------------------------------------------------
// Box2D Objects / OGE Glue
//-----------------------------------------------------------------------------
#pragma once

#include "sim/simBase.h"
#include "console/consoleTypes.h"
#include "core/fluxGlobals.h"
#include "core/fluxBaseObject.h"
#include "Box2D/Box2D.h"
#include "render/Sprite.h"


//=================================================================================================
// World2b
//=================================================================================================


extern U32 gB2ratio;

namespace ElfFlux {
class Body2b;

class World2b : 
    public SimObject,
    public b2ContactListener
{
private:
    typedef SimObject Parent;
protected:

public:
    DECLARE_CONOBJECT(World2b);


    World2b();
    ~World2b();
    b2World* mWorld;

    void setGravity(b2Vec2 lGravitiy);
    b2Vec2 getGravity(); 

    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;

}; //class

//=================================================================================================
// S h a p e s 
//=================================================================================================
/*
>> class b2Shape; << parent object
  virtual b2Shape* Clone(b2BlockAllocator* allocator) const = 0;
  Type GetType() const;
  virtual int32 GetChildCount() const = 0;
  virtual bool TestPoint(const b2Transform& xf, const b2Vec2& p) const = 0;
  virtual void ComputeDistance(const b2Transform& xf, const b2Vec2& p, float32* distance, b2Vec2* normal, int32 childIndex) const= 0;
  virtual bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
                        const b2Transform& transform, int32 childIndex) const = 0;
  virtual void ComputeAABB(b2AABB* aabb, const b2Transform& xf, int32 childIndex) const = 0;
  virtual void ComputeMass(b2MassData* massData, float32 density) const = 0;

  m_radius

class b2CircleShape;
   int32 GetSupport(const b2Vec2& d) const;
   const b2Vec2& GetSupportVertex(const b2Vec2& d) const;
   const b2Vec2& GetVertex(int32 index) const;

class b2EdgeShape;
   void Set(const b2Vec2& v1, const b2Vec2& v2);

class b2PolygonShape;
    void Set(const b2Vec2* points, int32 count);
    void SetAsBox(float32 hx, float32 hy);
    void SetAsBox(float32 hx, float32 hy, const b2Vec2& center, float32 angle);
    int32 GetVertexCount() const { return m_count; }
    const b2Vec2& GetVertex(int32 index) const;
    bool Validate() const;

class b2ChainShape;
   void CreateLoop(const b2Vec2* vertices, int32 count);
   void CreateChain(const b2Vec2* vertices, int32 count);
   void SetPrevVertex(const b2Vec2& prevVertex);
   void SetNextVertex(const b2Vec2& nextVertex);
   void GetChildEdge(b2EdgeShape* edge, int32 index) const;
*/

//=================================================================================================
// PolygonShape2b
//=================================================================================================

enum Shape2bShapeTypes
{
    PolygonShape = 0,
    CircleShape,
    EdgeShape,
    ChainShape

};

class Shape2b : public SimObject
{
private:
    typedef SimObject Parent;

public:
    DECLARE_CONOBJECT(Shape2b);

    Shape2bShapeTypes mShapeType;

    b2PolygonShape    mPoligonShape;
    b2CircleShape     mCircleShape;
    b2EdgeShape       mEdgeShape;
    b2ChainShape      mChainShape;

    Shape2b() { mShapeType = PolygonShape; }
    static void initPersistFields();
    b2Shape* getShape();
};


//=================================================================================================
// BodyDef2b ==>  b2BodyDef
//=================================================================================================
class BodyDef2b : public SimObject
{
private:
    typedef SimObject Parent;

public:
    DECLARE_CONOBJECT(BodyDef2b);
    static void initPersistFields();

    b2BodyDef mBodyDef;

};
//=================================================================================================
// FixtureDef2b ==>  b2FixtureDef
//=================================================================================================
class FixtureDef2b : public SimObject
{
private:
    typedef SimObject Parent;

public:
    DECLARE_CONOBJECT(FixtureDef2b);
    
    b2FixtureDef mFixtureDef;

    

    static void initPersistFields();



};
//=================================================================================================
// Body2b ==>  b2Body
//=================================================================================================
class Body2b : public Sprite
{
private:
    typedef Sprite Parent;
protected:
    Point2F mAxisVector = {0.f,0.f};
    F32     mAxisVectorAngle = 0.f;

public:
    DECLARE_CONOBJECT(Body2b);


    bool mDebugRender = true;
    b2Body* mBody = nullptr;
    b2Fixture* mFixture = nullptr;

    bool mSendCollision = false;
    bool getSendCollision() { return mSendCollision; }

    Point2F getAxisVector() { return mAxisVector; }
    F32 getAxisVectorAngle() { return mAxisVectorAngle; }



    // void copyFrom(Body2b* object, bool copyDynamicFields = true);
    // virtual Body2b* clone(bool attachToScreen = true, bool copyDynamicFields = true);

    virtual void debugRender();
    virtual void debugRenderShape(b2Fixture* fixture, const b2Transform& xf, const b2Color& color);
    

    virtual void Update(const double& dt) override;
    virtual void Draw() override;


    static void initPersistFields();
    bool onAdd() override;
    void onRemove() override;


};
} //namespace
