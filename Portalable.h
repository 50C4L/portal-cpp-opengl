#ifndef _PORTALABLE_H
#define _PORTALABLE_H

namespace portal
{
	class Portal;

	class Portalable
	{
	public:
		Portalable() = default;
		~Portalable() = default;

		virtual void Teleport( Portal& in_portal ) = 0;
	};
}

#endif
