// ------------------------------------------------------------------------------
// Physics.cpp - Implements the IGamePhysics interface with the Bullet library
// ------------------------------------------------------------------------------

#include "../Common/CommonStd.h"
#include "../App/App.h"
#include "Physics.h"
#include "../Actors/Actor.h"
#include "../Actors/TransformComponent.h"
#include "../ResourceCache/XmlResource.h"
#include "../EventManager/EventManager.h"

// ==============================================================
// g_Materials Description
//
// Predefines some useful physics materials. Define new ones here,
// and have similar objects use it, so if you ever need to change
// it you'll only have to change it here.
//
// ==============================================================
struct MaterialData
{
	float m_restitution;
	float m_friction;

	MaterialData(float restitution, float friction)
	{
		m_restitution = restitution;
		m_friction = friction;
	}

	MaterialData(const MaterialData& other)
	{
		m_restitution = other.m_restitution;
		m_friction = other.m_friction;
	}
};

// ==============================================================
// a physics implementation which does nothing. used if physics is disabled
//
// ==============================================================
class NullPhysics : public IGamePhysics
{

};

#ifndef DISABLE_PHYSICS

#include "../Graphics3D/Geometry.h"
#include "../EventManager/Events.h"

//#include "PhysicsDebugDrawer.h"
//#include "PhysicsEventListener.h"

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include <set>
#include <iterator>
#include <map>

// =================================================================================
// helpers for conversion to and from Bullet's data types

static btVector3 Vec3_to_btVector3(Vec3 const& vec3)
{
	return btVector3(vec3.x, vec3.y, vec3.z);
}

static Vec3 btVector3_to_Vec3(btVector3 const& btvec)
{
	return Vec3(btvec.x(), btvec.y(), btvec.z());
}


static btTransform Mat4x4_to_btTransform(Mat4x4 const & mat)
{
	// converts from Mat4x4 (Nova Engine) to btTransform (Bullet)
	btMatrix3x3 bulletRotation;
	btVector3 bulletPosition;

	// copy rotation matrix
	for (int row = 0; row < 3; ++row) {
		for (int column = 0; column < 3; ++column) {
			bulletRotation[row][column] = mat.m[column][row];
			// note the reversed indexing (row/column vs. column/row)
			// this is because Mat4x4s are row-major matrices and
			// btMatrix3x3 are column-major. This reversed indexing
			// implicitly transposes (flips along the diagonal)
			// the matrix when it is copied.
		}
	}
	// copy position
	for (int column = 0; column < 3; ++column) {
		bulletPosition[column] = mat.m[3][column];
	}

	return btTransform(bulletRotation, bulletPosition);
}

static Mat4x4 btTransform_to_Mat4x4(btTransform const & trans)
{
	Mat4x4 returnValue = Mat4x4::g_Identity;

	// convert from btTransform (Bullet) to Mat4x4(Nova Engine)
	btMatrix3x3 const & bulletRotation = trans.getBasis();
	btVector3 const & bulletPosition = trans.getOrigin();

	// copy rotation matrix
	for (int row = 0; row < 3; ++row) {
		for (int column = 0; column < 3; ++column) {
			returnValue.m[row][column] = bulletRotation[column][row];
			// note the reversed indexing (row/column vs. column/row)
			// this is because Mat4x4s are row-major matrices and
			// btMatrices3x3 are column-major. This reversed indexing
			// implicitly transposes (flips along the diagonal) 
			// the matrix when it is copied.
		}
	}
	// copy position
	for (int column = 0; column < 3; ++column) {
		returnValue.m[3][column] = bulletPosition[column];
	}

	return returnValue;
}

// ======================================================================================
// struct ActorMotionState								-Chapter 17, page 597
//
//	Interface that Bullet uses to communicate position and orientation changes back
//		to the game. note: this assumes that the actor's center of mass and world
//		position are the same point. If that was not the case, and additional 
//		transformation would need to be stored here to represent that difference.
//
// ======================================================================================
struct ActorMotionState : public btMotionState
{
	Mat4x4 m_worldToPositionTransform;

	ActorMotionState(Mat4x4 const& startingTransform)
		: m_worldToPositionTransform(startingTransform) { }

