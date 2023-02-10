#pragma once
#ifndef AISTATEDATA_H
#define AISTATEDATA_H

#include "AIStateType.h"
#include "JsonLoading.h"

using namespace AILogic;

namespace AIStateData
{
	struct FleeParams
	{
		float fMaxRange;
		float fMinRange;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FleeParams, fMaxRange, fMinRange);

	struct SeekParams
	{
		float fMaxRange;
		float fMinRange;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeekParams, fMaxRange, fMinRange);
	
	struct PatrolParams
	{
		float fSensingRange;
		int iWaypointCount;
		float fDistanceToWaypoint;
		PatrolType ePatrolType;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PatrolParams, fSensingRange, iWaypointCount, fDistanceToWaypoint, ePatrolType);

	struct FollowParams
	{
		float fFollowRange;
		float fRepulseRange;
		bool bKeepRange;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FollowParams, fFollowRange, fRepulseRange, bKeepRange);

	struct WanderParams
	{
		float fRange;
		float fWanderAngle;
		float fWanderDelay;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WanderParams, fWanderAngle, fWanderDelay);

	struct FireParams
	{
		float fRate;
		float fRange;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FireParams, fRate, fRange);

	struct AIStateJson
	{
		AIStateTypes iStateType;
		int iTypeID;
		float fMin, fMax;
		float fActivate;

		FleeParams oFleeParams;
		SeekParams oSeekParams;
		PatrolParams oPatrolParams;
		FollowParams oFollowParams;
		WanderParams oWanderParams;
		FireParams oFireParams;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AIStateJson, iStateType, iTypeID, fMin, fMax, fActivate, oFleeParams, oSeekParams, oPatrolParams, oFollowParams, oWanderParams, oFireParams);
}

#endif // !AISTATEDATA_H