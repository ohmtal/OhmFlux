//-----------------------------------------------------------------------------
// Ohmtal Game Engine
//-----------------------------------------------------------------------------
/*
  Box2D Joints / OGE Glue

  @author T.Huehn (XXTH)
  @since  2021-04-10

    [ ] adding joints like used in car demo (testbed)
        [X] b2RevoluteJointDef  ==> like pinball flipper or machine so that first :P
           look at Flipper or slider crank !!
        [X] b2DistanceJointDef
           Dominos use b2RevoluteJointDef and  b2DistanceJointDef
        [ ] b2PrismaticJointDef
        [ ] b2WheelJointDef

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
#include "b2Joints.h"


//=================================================================================================
// Joint2b
//=================================================================================================
//?? IMPLEMENT_CONOBJECT(Joint2b);

/* default defs ... easier to copy for each jointDef!
void JointDef2b::initPersistFields()
{
    Parent::initPersistFields();
    //addField("JointType", TypeS32, Offset(mJointType, JointDef2b));
    addField("collideConnected", TypeBool, Offset(mJointDef.collideConnected, JointDef2b));
    

}

ConsoleMethod(JointDef2b, setBodyA, bool, 3, 3, "(Body2b object)")
{
    Body2b* lBody = (Body2b*)Sim::findObject(dAtoi(argv[2]));
    if (!lBody || !lBody->mBody)
        return false;
    object->mJointDef.bodyA = lBody->mBody;
    return true;
}

ConsoleMethod(JointDef2b, setBodyB, bool, 3, 3, "(Body2b object)")
{
    Body2b* lBody = (Body2b*)Sim::findObject(dAtoi(argv[2]));
    if (!lBody || !lBody->mBody)
        return false;
    object->mJointDef.bodyB = lBody->mBody;
    return true;
}
*/

//-------------------------------------------------------------------------------------
// Joint Tools
//-------------------------------------------------------------------------------------
/// Utility to compute linear stiffness values from frequency and damping ratio
///  void b2LinearStiffness(float& stiffness, float& damping,
///     float frequencyHertz, float dampingRatio,
///     const b2Body* bodyA, const b2Body* bodyB);

ConsoleFunction(linearStiffness2b, const char*, 5, 5,
    "Utility to compute linear stiffness values from frequency and damping ratio "
    "(F32 frequencyHertz, F32 dampingRatio,Body2b bodyA, Body2b bodyB) return F32 stiffness SPC F32 damping,"
) {
    F32 stiffness = 0;
    F32 damping = 0;
    F32 frequencyHertz = dAtof(argv[1]);
    F32 dampingRatio   = dAtof(argv[2]);
    Body2b* lBodyA = (Body2b*)Sim::findObject(dAtoi(argv[3]));
    Body2b* lBodyB = (Body2b*)Sim::findObject(dAtoi(argv[4]));
    //sanity
    if (!lBodyA || !lBodyB || !lBodyA->mBody || !lBodyB->mBody)
        return "";
    LinearStiffness2b(stiffness, damping, frequencyHertz, dampingRatio, 
        lBodyA->mBody, lBodyB->mBody);

    char* returnBuffer = Con::getReturnBuffer(256);
    dSprintf(returnBuffer, 256, "%g %g", stiffness, damping);
    return returnBuffer;

}
/// Utility to compute rotational stiffness values frequency and damping ratio
///  void b2AngularStiffness(float& stiffness, float& damping,
///     float frequencyHertz, float dampingRatio,
///     const b2Body* bodyA, const b2Body* bodyB);
ConsoleFunction(angularStiffness2b, const char*, 5, 5,
    "Utility to compute rotational stiffness values frequency and damping ratio"
    "(F32 frequencyHertz, F32 dampingRatio,Body2b bodyA, Body2b bodyB) return F32 stiffness SPC F32 damping,"
) {
    F32 stiffness = 0;
    F32 damping = 0;
    F32 frequencyHertz = dAtof(argv[1]);
    F32 dampingRatio = dAtof(argv[2]);
    Body2b* lBodyA = (Body2b*)Sim::findObject(dAtoi(argv[3]));
    Body2b* lBodyB = (Body2b*)Sim::findObject(dAtoi(argv[4]));
    //sanity
    if (!lBodyA || !lBodyB || !lBodyA->mBody || !lBodyB->mBody)
        return "";
    AngularStiffness2b(stiffness, damping, frequencyHertz, dampingRatio,
        lBodyA->mBody, lBodyB->mBody);

    char* returnBuffer = Con::getReturnBuffer(256);
    dSprintf(returnBuffer, 256, "%g %g", stiffness, damping);
    return returnBuffer;
}


