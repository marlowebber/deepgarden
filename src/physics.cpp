#include "physics.h"
#include "graphics.h"
#include "menus.h"
#include "main.h"

#include <ctime>
#include <chrono>
#include <iostream>

#include <box2d.h>


b2World * m_world = nullptr;

b2MouseJoint* m_mouseJoint;
b2Body * mouseDraggingBody;
b2BodyDef groundBodyDef;
b2Body * m_groundBody;





class MyDestructionListener : public b2DestructionListener
{
	void SayGoodbye(b2Joint* joint)
	{
		// remove all references to joint.
		if (m_mouseJoint == joint)
		{
			m_mouseJoint = NULL;
		}
	}
};

b2DestructionListener * myDestructionListener;

void setupForMouseJoint()
{
	m_groundBody = m_world->CreateBody(&groundBodyDef);
	m_world->SetDestructionListener(myDestructionListener);
}

void destroyMouseJoint ()
{
	if (m_mouseJoint)
	{
		printf("destroyMouseJoint\n");
		m_world->DestroyJoint(m_mouseJoint);
		m_mouseJoint = nullptr;
		mouseDraggingBody = nullptr;
	}
}

void maintainMouseJoint (b2Vec2 p)
{
	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(p);
		m_groundBody->SetTransform(p, 0.0f);
	}
}

bool getMouseJointStatus ()
{
	if (m_mouseJoint)
	{
		return true;
	}
	return false;
}

void initMouseJointWithBody (b2Vec2 p, b2Body * body)
{
	if (m_mouseJoint != NULL)
	{
		return;
	}

	b2MouseJointDef md;
	md.bodyA = m_groundBody;
	md.bodyB = body;
	md.target = p;
	md.maxForce = 10000.0f * body->GetMass();
	m_mouseJoint = (b2MouseJoint*)m_world->CreateJoint(&md);
	body->SetAwake(true);

	m_mouseJoint->SetStiffness(10000.00f);
	m_mouseJoint->SetDamping(100.00f);

	mouseDraggingBody = body;

	printf("mouse joint\n");

	return;
}


// this class pins together the parts you need for a box2d physical-world object.
// if you make your own classes that represent physical objects, you should either have them inherit from this, or have a copy of one of these as a member.
PhysicalObject::
PhysicalObject (std::vector<b2Vec2>   vertices, bool flagStatic)
{

	this->owner = nullptr;

	// this->flagReady = false;
	this->flagDelete = false;
	this->fraction = 0;

	this->jointDef = b2RevoluteJointDef();
	this->p_joint = nullptr;

	// this->jointDef =  b2RevoluteJointDef();
	this->bodyDef = b2BodyDef();
	this->bodyDef.userData = b2BodyUserData();

	this->vertices = vertices;

	if (flagStatic)
	{
		this->bodyDef.type = b2_staticBody;
	}
	else
	{
		this->bodyDef.type = b2_dynamicBody;
	}

	this->shape.Set(this->vertices.data(), this->vertices.size());

	this->color = b2Color (0.1f, 0.1f, 0.1f, 1.0f);
}

std::list<b2Body* > rayContacts;
std::list<PhysicalObject*> physicalObjects;

int checkClickObjects (b2Vec2 worldClick)
{


	printf("testing world position %f %f\n", worldClick.x, worldClick.y);
	std::list<PhysicalObject*>::iterator object;
	for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	{

		if ((*object)->p_fixture->TestPoint( worldClick) )
		{

			initMouseJointWithBody (worldClick, (*object)->p_body);
			return 1;
		}

	}

	return 0;
}


void createJoint(PhysicalObject * a, PhysicalObject * b)
{

	printf("create joint\n");
	a->jointDef =  b2RevoluteJointDef();
	a->jointDef.collideConnected = false; // this means that limb segments dont collide with their children
	a->jointDef.bodyA = a->p_body;
	a->jointDef.bodyB = b->p_body;
	a->jointDef.localAnchorA = b2Vec2( 0.0f,   -1 * (lengthCursor / 2) );
	a->jointDef.localAnchorB = b2Vec2( 0.0f,   1  * (lengthCursor / 2) );
	a->jointDef.enableMotor = true;
	a->jointDef.maxMotorTorque = 10000.0f;
	a->p_joint = (b2RevoluteJoint *)(m_world->CreateJoint( &(a->jointDef) ));

}


