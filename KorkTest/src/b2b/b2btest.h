#pragma once

#include "sim/simBase.h"
#include "console/consoleTypes.h"
#include "core/fluxGlobals.h"
#include "core/fluxBaseObject.h"
#include "Box2D/Box2D.h"



namespace b2bTest
{

	class TestWorld : public SimObject, public FluxBaseObject
	{
	private:
		typedef SimObject Parent;
	
	protected:

		F32 mX = 0.f;
		F32 mY = 0.f;
		F32 mW = 200.f;
		F32 mH = 200.f;
		F32 mGroundGravity = 9.81f;

		b2Vec2 mGravity;
		b2World* mWorld = nullptr;
		//b2BodyDef mGroundBodyDef;
		//b2Body* mGroundBody;
		//b2PolygonShape mGroundBox;
		b2BodyDef mBodyDef;
		b2Body* mBody;
		b2PolygonShape mDynamicBox;
		b2FixtureDef mFixtureDef;


		b2CircleShape mBallShape;
		b2Body* mBallBody;


	public:
		DECLARE_CONOBJECT(TestWorld);

		virtual void Update(const double& dt) override;
		virtual void Draw() override;;

		
		static void initPersistFields();
		bool onAdd() override;
		void onRemove() override;

	};



}