	// btMotionState interface: Bullet calls these
	virtual void getWorldTransform(btTransform& worldTrans) const
	{
		worldTrans = Mat4x4_to_btTransform(m_worldToPositionTransform);
	}

	virtual void setWorldTransform(const btTransform& worldTrans)
	{
		m_worldToPositionTransform = btTransform_to_Mat4x4(worldTrans);
	}
};





// ======================================================================================
// 
// BulletPhysics								-Chapter 17, page 590
//
//		The implementation of IGamePhysics interface using the Bullet SDK.
//
// ======================================================================================
class BulletPhysics : public IGamePhysics, Nv_noncopyable
{
	// use auto pointers to automatically call delete on these objects
	//	during ~BulletPhysics

	// these are all of the objects that Bullet uses to do its work.
	//  see BulletPhysics::VInitialize() for more info.
	btDynamicsWorld*							m_dynamicsWorld;
	btBroadphaseInterface*						m_broadphase;
	btCollisionDispatcher*						m_dispatcher;
	btConstraintSolver*							m_solver;
	btDefaultCollisionConfiguration*				m_collisionConfiguration;
	//BulletDebugDrawer*						m_debugDrawer;

	// tables read from the XML
	typedef std::map<std::string, float> DensityTable;
	typedef std::map<std::string, MaterialData> MaterialTable;
	DensityTable m_densityTable;
	MaterialTable m_materialTable;

	void LoadXml();
	float LookupSpecificGravity(const std::string& densityStr);
	MaterialData LookupMaterialData(const std::string& materialStr);


	// keep track of the existing rigid bodies: To check them for updates
	// to the actors' positions and to remove them when their lifes are over.
	typedef std::map<ActorId, btRigidBody*> ActorIDToBulletRigidBodyMap;
	ActorIDToBulletRigidBodyMap m_actorIdToRigidBody;
	btRigidBody* FindBulletRigidBody(ActorId id) const;

	// also keep a map to get the actor id from the btRigidBody*
	typedef std::map<btRigidBody const *, ActorId> BulletRigidBodyToActorIDMap;
	BulletRigidBodyToActorIDMap m_rigidBodyToActorId;
	ActorId FindActorID(btRigidBody const *) const;

	// data used to store which collision pair (bodies that are touching) need
	//  Collision events sent. When a new pair of touching bodies are detected,
	//  they are added to m_previousTickCollisionPairs and an event is sent.
	//  When the pair is no longer detected, they are removed and another event
	//  is sent.
	typedef std::pair<btRigidBody const *, btRigidBody const *> CollisionPair;
	typedef std::set<CollisionPair> CollisionPairs;
	CollisionPairs m_previousTickCollisionPairs;

	// helpers for sending events relating to collision pairs.
	void SendCollisionPairAddEvent(btPersistentManifold const * manifold, btRigidBody const * body0, btRigidBody const * body1);
	void SendCollisionPairRemoveEvent(btRigidBody const * body0, btRigidBody const * body1);

	// common functionality used by VAddSphere, VAddBox, etc.
	void AddShape(StrongActorPtr pGameActor, btCollisionShape* shape, float mass, const std::string& physicsMaterial);

	// helper for cleaning up objects
	void RemoveCollisionObject(btCollisionObject* removeMe);

	// callback from bullet for each physics time step. set in VInitialize
	static void BulletInternalTickCallback(btDynamicsWorld* const world, btScalar const timeStep);

public:
	BulletPhysics();			// [mrmike] This was changed post-press to add event registration!
	virtual ~BulletPhysics();

	// Initialization and Maintenance of the Physics World
	virtual bool VInitialize() override;
	virtual void VSyncVisibleScene() override;
	virtual void VOnUpdate(float deltaSeconds) override;

	// Initialization of Physics Objects
	virtual void VAddSphere(float radius, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial) override;
	virtual void VAddBox(const Vec3& dimensions, WeakActorPtr pGameActor, const std::string& desnsityStr, const std::string& physicsMaterial) override;
	virtual void VAddPointCloud(Vec3* verts, int numPoints, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial) override;
	virtual void VRemoveActor(ActorId id) override;

	// Debugging 
	virtual void VRenderDiagnostics() override;
	