void createJointWithVariableBAnchor(PhysicalObject * a, PhysicalObject * b, b2Vec2 positionOnB)
{

	printf("create joint\n");
	a->jointDef =  b2RevoluteJointDef();
	a->jointDef.collideConnected = false; // this means that limb segments dont collide with their children
	a->jointDef.bodyA = a->p_body;
	a->jointDef.bodyB = b->p_body;
	a->jointDef.localAnchorA = b2Vec2( 0.0f,   -1 * (lengthCursor / 2) );
	a->jointDef.localAnchorB = positionOnB;//b2Vec2( 0.0f,   1 * (lengthCursor / 2) );
	a->jointDef.enableMotor = true;
	a->jointDef.maxMotorTorque = 10000.0f;
	a->p_joint = (b2RevoluteJoint *)m_world->CreateJoint( &(a->jointDef) );

}

void collisionHandler (b2Contact * contact)
{

	// printf("pliope\n");
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	uDataWrap * userDataA = (uDataWrap*)((fixtureA->GetBody())->GetUserData().pointer);
	uDataWrap * userDataB = (uDataWrap*)((fixtureB->GetBody())->GetUserData().pointer);

	if (userDataA != nullptr)
	{

		// printf("pencoiec  %u a\n", userDataA->dataType);
		if (userDataA->dataType == TYPE_BRANCH)
		{

			// printf("a\n");


			if ( ((PhysicalObject *)(userDataA->uData)) != nullptr  )
			{

				// printf("b\n");
				if ( ((PhysicalObject *)(userDataA->uData))->owner != nullptr)
				{

					// printf("c\n");
					if (((PhysicalObject *)(userDataA->uData))->owner->owner != nullptr)
					{

						// printf("d\n");

						((PhysicalObject *)(userDataA->uData))->owner->owner->mature  = true;
						((PhysicalObject *)(userDataA->uData))->owner->owner->sproutPosition = ((PhysicalObject *)(userDataA->uData))->owner->object.p_body->GetWorldCenter();

						if (userDataB != nullptr)
						{

							// printf("vnevtfic\n");
							// if (userDataB->dataType == TYPE_BRANCH)
							// {

							// printf("e\n");

							if ( ((PhysicalObject *)(userDataB->uData)) != nullptr  )
							{

								// printf("f\n");
								// if ( ((PhysicalObject *)(userDataB->uData))->owner != nullptr)
								// {

								// 	// printf("g\n");
								// 	if (((PhysicalObject *)(userDataB->uData))->owner->owner != nullptr)
								// 	{


								// printf("h\n");
								// ((PhysicalObject *)(userDataB->uData))->owner->owner->mature  = true;
								// ((PhysicalObject *)(userDataB->uData))->owner->owner->sproutPosition = ((PhysicalObject *)(userDataB->uData))->owner->GetWorldCenter();

								((PhysicalObject *)(userDataA->uData))->owner->owner->affixedObject = ((PhysicalObject *)(userDataB->uData));

								// 	}
								// }

							}
							// }

						}


					}
				}

			}


		}

	}

	if (userDataB != nullptr)
	{

		// printf("vnevtfic\n");
		if (userDataB->dataType == TYPE_BRANCH)
		{

			// printf("e\n");

			if ( ((PhysicalObject *)(userDataB->uData)) != nullptr  )
			{

				// printf("f\n");
				if ( ((PhysicalObject *)(userDataB->uData))->owner != nullptr)
				{

					// printf("g\n");
					if (((PhysicalObject *)(userDataB->uData))->owner->owner != nullptr)
					{


						// printf("h\n");
						((PhysicalObject *)(userDataB->uData))->owner->owner->mature  = true;
						((PhysicalObject *)(userDataB->uData))->owner->owner->sproutPosition = ((PhysicalObject *)(userDataB->uData))->owner->object.p_body->GetWorldCenter();

						if (userDataA != nullptr)
						{

							// printf("pencoiec  %u a\n", userDataA->dataType);
							// if (userDataA->dataType == TYPE_BRANCH)
							// {

							// printf("a\n");


							if ( ((PhysicalObject *)(userDataA->uData)) != nullptr  )
							{

								// printf("b\n");
								// if ( ((PhysicalObject *)(userDataA->uData))->owner != nullptr)
								// {

								// 	// printf("c\n");
								// 	if (((PhysicalObject *)(userDataA->uData))->owner->owner != nullptr)
								// 	{

								// printf("d\n");

								((PhysicalObject *)(userDataB->uData))->owner->owner->affixedObject = ((PhysicalObject *)(userDataA->uData));
								// ((PhysicalObject *)(userDataA->uData))->owner->owner->mature  = true;
								// ((PhysicalObject *)(userDataA->uData))->owner->owner->sproutPosition = ((PhysicalObject *)(userDataA->uData))->owner->GetWorldCenter();

								// 	}
								// }

							}


							// }

						}

					}
				}

			}
		}

	}


}

class MyListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact)
	{
		const b2Manifold* manifold = contact->GetManifold();

		if (manifold->pointCount == 0)
		{
			return;
		}

		collisionHandler (contact) ;
	}

	void EndContact(b2Contact* contact)
	{
		;
	}
};

MyListener listener;

class RayCastClosestCallback : public b2RayCastCallback
{
public:
	RayCastClosestCallback()
	{
		m_hit = false;
	}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{

		m_hit = true;
		m_point = point;
		m_normal = normal;

		b2Body* body = fixture->GetBody();

		rayContacts.push_back(body);

		return fraction;
	}

	bool m_hit;
	b2Vec2 m_point;
	b2Vec2 m_normal;
};

void shine (b2Vec2 p1, b2Vec2 p2)
{
	b2RayCastInput sunbeam;

	sunbeam.maxFraction = 1.0f;

	rayContacts.clear();

	sunbeam.p1 = p1;
	sunbeam.p2 = p2;

	RayCastClosestCallback jeremiahSnailMan;
	m_world->RayCast( &jeremiahSnailMan, sunbeam.p1, sunbeam.p2);

	// go through the list of contacts and only flag the closest one.
	std::list<b2Body*>::iterator contact;

	if (rayContacts.size() > 0)
	{
		unsigned int contactIndex = 0;
		for (contact = rayContacts.begin(); contact !=  rayContacts.end(); ++contact)
		{
			// do stuff to them

			uDataWrap * userDataA = (uDataWrap*)((*contact)->GetUserData().pointer);


			if (userDataA != nullptr)
			{

				// printf("vnevtfic\n");
				if (userDataA->dataType == TYPE_BRANCH)
				{

					// printf("e\n");

					if ( ((PhysicalObject *)(userDataA->uData)) != nullptr  )
					{

						// printf("f\n");
						if ( ((PhysicalObject *)(userDataA->uData))->owner != nullptr)
						{

							if ( ! (((PhysicalObject *)(userDataA->uData)) ->owner->seed) )
							{
								// printf("photo hitt\n");
								// ((PhysicalObject *)(userDataA->uData))->owner->capturedLight++;

								// printf("g\n");
								if (((PhysicalObject *)(userDataA->uData))->owner->owner != nullptr)
								{
									((PhysicalObject *)(userDataA->uData))->owner->owner->energyStored += 1.0f;

								}

							}
						}
					}
				}
			}



			contactIndex++;
		}
	}
}

