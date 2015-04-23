// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Physics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Physics/PhysObject.hpp>
#include <Nazara/Physics/Config.hpp>
#include <Nazara/Physics/PhysWorld.hpp>
#include <Newton/Newton.h>
#include <algorithm>
#include <Nazara/Physics/Debug.hpp>

NzPhysObject::NzPhysObject(NzPhysWorld* world, const NzMatrix4f& mat) :
NzPhysObject(world, NzNullGeom::New(), mat)
{
}

NzPhysObject::NzPhysObject(NzPhysWorld* world, NzPhysGeomRef geom, const NzMatrix4f& mat) :
m_matrix(mat),
m_geom(std::move(geom)),
m_forceAccumulator(NzVector3f::Zero()),
m_torqueAccumulator(NzVector3f::Zero()),
m_world(world),
m_gravityFactor(1.f),
m_mass(0.f)
{
	NazaraAssert(m_world, "Invalid world");

	if (!m_geom)
		m_geom = NzNullGeom::New();

	m_body = NewtonCreateDynamicBody(m_world->GetHandle(), m_geom->GetHandle(m_world), m_matrix);
	NewtonBodySetUserData(m_body, this);
}

NzPhysObject::NzPhysObject(const NzPhysObject& object) :
m_matrix(object.m_matrix),
m_geom(object.m_geom),
m_forceAccumulator(NzVector3f::Zero()),
m_torqueAccumulator(NzVector3f::Zero()),
m_world(object.m_world),
m_gravityFactor(object.m_gravityFactor),
m_mass(0.f)
{
	NazaraAssert(m_world, "Invalid world");
	NazaraAssert(m_geom, "Invalid geometry");

	m_body = NewtonCreateDynamicBody(m_world->GetHandle(), m_geom->GetHandle(m_world), m_matrix);
	NewtonBodySetUserData(m_body, this);
	SetMass(object.m_mass);
}

NzPhysObject::NzPhysObject(NzPhysObject&& object) :
m_matrix(std::move(object.m_matrix)),
m_forceAccumulator(std::move(object.m_forceAccumulator)),
m_torqueAccumulator(std::move(object.m_torqueAccumulator)),
m_body(object.m_body),
m_geom(std::move(object.m_geom)),
m_world(object.m_world),
m_gravityFactor(object.m_gravityFactor),
m_mass(object.m_mass)
{
	object.m_body = nullptr;
}

NzPhysObject::~NzPhysObject()
{
	if (m_body)
		NewtonDestroyBody(m_world->GetHandle(), m_body);
}

void NzPhysObject::AddForce(const NzVector3f& force, nzCoordSys coordSys)
{
	switch (coordSys)
	{
		case nzCoordSys_Global:
			m_forceAccumulator += force;
			break;

		case nzCoordSys_Local:
			m_forceAccumulator += GetRotation()*force;
			break;
	}

	// On réveille le corps pour que le callback soit appelé et que les forces soient appliquées
	NewtonBodySetSleepState(m_body, 0);
}

void NzPhysObject::AddForce(const NzVector3f& force, const NzVector3f& point, nzCoordSys coordSys)
{
	switch (coordSys)
	{
		case nzCoordSys_Global:
			m_forceAccumulator += force;
			m_torqueAccumulator += NzVector3f::CrossProduct(point - GetMassCenter(nzCoordSys_Global), force);
			break;

		case nzCoordSys_Local:
			AddForce(m_matrix.Transform(force, 0.f), m_matrix.Transform(point));
			return;
	}

	// On réveille le corps pour que le callback soit appelé et que les forces soient appliquées
	NewtonBodySetSleepState(m_body, 0);
}

void NzPhysObject::AddTorque(const NzVector3f& torque, nzCoordSys coordSys)
{
	switch (coordSys)
	{
		case nzCoordSys_Global:
			m_torqueAccumulator += torque;
			break;

		case nzCoordSys_Local:
			m_torqueAccumulator += m_matrix.Transform(torque, 0.f);
			break;
	}

	// On réveille le corps pour que le callback soit appelé et que les forces soient appliquées
	NewtonBodySetSleepState(m_body, 0);
}

void NzPhysObject::EnableAutoSleep(bool autoSleep)
{
	NewtonBodySetAutoSleep(m_body, autoSleep);
}

NzBoxf NzPhysObject::GetAABB() const
{
	NzVector3f min, max;
	NewtonBodyGetAABB(m_body, min, max);

	return NzBoxf(min, max);
}

NzVector3f NzPhysObject::GetAngularVelocity() const
{
	NzVector3f angularVelocity;
	NewtonBodyGetOmega(m_body, angularVelocity);

	return angularVelocity;
}

const NzPhysGeomRef& NzPhysObject::GetGeom() const
{
	return m_geom;
}

float NzPhysObject::GetGravityFactor() const
{
	return m_gravityFactor;
}

NewtonBody* NzPhysObject::GetHandle() const
{
	return m_body;
}

float NzPhysObject::GetMass() const
{
	return m_mass;
}

NzVector3f NzPhysObject::GetMassCenter(nzCoordSys coordSys) const
{
	NzVector3f center;
	NewtonBodyGetCentreOfMass(m_body, center);

	switch (coordSys)
	{
		case nzCoordSys_Global:
			center = m_matrix.Transform(center);
			break;

		case nzCoordSys_Local:
			break; // Aucune opération à effectuer sur le centre de rotation
	}

	return center;
}