	virtual void VCreateTrigger(WeakActorPtr pGameActor, const Vec3& pos, const float dim) override;
	virtual void VApplyForce(const Vec3& dir, float newtons, ActorId aid) override;
	virtual void VApplyTorque(const Vec3& dir, float newtons, ActorId aid) override;
	virtual bool VKinematicMove(const Mat4x4& mat, ActorId aid) override;

	virtual void VRotateY(ActorId actorId, float angleRadians, float time);
	virtual float VGetOrientationY(ActorId actorId);
	virtual void VStopActor(ActorId actorId);
	virtual Vec3 VGetVelocity(ActorId actorId);
	virtual void VSetVelocity(ActorId actorId, const Vec3& vel);
	virtual Vec3 VGetAngularVelocity(ActorId actorId);
	virtual void VSetAngularVelocity(ActorId actorId, const Vec3& vel);
	virtual void VTranslate(ActorId actorId, const Vec3& vec);

	virtual void VSetTransform(const ActorId id, const Mat4x4& mat);

	virtual Mat4x4 VGetTransform(const ActorId id);
};

BulletPhysics::BulletPhysics()
{
	// [mrmike] This was changed pos-press to add event registration!
	REGISTER_EVENT(EvtData_PhysTrigger_Enter);
	REGISTER_EVENT(EvtData_PhysTrigger_Leave);
	REGISTER_EVENT(EvtData_PhysCollision);
	REGISTER_EVENT(EvtData_PhysSeparation);
}

// ============================================================================
// BulletPhysics::~BulletPhysics()					- Chapter 16, page 596
// ============================================================================
BulletPhysics::~BulletPhysics()
{
	// delete any physics objects which are still in the world

	// iterate backwards because removing the last object doesn't affect the
	// other objects stored in a vector-type array.
	for (int ii = m_dynamicsWorld->getNumCollisionObjects() - 1; ii >= 0; --ii)
	{
		btCollisionObject* const obj = m_dynamicsWorld->getCollisionObjectArray()[ii];

		RemoveCollisionObject(obj);
	}

	m_rigidBodyToActorId.clear();

	//SAFE_DELETE(m_debugDrawer);
	SAFE_DELETE(m_dynamicsWorld);
	SAFE_DELETE(m_solver);
	SAFE_DELETE(m_broadphase);
	SAFE_DELETE(m_dispatcher);
	SAFE_DELETE(m_collisionConfiguration);
}

// ==============================================================================
// BulletPhysics::LoadXml							- not described in the book
// 
//		Loads the physics materials from an XML file.
//
// ==============================================================================
void BulletPhysics::LoadXml()
{
	// Load the physics config file and grab the root XML node
	TiXmlElement* pRoot = XmlResourceLoader::LoadAndReturnRootXmlElement("config\\Physics.xml");
	//Nv_ASSERT(pRoot);

	// load all the materials
	TiXmlElement* pParentNode = pRoot->FirstChildElement("PhysicsMaterials");
	//Nv_ASSERT(pParentNode);
	for (TiXmlElement* pNode = pParentNode->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		double restitution = 0;
		double friction = 0;
		pNode->Attribute("restitution", &restitution);
		pNode->Attribute("friction", &friction);
		m_materialTable.insert(std::make_pair(pNode->Value(), MaterialData((float)restitution, (float)friction)));
	}

	// load all densities
	pParentNode = pRoot->FirstChildElement("DensityTable");
	//Nv_ASSERT(pParentNode);
	for (TiXmlElement* pNode = pParentNode->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		m_densityTable.insert(std::make_pair(pNode->Value(), (float)atof(pNode->FirstChild()->Value())));
	}
}