void  addToWorld(PhysicalObject * object, b2Vec2 position, float angle)
{
	// PhysicalObject * pushedObject = &(physicalObjects.back());


	uDataWrap  * p_dataWrapper =  new uDataWrap(object, TYPE_TERRAIN);
	if (  object->bodyDef.type == b2_dynamicBody)
	{
		p_dataWrapper->dataType = TYPE_BRANCH;

	}



	// b2BodyUserData  b2usrdat =  b2BodyUserData();
	// b2usrdat.pointer = (uintptr_t ) (&p_dataWrapper);
	object->bodyDef.userData.pointer =  (uintptr_t ) (p_dataWrapper);






	object->p_body = m_world->CreateBody( &(object->bodyDef) );




	object->p_body ->SetTransform(position, angle);


	object->p_fixture = object->p_body->CreateFixture(&(object->shape), 0.012f);	// this endows the shape with mass and is what adds it to the physical world. // 1.2f is the default density










	b2Filter tempFilter = object->p_fixture->GetFilterData();
	tempFilter.groupIndex = 0;
	tempFilter.maskBits = 0;


	if (  object->bodyDef.type == b2_dynamicBody)
	{


		tempFilter.categoryBits = TYPE_BRANCH;//1 << 1; // i am a..

		// if (m_deepSeaSettings.noClipStatus  )
		// {
		tempFilter.maskBits = TYPE_TERRAIN; //  | 1 << 5 | 1 << 6 | 1 << 7 | tempFilter.maskBits;	// and i collide with
		// }


	}

	else if (object->bodyDef.type == b2_staticBody)
	{
		tempFilter.categoryBits = TYPE_TERRAIN; // i am a..

		// if (m_deepSeaSettings.noClipStatus  )
		// {
		tempFilter.maskBits = TYPE_BRANCH; //1  | 1 << 5 | 1 << 6 | 1 << 7 | tempFilter.maskBits;	// and i collide with
		// }
	}







	object->p_fixture->SetFilterData(tempFilter);




// uDataWrap * p_dataWrapper = new uDataWrap(p_bone, TYPE_FOOD);
	// b2BodyUserData * p_b2usrdat = new b2BodyUserData;
	// p_b2usrdat->pointer = (uintptr_t )p_dataWrapper;

	// // p_bone->p_body->SetUserData((void*)p_b2usrdat);
	// p_bone->bodyDef.userData = *p_b2usrdat;



	// object->flagReady = true;

	physicalObjects.push_back( object  );

	// return pushedObject;
}

void deleteFromWorld (PhysicalObject * object)
{

	// destroy the user data object

	delete (uDataWrap*)(object->p_body->GetUserData().pointer);

	// m_world->DestroyJoint(object->p_joint);
	object->p_body->DestroyFixture(object->p_fixture);
	m_world->DestroyBody(object->p_body); 	// this action is for real, you can't grow it back.

	if (object->p_joint != nullptr)
	{

		m_world->DestroyJoint(object->p_joint);
	}
}

void exampleMenuCallback(void * userData)
{
	printf("a menu was clicked\n");
}

void initializePhysics ()
{
	// The first parts are necessary to set up the game world. Don't change them unless you know what you're doing.

	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	setupExtremelyFastNumberGenerators();
	srand((unsigned int)time(NULL));
	b2Vec2 gravity = b2Vec2(0.0f, -10.0f);
	m_world = new b2World(gravity);
	m_world->SetContactListener(&listener);
	setupForMouseJoint();



}







void threadPhysics ()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	if (!m_world->IsLocked() )
	{


		threadInterface(); // if you do this while the world is unlocked, you can add and remove stuff.

		threadGame();

		float timeStep = nominalFramerate > 0.0f ? 1.0f / nominalFramerate : float(0.0f);
		m_world->Step(timeStep, 1, 1);

	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGame " << elapsed.count() << " microseconds." << std::endl;
#endif
}


void threadGraphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	preDraw();

	prepareForWorldDraw();

	gameGraphics();


	cleanupAfterWorldDraw();
	drawMenus ();
	drawCaptureText ();


	b2Vec2 worldMousePos = transformScreenPositionToWorld( b2Vec2(mouseX, mouseY) );

	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGraphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}
