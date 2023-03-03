class FastCat : public RoboCat
{
public:
	CLASS_IDENTIFICATION('FCAT', GameObject)


	static	GameObjectPtr StaticCreate() { return NetworkManager::sInstance->RegisterAndReturn(new FastCat()); }

	FastCat();

	bool MoveToLocation(float inDeltaTime, const Vector3& inLocation);
};

typedef shared_ptr< FastCat > FastCatPtr;