// ==============================================================================
// BulletPhysics::VInitialize							- Chapter 17, page 594
// ==============================================================================
bool BulletPhysics::VInitialize()
{
	LoadXml();

	// this controls how Bullet does internal memory management during the collision pass
	m_collisionConfiguration = Nv_NEW btDefaultCollisionConfiguration();

	// this manages how Bullet detects precise collisions between pairs of objects
	m_dispatcher = Nv_NEW btCollisionDispatcher(m_collisionConfiguration);

	// Bullet uses this to quickly (imprecisely) detect collisions between objects.
	//	Once a possible collision passes the broad range, it will be passed to the
	//  slower but more precise narrow-phase collision detection (btCollisionDispatcher)
	m_broadphase = Nv_NEW btDbvtBroadphase();

	// Manages constraints which apply forces to the physics simulation. Used
	// for e.g springs motors. We don't use any constraints right now.
	m_solver = Nv_NEW btSequentialImpulseConstraintSolver;

	// This is the main Bullet interface point. Pass in all these components to customize its behavior.
	m_dynamicsWorld = Nv_NEW btDiscreteDynamicsWorld(m_dispatcher, 
														m_broadphase, 
														m_solver, 
														m_collisionConfiguration);

	m_debugDrawer = Nv_NEW BulletDebugDrawer;
	m_debugDrawer->ReadOptions();

	if (!m_collisionConfiguration || !m_dispatcher || !m_broadphase ||
		!m_solver || !m_dynamicsWorld || !m_debugDrawer)
	{
		//Nv_ERROR("BulletPhysics::VInitialize failed!");
		return false;
	}

	m_dynamicsWorld->setDebugDrawer(m_debugDrawer);

	// and set the internal tick callback to our own method "BulletInternalTickCallback"
	m_dynamicsWorld->setInternalTickCallback(BulletInternalTickCallback);
	m_dynamicsWorld->setWorldUserInfo(this);

	return true;
}

// ==============================================================================
// BulletPhysics::VOnUpdate							- Chapter 17, page 596
// ==============================================================================
void BulletPhysics::VOnUpdate(float const deltaSeconds)
{
	// Bullet uses an internal fixed timestep (default 1/60th of a second)
	//  We pass in 4 as a max number of sub steps. Bullet will run the simulation
	//	in increments of the fixed timestep until "deltaSeconds" amount of time has
	//  passed, but will only run a maximum of 4 steps this way.
	m_dynamicsWorld->stepSimulation(deltaSeconds, 4);
}

