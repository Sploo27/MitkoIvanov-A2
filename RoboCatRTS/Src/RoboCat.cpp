#include "RoboCatPCH.h"
#include <zlib.h>

const float kMoveSpeed = 2.5f;
const float kDashSpeed = kMoveSpeed * 3.0f;
const float kAttackRangeSq = 1.5f * 1.5f;
const float kYarnCooldown = 1.0f;

RoboCat::RoboCat() :
	GameObject(),
	mHealth( 5 ),
	mState( RC_IDLE ),
	mTargetNetId( 0 ),
	mTimeSinceLastAttack( kYarnCooldown )
{
	SetScale( 0.75f );
	SetCollisionRadius( 0.6f );
	mSpriteComponent = std::make_shared<SpriteComponent>( this );
	mSpriteComponent->SetTexture( TextureManager::sInstance->GetTexture( "cat" ) );
}

void RoboCat::WriteForCRC( OutputMemoryBitStream& inStream )
{
	inStream.Write( mPlayerId );
	inStream.Write( mNetworkId );
	inStream.Write( mLocation );
	inStream.Write( mHealth );
	inStream.Write( mState );
	inStream.Write( mTargetNetId );
}

bool RoboCat::MoveToLocation( float inDeltaTime, const Vector3& inLocation )
{
	bool finishedMove = false;

	Vector3 toMoveVec = inLocation - GetLocation();
	float distToTarget = toMoveVec.Length();
	toMoveVec.Normalize2D();
	if( distToTarget > 0.1f )
	{
		if ( distToTarget > ( kMoveSpeed * inDeltaTime ) )
		{
			SetLocation( GetLocation() + toMoveVec * inDeltaTime * kMoveSpeed );
		}
		else
		{
			//we're basically almost there, so set it to move location
			SetLocation( inLocation );
			finishedMove = true;
		}
	}
	else
	{
		//since we're close, stop moving towards the target
		finishedMove = true;
	}

	return finishedMove;
}

bool RoboCat::Dash(float inDeltaTime)//dashes the cat straight
{
	bool finishedMove = false;

	Vector3 toMoveVec = GetForwardVector();
	mDashTime += inDeltaTime;
	float mMaxDashTime = 0.4f;

	if (mDashTime < mMaxDashTime)
	{
		SetLocation(GetLocation() + toMoveVec * inDeltaTime * (kDashSpeed));
	}
	else
	{
		finishedMove = true;
		mDashTime = 0.0f;
	}

	return finishedMove;

}

void RoboCat::UpdateRotation( const Vector3& inTarget )
{
	Vector3 toMoveVec = inTarget - GetLocation();
	toMoveVec.Normalize2D();
	float angle = acosf( Dot2D( toMoveVec, Vector3::NegUnitY ) );
	Vector3 cross = Cross( Vector3::NegUnitY, toMoveVec );
	if ( cross.mZ < 0.0f )
	{
		angle *= -1.0f;
	}
	SetRotation( angle );
}

void RoboCat::EnterMovingState( const Vector3& inTarget )
{
	mMoveLocation = inTarget;
	
	UpdateRotation( inTarget );

	mState = RC_MOVING;
}

void RoboCat::EnterAttackState( uint32_t inTargetNetId )
{
	mTargetNetId = inTargetNetId;

	//cache the target cat
	GameObjectPtr go = NetworkManager::sInstance->GetGameObject( mTargetNetId );
	//double check this attack target is valid
	if ( go && go->GetClassId() == RoboCat::kClassId && go->GetPlayerId() != GetPlayerId() )
	{
		mTargetCat = go;
		mState = RC_ATTACK;
	}
	//make sure it recognizes fast cats
	if(go && go->GetClassId() == FastCat::kClassId && go->GetPlayerId() != GetPlayerId())
	{
		mTargetCat = go;
		mState = RC_ATTACK;
	}
	else
	{
		mState = RC_IDLE;
	}
}

void RoboCat::EnterSpecialAttackState(const Vector3& inTarget)
{
	mAttackLocation = inTarget;

	UpdateRotation(inTarget);

	mState = RC_SPECIAL;
}

void RoboCat::EnterDashState()
{
	mState = RC_DASH;
}

void RoboCat::EnterBuildState(const Vector3& inTarget)
{
	mBuildLocation = inTarget;

	UpdateRotation(inTarget);

	mState = RC_BUILD;

}