const NzMatrix4f& NzPhysObject::GetMatrix() const
{
	return m_matrix;
}

NzVector3f NzPhysObject::GetPosition() const
{
	return m_matrix.GetTranslation();
}

NzQuaternionf NzPhysObject::GetRotation() const
{
	return m_matrix.GetRotation();
}

NzVector3f NzPhysObject::GetVelocity() const
{
	NzVector3f velocity;
	NewtonBodyGetVelocity(m_body, velocity);

	return velocity;
}

bool NzPhysObject::IsAutoSleepEnabled() const
{
	return NewtonBodyGetAutoSleep(m_body) != 0;
}

bool NzPhysObject::IsMoveable() const
{
	return m_mass > 0.f;
}

bool NzPhysObject::IsSleeping() const
{
	return NewtonBodyGetSleepState(m_body) != 0;
}

void NzPhysObject::SetAngularVelocity(const NzVector3f& angularVelocity)
{
	NewtonBodySetOmega(m_body, angularVelocity);
}

void NzPhysObject::SetGeom(NzPhysGeomRef geom)
{
	if (m_geom != geom)
	{
		if (geom)
			m_geom = geom;
		else
			m_geom = NzNullGeom::New();

		NewtonBodySetCollision(m_body, m_geom->GetHandle(m_world));
	}
}

void NzPhysObject::SetGravityFactor(float gravityFactor)
{
	m_gravityFactor = gravityFactor;
}

void NzPhysObject::SetMass(float mass)
{
	if (m_mass > 0.f)
	{
		float Ix, Iy, Iz;
		NewtonBodyGetMassMatrix(m_body, &m_mass, &Ix, &Iy, &Iz);
		float scale = mass/m_mass;
		NewtonBodySetMassMatrix(m_body, mass, Ix*scale, Iy*scale, Iz*scale);
	}
	else if (mass > 0.f)
	{
		NzVector3f inertia, origin;
		m_geom->ComputeInertialMatrix(&inertia, &origin);

		NewtonBodySetCentreOfMass(m_body, &origin.x);
		NewtonBodySetMassMatrix(m_body, mass, inertia.x*mass, inertia.y*mass, inertia.z*mass);
		NewtonBodySetForceAndTorqueCallback(m_body, &ForceAndTorqueCallback);
		NewtonBodySetTransformCallback(m_body, &TransformCallback);
	}

	m_mass = mass;
}

void NzPhysObject::SetMassCenter(const NzVector3f& center)
{
	if (m_mass > 0.f)
		NewtonBodySetCentreOfMass(m_body, center);
}

void NzPhysObject::SetPosition(const NzVector3f& position)
{
	m_matrix.SetTranslation(position);
	UpdateBody();
}

void NzPhysObject::SetRotation(const NzQuaternionf& rotation)
{
	m_matrix.SetRotation(rotation);
	UpdateBody();
}

void NzPhysObject::SetVelocity(const NzVector3f& velocity)
{
	NewtonBodySetVelocity(m_body, velocity);
}

NzPhysObject& NzPhysObject::operator=(const NzPhysObject& object)
{
	NzPhysObject physObj(object);
	return operator=(std::move(physObj));
}

void NzPhysObject::UpdateBody()
{
	NewtonBodySetMatrix(m_body, m_matrix);

	/*for (std::set<PhysObjectListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
		(*it)->PhysObjectOnUpdate(this);*/
}

NzPhysObject& NzPhysObject::operator=(NzPhysObject&& object)
{
	if (m_body)
		NewtonDestroyBody(m_world->GetHandle(), m_body);

	m_body               = object.m_body;
	m_forceAccumulator   = std::move(object.m_forceAccumulator);
	m_geom               = std::move(object.m_geom);
	m_gravityFactor      = object.m_gravityFactor;
	m_mass               = object.m_mass;
	m_matrix             = std::move(object.m_matrix);
	m_torqueAccumulator  = std::move(object.m_torqueAccumulator);
	m_world              = object.m_world;

	object.m_body = nullptr;

	return *this;
}

void NzPhysObject::ForceAndTorqueCallback(const NewtonBody* body, float timeStep, int threadIndex)
{
	NazaraUnused(timeStep);
	NazaraUnused(threadIndex);

	NzPhysObject* me = static_cast<NzPhysObject*>(NewtonBodyGetUserData(body));

	if (!NzNumberEquals(me->m_gravityFactor, 0.f))
		me->m_forceAccumulator += me->m_world->GetGravity() * me->m_gravityFactor * me->m_mass;

	/*for (std::set<PhysObjectListener*>::iterator it = me->m_listeners.begin(); it != me->m_listeners.end(); ++it)
		(*it)->PhysObjectApplyForce(me);*/

	NewtonBodySetForce(body, me->m_forceAccumulator);
	NewtonBodySetTorque(body, me->m_torqueAccumulator);

	me->m_torqueAccumulator.Set(0.f);
	me->m_forceAccumulator.Set(0.f);

	///TODO: Implanter la force gyroscopique?
}

void NzPhysObject::TransformCallback(const NewtonBody* body, const float* matrix, int threadIndex)
{
	NazaraUnused(threadIndex);

	NzPhysObject* me = static_cast<NzPhysObject*>(NewtonBodyGetUserData(body));
	me->m_matrix.Set(matrix);

	/*for (std::set<PhysObjectListener*>::iterator it = me->m_listeners.begin(); it != me->m_listeners.end(); ++it)
		(*it)->PhysObjectOnUpdate(me);*/
}