// ==============================================================================
// BulletPhysics::VSyncVisibleScene						- Chapter 17, page 598
// ==============================================================================
void BulletPhysics::VSyncVisibleScene()
{
	// Keep physics & graphics in sync.

	// check all the existing actor's bodies for changes.
	// If there is a change, send the appropriate event for the game system.
	for (ActorIDToBulletRigidBodyMap::const_iterator it = m_actorIdToRigidBody.begin();
		it != m_actorIdToRigidBody.end();
		++it)
	{
		ActorId const id = it->first;

		// get the MotionState. this object is updated by Bullet.
		// it's safe to cast the btMotionState to ActorMotionState, because all the bodies in the m_actorIdToRigidBody
		//	were created through AddShape()
		ActorMotionState const* const actorMotionState = static_cast<ActorMotionState*>(it->second->getMotionState());
		//Nv_ASSERT(actorMotionState);

		StrongActorPtr pGameActor = MakeStrongPtr(g_pApp->m_pGame->VGetActor(id));
		if (pGameActor && actorMotionState)
		{
			std::shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(pGameActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
			if (pTransformComponent)
			{
				if (pTransformComponent->GetTransform() != actorMotionState->m_worldToPositionTransform)
				{
					// Bullet has moved the actor's physics object. Sync the transform and inform the game an actor has moved.
					pTransformComponent->SetTransform(actorMotionState->m_worldToPositionTransform);
					std::shared_ptr<EvtData_Move_Actor> pEvent(Nv_NEW EvtData_Move_Actor(id, actorMotionState->m_worldToPositionTransform));
					IEventManager::Get()->VQueueEvent(pEvent);
				}
			}
		}
	}
}

// ==============================================================================
// BulletPhysics::AddShape						- Chapter 17, page 600
// ==============================================================================
void BulletPhysics::AddShape(StrongActorPtr pGameActor, btCollisionShape* shape, float mass, const std::string& physicsMaterial)
{
	//Nv_ASSERT(pGameActor);

	ActorId actorID = pGameActor->GetId();
	//Nv_ASSERT(m_actorIdToRigidBody.find(actorID) == m_actorIdToRigidBody.end() && "Actor with more than one physics body?");

	// lookup the material
	MaterialData material(LookupMaterialData(physicsMaterial));

	// localIntertia defines how the object's mass is distributed
	btVector3 localInertia(0.f, 0.f, 0.f);
	if (mass > 0.f)
	{
		shape->calculateLocalInertia(mass, localInertia);
	}

	Mat4x4 transform = Mat4x4::g_Identity;
	std::shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(pGameActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
	//Nv_ASSERT(pTransformComponent);
	if (pTransformComponent) {
		transform = pTransformComponent->GetTransform();
	}
	else
	{
		// Physics can't work on an actor that doesn't have a TransformComponent!
		return;
	}

	// set the initial transform of the body from the actor
	ActorMotionState* const myMotionState = Nv_NEW ActorMotionState(transform);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);

	// set up the material properties
	rbInfo.m_restitution = material.m_restitution;
	rbInfo.m_friction = material.m_friction;

	btRigidBody * const body = new btRigidBody(rbInfo);

	m_dynamicsWorld->addRigidBody(body);

	// add it to the collection to be checked for changes in VSyncVisibleScene
	m_actorIdToRigidBody[actorID] = body;
	m_rigidBodyToActorId[body] = actorID;
}

// ==============================================================================
// BulletPhysics::RemoveCollisionObject				- not described in the book
//
// Removes a collision object from the game world
//
// ==============================================================================
void BulletPhysics::RemoveCollisionObject(btCollisionObject* const removeMe)
{
	// first remove the object from the physics sim.
	m_dynamicsWorld->removeCollisionObject(removeMe);

	// then remove the pointer from the ongoing contacts list.
	for (CollisionPairs::iterator it = m_previousTickCollisionPairs.begin();
		it != m_previousTickCollisionPairs.end(); )
	{
		CollisionPairs::iterator next = it;
		++next;

		if (it->first == removeMe || it->second == removeMe)
		{
			SendCollisionPairRemoveEvent(it->first, it->second);
			m_previousTickCollisionPairs.erase(it);
		}

		it = next;
	}

	// if the object is a RigidBody (all of ours are RigidBodies, but it's good to be safe)
	if (btRigidBody * const body = btRigidBody::upcast(removeMe))
	{
		// delete the components of the object
		delete body->getMotionState();
		delete body->getCollisionShape();
		delete body->getUserPointer();
		//delete body->getUserPointer();

		for (int ii = body->getNumConstraintRefs() - 1; ii >= 0; --ii)
		{
			btTypedConstraint* const constraint = body->getConstraintRef(ii);
			m_dynamicsWorld->removeConstraint(constraint);
			delete constraint;
		}
	}

	delete removeMe;
}

// ==============================================================================
// BulletPhysics::FindBulletRigidBody				- not described in the book
//		Finds a Bullet rigid body given an actor ID.
// ==============================================================================
btRigidBody* BulletPhysics::FindBulletRigidBody(ActorId const id) const
{
	ActorIDToBulletRigidBodyMap::const_iterator found = m_actorIdToRigidBody.find(id);
	if (found != m_actorIdToRigidBody.end()) {
		return found->second;
	}

	return NULL;
}

// ==============================================================================
// BulletPhysics::FindActorID						- not described in the book
//		Finds an Actor ID given a Bullet rigid body
// ==============================================================================
ActorId BulletPhysics::FindActorID(btRigidBody const * const body) const
{
	BulletRigidBodyToActorIDMap::const_iterator found = m_rigidBodyToActorId.find(body);
	if (found != m_rigidBodyToActorId.end()) {
		return found->second;
	}

	return ActorId();
}

// ==============================================================================
// BulletPhysics::VAddSphere						- Chapter 17, page 599
// ==============================================================================
void BulletPhysics::VAddSphere(float radius, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial)
{
	StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
	if (!pStrongActor) {
		return; // FUTURE WORK - Add a call to the error log here.
	}

	// create the collision body, which specifies the shape of the object
	btSphereShape* const collisionShape = new btSphereShape(radius);

	// calculate absolute mass from specificGravity
	float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = (4.f / 3.f) * Nv_PI * radius * radius * radius;
	btScalar const mass = volume * specificGravity;

	AddShape(pStrongActor, /* initialTransform, */ collisionShape, mass, physicsMaterial);
}

// ==============================================================================
// BulletPhysics::VAddBox						
// ==============================================================================
void BulletPhysics::VAddBox(const Vec3& dimensions, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial)
{
	StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
	if (!pStrongActor) {
		return; // FUTURE WORK: Add a call to the error log here
	}

	// create the collision body, which specifies the shape of the object
	btBoxShape* const boxShape = new btBoxShape(Vec3_to_btVector3(dimensions));

	// calculate absolute mass from specificGravity
	float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = dimensions.x * dimensions.y * dimensions.z;
	btScalar const mass = volume * specificGravity;

	AddShape(pStrongActor, /* initialTransform */ boxShape, mass, physicsMaterial);
}

// ==============================================================================
// BulletPhysics::VAddPointCloud 						
// ==============================================================================
void BulletPhysics::VAddPointCloud(Vec3* verts, int numPoints, WeakActorPtr pGameActor, /* const Mat4x4& initialTransform, */ const std::string& densityStr, const std::string& physicsMaterial)
{
	StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
	if (!pStrongActor) {
		return;	// FUTURE WORK: Add a call to the error log here.
	}

	btConvexHullShape* const shape = new btConvexHullShape();

	// add the points to the shape one at a time
	for (int ii = 0; ii < numPoints; ++ii)
	{
		shape->addPoint(Vec3_to_btVector3(verts[ii]));
	}

	//approximate absolute mass using bounding box
	btVector3 aabbMin(0, 0, 0), aabbMax(0, 0, 0);
	shape->getAabb(btTransform::getIdentity(), aabbMin, aabbMax);

	btVector3 const aabbExtents = aabbMax - aabbMin;

	float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = aabbExtents.x() * aabbExtents.y() * aabbExtents.z();
	btScalar const mass = volume * specificGravity;

	AddShape(pStrongActor, shape, mass, physicsMaterial);
}

// ==============================================================================
// BulletPhysics::VRemoveActor						-not described in the book 						
//
//		Implements the method to remove actors from the physics simulation
//
// ==============================================================================
void BulletPhysics::VRemoveActor(ActorId id)
{
	if (btRigidBody* const body = FindBulletRigidBody(id))
	{
		// destroy the body and all its components
		RemoveCollisionObject(body);
		m_actorIdToRigidBody.erase(id);
		m_rigidBodyToActorId.erase(body);
	}
}

// ==============================================================================
// BulletPhysics::VRenderDiagnostics					- Chapter 17, page 604 						
// ==============================================================================
void BulletPhysics::VRenderDiagnostics()
{
	m_dynamicsWorld->debugDrawWorld();
}

// ==============================================================================
// BulletPhysics::VCreateTrigger						- Chapter 17, page 602
//
// FUTURE WORK: Mike create a trigger actor archetype that can be instantiated in the editor!!!
//
// ==============================================================================
void BulletPhysics::VCreateTrigger(WeakActorPtr pGameActor, const Vec3& pos, const float dim)
{
	StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
	if (!pStrongActor) {
		return; // FUTURE WORK: Add a call to the error log here
	}

	// Create the collision body, which specifies the shape of the object
	btBoxShape* const boxShape = new btBoxShape(Vec3_to_btVector3(Vec3(dim, dim, dim)));

	// triggers are immoveable. 0 mass signals this to Bullet.
	btScalar const mass = 0;

	// set the initial position of the body from the actor
	Mat4x4 triggerTrans = Mat4x4::g_Identity;
	triggerTrans.SetPosition(pos);
	ActorMotionState* const myMotionState = Nv_NEW ActorMotionState(triggerTrans);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, boxShape, btVector3(0, 0, 0));
	btRigidBody * const body = new btRigidBody(rbInfo);

	m_dynamicsWorld->addRigidBody(body);

	// a trigger is just a box that doesn't collide with anything. That's what "CF_NO_CONTACT_RESPONSE" indicates.
	body->setCollisionFlags(body->getCollisionFlags() | btRigidBody::CF_NO_CONTACT_RESPONSE);

	m_actorIdToRigidBody[pStrongActor->GetId()] = body;
	m_rigidBodyToActorId[body] = pStrongActor->GetId();
}




#endif



// ==============================================================
//
//	CreateGamePhysics
//		The free function that creates an object that implements
//		the IGamePhysics interface.
//
// ==============================================================
IGamePhysics* CreateGamePhysics
{

}

IGamePhysics* CreateNullPhysics()
{

}