void RoboCat::TakeDamage( int inDmgAmount )
{
	mHealth -= inDmgAmount;
	if ( mHealth == 0 )
	{
		SetDoesWantToDie( true );
	}
}

void RoboCat::HandleDying()
{
	GameObject::HandleDying();
	ScoreBoardManager::sInstance->IncScore( mPlayerId, -1 );
}

void RoboCat::Update( float inDeltaTime )
{
	switch ( mState )
	{
	case RC_IDLE:
		UpdateIdleState( inDeltaTime );
		break;
	case RC_MOVING:
		UpdateMovingState( inDeltaTime );
		break;
	case RC_ATTACK:
		UpdateAttackState( inDeltaTime );
		break;
	case RC_SPECIAL:
		UpdateSpecialAttackState(inDeltaTime);
		break;
	case RC_DASH:
		UpdateDashState(inDeltaTime);
		break;
	case RC_BUILD:
		UpdateBuildState(inDeltaTime);

	}
}

void RoboCat::UpdateIdleState( float inDeltaTime )
{
	//could do something cute here, like have the cat meow if idle for a while!
	mTimeSinceLastAttack += inDeltaTime;
}

void RoboCat::UpdateMovingState( float inDeltaTime )
{
	mTimeSinceLastAttack += inDeltaTime;
	if ( MoveToLocation( inDeltaTime, mMoveLocation ) )
	{
		//done with the move, so go idle
		mState = RC_IDLE;
	}
}

void RoboCat::UpdateAttackState( float inDeltaTime )
{
	mTimeSinceLastAttack += inDeltaTime;

	if ( mTargetCat && !mTargetCat->DoesWantToDie() )
	{
		//determine the distance to the target cat
		Vector3 diff = mTargetCat->GetLocation() - mLocation;
		float distSq = diff.LengthSq2D();

		//if we're in yarn cooldown, we aren't allowed to do anything
		if ( mTimeSinceLastAttack >= kYarnCooldown )
		{
			UpdateRotation( mTargetCat->GetLocation() );
			if ( distSq <= kAttackRangeSq )
			{
				//if we're in range, throw a ball of yarn
				mTimeSinceLastAttack = 0.0f;
				GameObjectPtr me = NetworkManager::sInstance->GetGameObject( mNetworkId );
				YarnPtr yarn = std::static_pointer_cast< Yarn >( GameObjectRegistry::sInstance->CreateGameObject( 'YARN' ) );
				yarn->InitFromShooter( me, mTargetCat );
			}
			else
			{
				MoveToLocation( inDeltaTime, mTargetCat->GetLocation() );
			}
		}
	}
	else
	{
		//target cat is dead
		mTargetCat.reset();
		mState = RC_IDLE;
	}
}

void RoboCat::UpdateSpecialAttackState(float inDeltaTime)
{
	mTimeSinceLastAttack += inDeltaTime;
	
	Vector3 diff = mAttackLocation - mLocation;
	float distSq = diff.LengthSq2D();

	//if we're in yarn cooldown, we aren't allowed to do anything
	if (mTimeSinceLastAttack >= kYarnCooldown)
	{
		UpdateRotation(mAttackLocation);
		
		mTimeSinceLastAttack = 0.0f;
		GameObjectPtr me = NetworkManager::sInstance->GetGameObject(mNetworkId);
		YarnPtr yarn = std::static_pointer_cast<Yarn>(GameObjectRegistry::sInstance->CreateGameObject('YARN'));
		yarn->InitFromPosition(me, mAttackLocation);
		
	}
	mState = RC_IDLE;
}

void RoboCat::UpdateDashState(float inDeltaTime)
{
	mTimeSinceLastAttack += inDeltaTime;
	if (Dash(inDeltaTime))
	{
		//done with the move, so go idle
		mState = RC_IDLE;
	}

}

void RoboCat::UpdateBuildState(float inDeltaTime)
{
	mTimeSinceLastAttack += inDeltaTime;

	UpdateRotation(mBuildLocation);

	GameObjectPtr me = NetworkManager::sInstance->GetGameObject(mNetworkId);
	NetworkManager::sInstance->SpawnCat(mPlayerId, mBuildLocation);
	
	mState = RC_IDLE;
}

