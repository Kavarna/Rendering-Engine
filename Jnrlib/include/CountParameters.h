#pragma once 

namespace Jnrlib
{
	template <typename ... Args>  struct CountParameters;

	template <>
	struct CountParameters<>
	{
		constexpr static const int value = 0;
	};

	template <typename T, typename ... Args>
	struct CountParameters<T, Args...>
	{
		constexpr static const int value = 1 + CountParameters<Args...>::value;
	};
}