//-------------------------------------------------------------------------------------
// Revolute Joint
//-------------------------------------------------------------------------------------
// RevoluteJointDef2b ==> b2RevoluteJointDef

IMPLEMENT_CONOBJECT(RevoluteJointDef2b);

void RevoluteJointDef2b::initPersistFields()
{
    Parent::initPersistFields();

    addField("collideConnected", TypeBool, Offset(mJointDef.collideConnected, RevoluteJointDef2b), 
        "Set this flag to true if the attached bodies should collide.");


    /// The bodyB angle minus bodyA angle in the reference state (radians).
    /// float32 referenceAngle;
    addField("referenceAngle", TypeF32, Offset(mJointDef.referenceAngle, RevoluteJointDef2b),
        "The bodyB angle minus bodyA angle in the reference state (radians).");

    /// A flag to enable joint limits.
    /// bool enableLimit;
    addField("enableLimit", TypeBool, Offset(mJointDef.enableLimit, RevoluteJointDef2b), 
        "A flag to enable joint limits.");

    /// The lower angle for the joint limit (radians).
    /// float32 lowerAngle;
    addField("lowerAngle", TypeF32, Offset(mJointDef.lowerAngle, RevoluteJointDef2b), 
        "The lower angle for the joint limit (radians)");

    /// The upper angle for the joint limit (radians).
    /// float32 upperAngle;
    addField("upperAngle", TypeF32, Offset(mJointDef.upperAngle, RevoluteJointDef2b),
        "The upper angle for the joint limit (radians).");

    /// A flag to enable the joint motor.
    /// bool enableMotor;
    addField("enableMotor", TypeBool, Offset(mJointDef.enableMotor, RevoluteJointDef2b),
        "A flag to enable the joint motor.");

    /// The desired motor speed. Usually in radians per second.
    /// float32 motorSpeed;
    addField("motorSpeed", TypeF32, Offset(mJointDef.motorSpeed, RevoluteJointDef2b),
        "The desired motor speed. Usually in radians per second.");

    /// The maximum motor torque used to achieve the desired motor speed.
    /// Usually in N-m.
    /// float32 maxMotorTorque;
    addField("maxMotorTorque", TypeF32, Offset(mJointDef.maxMotorTorque, RevoluteJointDef2b),
        "The maximum motor torque used to achieve the desired motor speed.");


}


ConsoleMethod(RevoluteJointDef2b, setBodyA, bool, 3, 3, "(Body2b object)")
{
    Body2b* lBody = (Body2b*)Sim::findObject(dAtoi(argv[2]));
    if (!lBody || !lBody->mBody)
        return false;
    object->mJointDef.bodyA = lBody->mBody;
    return true;
}

ConsoleMethod(RevoluteJointDef2b, setBodyB, bool, 3, 3, "(Body2b object)")
{
    Body2b* lBody = (Body2b*)Sim::findObject(dAtoi(argv[2]));
    if (!lBody || !lBody->mBody)
        return false;
    object->mJointDef.bodyB = lBody->mBody;
    return true;
}


/// Initialize the bodies, anchors, and reference angle using a world
/// anchor point.
//// void Initialize(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor);

ConsoleMethod(RevoluteJointDef2b, Initialize, bool, 5, 5, "(Body2b bodyA, Body2b bodyB, Point2F anchor)"
    "Initialize the bodies, anchors, and reference angle using a world")
{
    Body2b* lBodyA = (Body2b*)Sim::findObject(dAtoi(argv[2]));
    Body2b* lBodyB = (Body2b*)Sim::findObject(dAtoi(argv[3]));
    //sanity
    if (!lBodyA || !lBodyB || !lBodyA->mBody || !lBodyB->mBody)
        return false;
    b2Vec2 anchor;
    dSscanf(argv[4], "%g %g", &anchor.x, &anchor.y);
    anchor.x /= gB2ratio;
    anchor.y /= gB2ratio;


    object->mJointDef.Initialize(lBodyA->mBody, lBodyB->mBody, anchor);
    return true;
}

  
/// The local anchor point relative to bodyA's origin.
/// b2Vec2 localAnchorA;
ConsoleMethod(RevoluteJointDef2b, setlocalAnchorA, bool, 3, 3, "(Point2F anchor)"
    "The local anchor point relative to bodyA's origin.")
{

    b2Vec2 anchor;
    dSscanf(argv[2], "%g %g", &anchor.x, &anchor.y);
    anchor.x /= gB2ratio;
    anchor.y /= gB2ratio;

    object->mJointDef.localAnchorA = anchor;
    return true;
}

