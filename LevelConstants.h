#ifndef _LEVEL_CONSTANTS_H
#define _LEVEL_CONSTANTS_H

namespace portal
{
	namespace level
	{
		enum class PhysicsGroup : int
		{
			RAY = 0x01,
			PLAYER = 0x02,
			WALL = 0x04,
		};
	}
}

#endif
