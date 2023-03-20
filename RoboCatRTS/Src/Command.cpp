#include "RoboCatPCH.h"

shared_ptr< Command > Command::StaticReadAndCreate( InputMemoryBitStream& inInputStream )
{
	CommandPtr retVal;
	
	int type = CM_INVALID;
	inInputStream.Read( type );
	uint32_t networkId = 0;
	inInputStream.Read( networkId );
	uint32_t playerId = 0;
	inInputStream.Read( playerId );

	switch ( type )
	{
	case CM_ATTACK:
		retVal = std::make_shared< AttackCommand >();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read( inInputStream );
		break;
	case CM_MOVE:
		retVal = std::make_shared< MoveCommand >();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read( inInputStream );
		break;
	case CM_SPECIAL:
		retVal = std::make_shared<SpecialAttackCommand>();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read(inInputStream);
		break;
	case CM_DASH:
		retVal = std::make_shared<DashCommand>();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read(inInputStream);
		break;
	case CM_BUILD:
		retVal = std::make_shared<BuildCommand>();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read(inInputStream);
		break;
	default:
		LOG( "Read in an unknown command type??" );
		break;
	}

	return retVal;
}

void Command::Write( OutputMemoryBitStream& inOutputStream )
{
	inOutputStream.Write( mCommandType );
	inOutputStream.Write( mNetworkId );
	inOutputStream.Write( mPlayerId );
}

AttackCommandPtr AttackCommand::StaticCreate( uint32_t inMyNetId, uint32_t inTargetNetId )
{
	AttackCommandPtr retVal;
	GameObjectPtr me = NetworkManager::sInstance->GetGameObject( inMyNetId );
	GameObjectPtr target = NetworkManager::sInstance->GetGameObject( inTargetNetId );
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();
	//can only issue commands to this unit if I own it, and it's a cat,
	//and if the target is a cat that's on a different team
	if ( me && me->GetClassId() == RoboCat::kClassId &&
		me->GetPlayerId() ==  playerId &&
		target && target->GetClassId() == RoboCat::kClassId &&
		target->GetPlayerId() != me->GetPlayerId() )
	{
		retVal = std::make_shared< AttackCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
		retVal->mTargetNetId = inTargetNetId;
	}

	//inclusion for the fast cat
	if (me && me->GetClassId() == FastCat::kClassId &&
		me->GetPlayerId() == playerId &&
		target && target->GetClassId() == FastCat::kClassId &&
		target->GetPlayerId() != me->GetPlayerId())
	{
		retVal = std::make_shared< AttackCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
		retVal->mTargetNetId = inTargetNetId;
	}

	return retVal;
}

void AttackCommand::Write( OutputMemoryBitStream& inOutputStream )
{
	Command::Write( inOutputStream );
	inOutputStream.Write( mTargetNetId );
}

void AttackCommand::Read( InputMemoryBitStream& inInputStream )
{
	inInputStream.Read( mTargetNetId );
}

void AttackCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject( mNetworkId );
	//robo cat attack command
	if ( obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId )
	{
		RoboCat* rc = obj->GetAsCat();
		rc->EnterAttackState( mTargetNetId );
	}

	//fast cat attack command
	if (obj && obj->GetClassId() == FastCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* fc = obj->GetAsCat();
		fc->EnterAttackState(mTargetNetId);
	}

}

MoveCommandPtr MoveCommand::StaticCreate( uint32_t inNetworkId, const Vector3& inTarget )
{
	MoveCommandPtr retVal;
	GameObjectPtr go = NetworkManager::sInstance->GetGameObject( inNetworkId );
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();

	//can only issue commands to this unit if I own it, and it's a cat
	if ( go && go->GetClassId() == FastCat::kClassId && 
		go->GetPlayerId() == playerId )
	{
		retVal = std::make_shared< MoveCommand >();
		retVal->mNetworkId = inNetworkId;
		retVal->mPlayerId = playerId;
		retVal->mTarget = inTarget;
	}

	if (go && go->GetClassId() == RoboCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< MoveCommand >();
		retVal->mNetworkId = inNetworkId;
		retVal->mPlayerId = playerId;
		retVal->mTarget = inTarget;
	}

	return retVal;
}

