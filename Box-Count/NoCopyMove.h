#pragma once

namespace eg::sys
{
	class NoCopyMove
	{
	public:
		NoCopyMove() = default;
		NoCopyMove(const NoCopyMove&) = delete;
		NoCopyMove(NoCopyMove&&) = delete;
		NoCopyMove& operator=(const NoCopyMove&) = delete;
		NoCopyMove& operator=(NoCopyMove&&) = delete;
		virtual ~NoCopyMove() = default;
	};
}