/// The local anchor point relative to bodyB's origin.
/// b2Vec2 localAnchorB;
ConsoleMethod(RevoluteJointDef2b, setlocalAnchorB, bool, 3, 3, "(Point2F anchor)"
    "The local anchor point relative to bodyB's origin.")
{

    b2Vec2 anchor;
    anchor.x /= gB2ratio;
    anchor.y /= gB2ratio;
    dSscanf(argv[2], "%g %g", &anchor.x, &anchor.y);
    object->mJointDef.localAnchorB = anchor;
    return true;
}

//-------------------------------------------------------------------------------------
// RevoluteJoint2b ==> b2RevoluteJoint

IMPLEMENT_CONOBJECT(RevoluteJoint2b);

RevoluteJoint2b::~RevoluteJoint2b()
{

    
    if (mJoint) {
        mWorld->DestroyJoint(mJoint);
        mJoint = NULL;
    }
}

ConsoleMethod(RevoluteJoint2b, createJoint, bool, 4, 4, "(World2b world, RevoluteJointDef2b jointdef)"
    "")
{

    World2b* lWorld = (World2b*)Sim::findObject(dAtoi(argv[2]));
    RevoluteJointDef2b* lJointdef = (RevoluteJointDef2b*)Sim::findObject(dAtoi(argv[3]));
    
    object->mWorld = lWorld->mWorld;
    object->mJoint = (b2RevoluteJoint*)object->mWorld->CreateJoint(&lJointdef->mJointDef);

    //userdata 
    object->mJoint->SetUserData(object);

    return true;

}

ConsoleMethod(RevoluteJoint2b, setMotorSpeed, void, 3, 3, "Set the motor speed in radians per second.")
{
    object->mJoint->SetMotorSpeed(dAtof(argv[2]));
}

// FIXME many more stuff to add!!!!

//-----------------------------------------------------------------------------
// Distance Joint
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(DistanceJointDef2b);

void DistanceJointDef2b::initPersistFields()
{
    Parent::initPersistFields();
    addField("collideConnected", TypeBool, Offset(mJointDef.collideConnected, DistanceJointDef2b),
        "Set this flag to true if the attached bodies should collide.");


    addField("length", TypeF32, Offset(mJointDef.length, DistanceJointDef2b),
        "The natural length between the anchor points.");

    addField("minLength", TypeF32, Offset(mJointDef.minLength, DistanceJointDef2b),
        "Minimum length. Clamped to a stable minimum value."
    );
    addField("maxLength", TypeF32, Offset(mJointDef.maxLength, DistanceJointDef2b),
        "Maximum length. Must be greater than or equal to the minimum length.");

    addField("stiffness", TypeF32, Offset(mJointDef.stiffness, DistanceJointDef2b),
        "The linear stiffness in N/m.");

    addField("damping", TypeF32, Offset(mJointDef.damping, DistanceJointDef2b),
        "The linear damping in N*s/m.");
}


/// Initialize the bodies, anchors, and length using the world
/// anchors.
/// void Initialize(b2Body* bodyA, b2Body* bodyB,
///    const b2Vec2& anchorA, const b2Vec2& anchorB);

ConsoleMethod(DistanceJointDef2b, Initialize, bool, 6, 6, "(Body2b bodyA, Body2b bodyB, Point2F anchorA, Point2F anchorB)"
    "Initialize the bodies, anchors, and reference angle using a world")
{
    Body2b* lBodyA = (Body2b*)Sim::findObject(dAtoi(argv[2]));
    Body2b* lBodyB = (Body2b*)Sim::findObject(dAtoi(argv[3]));
    //sanity
    if (!lBodyA || !lBodyB || !lBodyA->mBody || !lBodyB->mBody)
        return false;
    b2Vec2 anchorA, anchorB;
    dSscanf(argv[4], "%g %g", &anchorA.x, &anchorA.y);
    dSscanf(argv[5], "%g %g", &anchorB.x, &anchorB.y);
    anchorA.x /= gB2ratio;
    anchorA.y /= gB2ratio;
    anchorB.x /= gB2ratio;
    anchorB.y /= gB2ratio;


    object->mJointDef.Initialize(lBodyA->mBody, lBodyB->mBody, anchorA, anchorB);
    return true;
}


/// The local anchor point relative to bodyA's origin.
/// b2Vec2 localAnchorA;
ConsoleMethod(DistanceJointDef2b, setlocalAnchorA, bool, 3, 3, "(Point2F anchor)"
    "The local anchor point relative to bodyA's origin.")
{

    b2Vec2 anchor;
    dSscanf(argv[2], "%g %g", &anchor.x, &anchor.y);
    anchor.x /= gB2ratio;
    anchor.y /= gB2ratio;

    object->mJointDef.localAnchorA = anchor;
    return true;
}