void MoveCommand::Write( OutputMemoryBitStream& inOutputStream )
{
	Command::Write( inOutputStream );
	inOutputStream.Write( mTarget );
}

void MoveCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject( mNetworkId );
	//normal cat move command
	if ( obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId )
	{
		RoboCat* rc = obj->GetAsCat();
		rc->EnterMovingState( mTarget );
	}
	//fast cat move command
	if (obj && obj->GetClassId() == FastCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* fc = obj->GetAsCat();
		fc->EnterMovingState(mTarget);
	}
}

void MoveCommand::Read( InputMemoryBitStream& inInputStream )
{
	inInputStream.Read( mTarget );
}

shared_ptr<SpecialAttackCommand> SpecialAttackCommand::StaticCreate(uint32_t inMyNetId, const Vector3 inTarget)
{
	SpecialAttackCommandPtr retVal;
	GameObjectPtr go = NetworkManager::sInstance->GetGameObject(inMyNetId);
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();
	//can only issue commands to this unit if I own it, and it's a cat,
	//and if the target is a cat that's on a different team
	if (go && go->GetClassId() == RoboCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< SpecialAttackCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
		retVal->mTarget = inTarget;
	}

	//inclusion for the fast cat
	if (go && go->GetClassId() == FastCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< SpecialAttackCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
		retVal->mTarget = inTarget;
	}

	return retVal;
}

void SpecialAttackCommand::Write(OutputMemoryBitStream& inOutputStream)
{
	Command::Write(inOutputStream);
	inOutputStream.Write(mTarget);
}

void SpecialAttackCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject(mNetworkId);
	//robo cat attack command
	if (obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* rc = obj->GetAsCat();
		rc->EnterSpecialAttackState(mTarget);
	}

	//fast cat attack command
	if (obj && obj->GetClassId() == FastCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* fc = obj->GetAsCat();
		fc->EnterSpecialAttackState(mTarget);
	}

}

void SpecialAttackCommand::Read(InputMemoryBitStream& inInputStream)
{
	inInputStream.Read(mTarget);
}

shared_ptr<DashCommand> DashCommand::StaticCreate(uint32_t inMyNetId)
{
	DashCommandPtr retVal;
	GameObjectPtr go = NetworkManager::sInstance->GetGameObject(inMyNetId);
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();

	//can only issue commands to this unit if I own it, and it's a cat
	if (go && go->GetClassId() == FastCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< DashCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
	}

	if (go && go->GetClassId() == RoboCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< DashCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
	}

	return retVal;
}

void DashCommand::Write(OutputMemoryBitStream& inOutputStream)
{
	Command::Write(inOutputStream);
}

void DashCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject(mNetworkId);
	//normal cat move command
	if (obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* rc = obj->GetAsCat();
		rc->EnterDashState();
	}
	//fast cat move command
	if (obj && obj->GetClassId() == FastCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* fc = obj->GetAsCat();
		fc->EnterDashState();
	}

}

void DashCommand::Read(InputMemoryBitStream& inInputStream)
{
}

shared_ptr<BuildCommand> BuildCommand::StaticCreate(uint32_t inMyNetId, const Vector3 inTarget)
{
	BuildCommandPtr retVal;
	GameObjectPtr go = NetworkManager::sInstance->GetGameObject(inMyNetId);
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();
	//can only issue commands to this unit if I own it, and it's a cat,
	//and if the target is a cat that's on a different team
	if (go && go->GetClassId() == RoboCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< BuildCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
		retVal->mTarget = inTarget;
	}

	//inclusion for the fast cat
	if (go && go->GetClassId() == FastCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< BuildCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
		retVal->mTarget = inTarget;
	}

	return retVal;
}

void BuildCommand::Write(OutputMemoryBitStream& inOutputStream)
{
	Command::Write(inOutputStream);
	inOutputStream.Write(mTarget);
}

void BuildCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject(mNetworkId);
	//robo cat attack command
	if (obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* rc = obj->GetAsCat();
		rc->EnterBuildState(mTarget);
	}

	//fast cat attack command
	if (obj && obj->GetClassId() == FastCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* fc = obj->GetAsCat();
		fc->EnterBuildState(mTarget);
	}

}

void BuildCommand::Read(InputMemoryBitStream& inInputStream)
{
	inInputStream.Read(mTarget);
}
