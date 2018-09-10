// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "Listener.h"

namespace CryAudio
{
namespace Impl
{
namespace Fmod
{
//////////////////////////////////////////////////////////////////////////
CListener::CListener(CObjectTransformation const& transformation, int const id)
	: m_id(id)
	, m_transformation(transformation)
	, m_isMovingOrDecaying(false)
	, m_velocity(ZERO)
	, m_position(transformation.GetPosition())
	, m_previousPosition(transformation.GetPosition())
{
	ZeroStruct(m_attributes);
}

//////////////////////////////////////////////////////////////////////////
void CListener::Update(float const deltaTime)
{
	if (m_isMovingOrDecaying && (deltaTime > 0.0f))
	{
		Vec3 const deltaPos(m_position - m_previousPosition);

		if (!deltaPos.IsZero())
		{
			m_velocity = deltaPos / deltaTime;
			m_previousPosition = m_position;
		}
		else if (!m_velocity.IsZero())
		{
			// We did not move last frame, begin exponential decay towards zero.
			float const decay = std::max(1.0f - deltaTime / 0.05f, 0.0f);
			m_velocity *= decay;

			if (m_velocity.GetLengthSquared() < FloatEpsilon)
			{
				m_velocity = ZERO;
				m_isMovingOrDecaying = false;
			}

			SetVelocity();
		}
		else
		{
			m_isMovingOrDecaying = false;
			SetVelocity();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CListener::SetName(char const* const szName)
{
#if defined(INCLUDE_FMOD_IMPL_PRODUCTION_CODE)
	m_name = szName;
#endif  // INCLUDE_FMOD_IMPL_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
void CListener::SetTransformation(CObjectTransformation const& transformation)
{
	m_transformation = transformation;
	m_position = transformation.GetPosition();

	Fill3DAttributeTransformation(m_transformation, m_attributes);

#if defined(INCLUDE_FMOD_IMPL_PRODUCTION_CODE)
	// Always update velocity in non-release builds for debug draw.
	m_isMovingOrDecaying = true;
#endif  // INCLUDE_FMOD_IMPL_PRODUCTION_CODE

	if (g_numObjectsWithDoppler > 0)
	{
		m_isMovingOrDecaying = true;
		Fill3DAttributeVelocity(m_velocity, m_attributes);
	}
	else
	{
		m_previousPosition = m_position;
	}

	FMOD_RESULT const fmodResult = s_pSystem->setListenerAttributes(m_id, &m_attributes);
	ASSERT_FMOD_OK;
}

//////////////////////////////////////////////////////////////////////////
void CListener::SetVelocity()
{
	Fill3DAttributeVelocity(m_velocity, m_attributes);
	FMOD_RESULT const fmodResult = s_pSystem->setListenerAttributes(m_id, &m_attributes);
	ASSERT_FMOD_OK;
}
} // namespace Fmod
} // namespace Impl
} // namespace CryAudio