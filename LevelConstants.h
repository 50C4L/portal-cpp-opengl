#ifndef _LEVEL_CONSTANTS_H
#define _LEVEL_CONSTANTS_H

namespace portal
{
	namespace level
	{
		// 物理分组
		// 用于区分哪些组之间可以发生碰撞
		enum class PhysicsGroup : int
		{
			RAY = 0x01,
			PLAYER = 0x02,
			WALL = 0x04,
			PORTAL_FRAME = 0x08,
			BOX = 0x16
		};
	}
}

#endif
