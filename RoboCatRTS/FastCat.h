#pragma once
class FastCat : public RoboCat
{
public:
	CLASS_IDENTIFICATION('FCAT', GameObject)

	bool MoveToLocation(float inDeltaTime, const Vector3& inLocation);
};

typedef shared_ptr< FastCat > FastCatPtr;