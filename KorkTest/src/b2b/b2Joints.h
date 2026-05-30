//-----------------------------------------------------------------------------
// Ohmtal Game Engine
//-----------------------------------------------------------------------------
// Box2D Joints / OGE Glue
//-----------------------------------------------------------------------------
#ifndef _B2JOINTS_H_
#define _B2JOINTS_H_

#ifndef _CONSOLEOBJECT_H_
#include "console/consoleObject.h"
#endif

#ifndef _MPOINT_H_
#include "math/mPoint.h"
#endif

#ifndef _TOM2D_H_
#include "tom2D.h"
#endif

#ifndef BOX2D_H
#include "Box2D/Box2D.h"
#endif

//=================================================================================================
// Joint2b ==>  b2Joint
// 
/* enum b2JointType
{
e_unknownJoint,
e_revoluteJoint,
e_prismaticJoint,
e_distanceJoint,
e_pulleyJoint,
e_mouseJoint,
e_gearJoint,
e_wheelJoint,
e_weldJoint,
e_frictionJoint,
e_ropeJoint,
e_motorJoint
};
*/
//=================================================================================================
/* 
class Joint2b :
    public SimObject,
    public b2Joint
    
{
private:
    typedef SimObject Parent;
public:
   //?? DECLARE_CONOBJECT(Joint2b);
}; //class Joint2b
*/
//-------------------------------------------------------------------------------------
// Joint Tools
//-------------------------------------------------------------------------------------
inline void LinearStiffness2b(float& stiffness, float& damping,
	float frequencyHertz, float dampingRatio,
	const b2Body* bodyA, const b2Body* bodyB)
{
	float massA = bodyA->GetMass();
	float massB = bodyB->GetMass();
	float mass;
	if (massA > 0.0f && massB > 0.0f)
	{
		mass = massA * massB / (massA + massB);
	}
	else if (massA > 0.0f)
	{
		mass = massA;
	}
	else
	{
		mass = massB;
	}

	float omega = 2.0f * b2_pi * frequencyHertz;
	stiffness = mass * omega * omega;
	damping = 2.0f * mass * dampingRatio * omega;
}

inline void AngularStiffness2b(float& stiffness, float& damping,
	float frequencyHertz, float dampingRatio,
	const b2Body* bodyA, const b2Body* bodyB)
{
	float IA = bodyA->GetInertia();
	float IB = bodyB->GetInertia();
	float I;
	if (IA > 0.0f && IB > 0.0f)
	{
		I = IA * IB / (IA + IB);
	}
	else if (IA > 0.0f)
	{
		I = IA;
	}
	else
	{
		I = IB;
	}

	float omega = 2.0f * b2_pi * frequencyHertz;
	stiffness = I * omega * omega;
	damping = 2.0f * I * dampingRatio * omega;
}

//-------------------------------------------------------------------------------------
// Revolute Joint
//-------------------------------------------------------------------------------------

class RevoluteJointDef2b : public SimObject {

private:
    typedef SimObject Parent;
public:
    DECLARE_CONOBJECT(RevoluteJointDef2b);

//damn override does not work!!!
    b2RevoluteJointDef mJointDef;

    static void initPersistFields();

}; //class RevoluteJointDef2b

class RevoluteJoint2b :
    public SimObject
{
private:
    typedef SimObject Parent;
public:
    DECLARE_CONOBJECT(RevoluteJoint2b);

    ~RevoluteJoint2b();
    b2World* mWorld;
    b2RevoluteJoint* mJoint;

}; //class RevoluteJoint2b

//-----------------------------------------------------------------------------
// Distance Joint
//-----------------------------------------------------------------------------
class DistanceJointDef2b : public SimObject {

private:
    typedef SimObject Parent;
public:
    DECLARE_CONOBJECT(DistanceJointDef2b);

    //damn override does not work!!!
    b2DistanceJointDef mJointDef;

    static void initPersistFields();

}; //class RevoluteJointDef2b


class DistanceJoint2b :
	public SimObject
{
private:
	typedef SimObject Parent;
public:
	DECLARE_CONOBJECT(DistanceJoint2b);

	~DistanceJoint2b();
	b2World* mWorld;
	b2DistanceJoint* mJoint;
	

}; //class RevoluteJoint2b


#endif //_B2JOINTS_H_