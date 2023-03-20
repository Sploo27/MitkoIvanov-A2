class RoboCat : public GameObject
{
public:
	CLASS_IDENTIFICATION( 'RCAT', GameObject )

	enum RoboCatState
	{
		RC_IDLE,
		RC_MOVING,
		RC_ATTACK,
		RC_SPECIAL,
		RC_DASH,
		RC_BUILD
	};

	static	GameObjectPtr StaticCreate() { return NetworkManager::sInstance->RegisterAndReturn( new RoboCat() ); }

	virtual	RoboCat*	GetAsCat()	override { return this; }

	virtual void Update( float inDeltaTime )	override;
	void UpdateIdleState( float inDeltaTime );
	void UpdateMovingState( float inDeltaTime );
	void UpdateAttackState( float inDeltaTime );
	void UpdateSpecialAttackState(float inDeltaTime);
	void UpdateDashState(float inDeltaTime);
	void UpdateBuildState(float inDeltaTime);

	// returns true if the move is done
	bool MoveToLocation( float inDeltaTime, const Vector3& inLocation );
	bool Dash(float inDeltaTime);
	void UpdateRotation( const Vector3& inTarget );
	
	void EnterMovingState( const Vector3& inTarget );
	void EnterAttackState( uint32_t inTargetNetId );
	void EnterSpecialAttackState(const Vector3& inTarget);
	void EnterDashState();
	void EnterBuildState(const Vector3& inTarget);

	void TakeDamage( int inDmgAmount );
	virtual void HandleDying() override;

	RoboCat();

	virtual void WriteForCRC( OutputMemoryBitStream& inStream ) override;
private:
	Vector3				mMoveLocation;
	Vector3				mAttackLocation;
	Vector3				mBuildLocation;
	SpriteComponentPtr	mSpriteComponent;

	///move down here for padding reasons...
	int					mHealth;
	RoboCatState		mState;
	uint32_t			mTargetNetId;
	GameObjectPtr		mTargetCat;
	float				mTimeSinceLastAttack;
	float				mDashTime;
};

typedef shared_ptr< RoboCat >	RoboCatPtr;