/// The local anchor point relative to bodyB's origin.
/// b2Vec2 localAnchorB;
ConsoleMethod(DistanceJointDef2b, setlocalAnchorB, bool, 3, 3, "(Point2F anchor)"
    "The local anchor point relative to bodyB's origin.")
{

    b2Vec2 anchor;
    anchor.x /= gB2ratio;
    anchor.y /= gB2ratio;
    dSscanf(argv[2], "%g %g", &anchor.x, &anchor.y);
    object->mJointDef.localAnchorB = anchor;
    return true;
}
//------------
IMPLEMENT_CONOBJECT(DistanceJoint2b);

DistanceJoint2b::~DistanceJoint2b()
{


    if (mJoint) {
        mWorld->DestroyJoint(mJoint);
        mJoint = NULL;
    }
}

ConsoleMethod(DistanceJoint2b, createJoint, bool, 4, 4, "(World2b world, RevoluteJointDef2b jointdef)"
    "")
{

    World2b* lWorld = (World2b*)Sim::findObject(dAtoi(argv[2]));
    RevoluteJointDef2b* lJointdef = (RevoluteJointDef2b*)Sim::findObject(dAtoi(argv[3]));

    object->mWorld = lWorld->mWorld;
    object->mJoint = (b2DistanceJoint*)object->mWorld->CreateJoint(&lJointdef->mJointDef);

    //userdata 
    object->mJoint->SetUserData(object);

    return true;

}
/// Get the rest length
/// float GetLength() const { return m_length; }
ConsoleMethod(DistanceJoint2b, getLength, F32, 2, 2, "Get the rest length")
{
    return object->mJoint->GetLength();
}

/// Set the rest length
/// @returns clamped rest length
/// float SetLength(float length);
ConsoleMethod(DistanceJoint2b, setLength, F32, 3, 3, 
    "set the rest length"
    "@returns clamped rest length")
{
    return object->mJoint->SetLength(dAtof(argv[2]));
}



/// Get the minimum length
/// float GetMinLength() const { return m_minLength; }
ConsoleMethod(DistanceJoint2b, getMinLength, F32, 2, 2, "Get the minimum length")
{
    return object->mJoint->GetMinLength();
}

/// Set the minimum length
/// @returns the clamped minimum length
/// float SetMinLength(float minLength);
ConsoleMethod(DistanceJoint2b, setMinLength, F32, 3, 3,
    "Set the minimum length"
    "@returns the clamped minimum length"
)
{
    return object->mJoint->SetMinLength(dAtof(argv[2]));
}

/// Get the maximum length
/// float GetMaxLength() const { return m_maxLength; }
ConsoleMethod(DistanceJoint2b, getMaxLength, F32, 2, 2, "Get the maximum length")
{
    return object->mJoint->GetMaxLength();
}

/// Set the maximum length
/// @returns the clamped maximum length
/// float SetMaxLength(float maxLength);
ConsoleMethod(DistanceJoint2b, setMaxLength, F32, 3, 3, 
    "Set the maximum length"
    "@returns the clamped maximum length"
)
{
    return object->mJoint->SetMaxLength(dAtof(argv[2]));
}

/// Get the current length
/// float GetCurrentLength() const;
ConsoleMethod(DistanceJoint2b, getCurrentLength, F32, 2, 2, "Get the current length")
{
    return object->mJoint->GetCurrentLength();
}

/// Set/get the linear stiffness in N/m
/// void SetStiffness(float stiffness) { m_stiffness = stiffness; }
/// float GetStiffness() const { return m_stiffness; }

ConsoleMethod(DistanceJoint2b, setStiffness, void, 3, 3,
    "Set the linear stiffness in N/m"
)
{
    object->mJoint->SetStiffness(dAtof(argv[2]));
}

ConsoleMethod(DistanceJoint2b, getStiffness, F32, 2, 2, "get the linear stiffness in N/m")
{
    return object->mJoint->GetStiffness();
}

/// Set/get linear damping in N*s/m
/// void SetDamping(float damping) { m_damping = damping; }
/// float GetDamping() const { return m_damping; }
ConsoleMethod(DistanceJoint2b, getDamping, F32, 2, 2, "get the linear damping in N/m")
{
    return object->mJoint->GetDamping();
}
ConsoleMethod(DistanceJoint2b, setDamping, void, 3, 3,
    "Set linear damping in N*s/m"
)
{
    object->mJoint->SetDamping(dAtof(argv[2]));
}
