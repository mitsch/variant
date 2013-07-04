/// @file type_algorithms.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __VARIANT_TYPE_ALGORITHMS_HPP__
#define __VARIANT_TYPE_ALGORITHMS_HPP__

#include <type_traits>

namespace variant
{
namespace type
{

	template <typename T> struct typer {using type = T;};

	template <typename ... Ts> struct length;
	template <> struct length<> {static constexpr std::size_t value = 0;};
	template <typename T, typename ... Ts> struct length<T, Ts ...> {static constexpr std::size_t value = 1 + length<Ts ...>::value;};

	template <unsigned int index, typename T, typename ... Ts> struct at
	{
		static_assert(index < length<T, Ts ...>::value, "index is higher than length of type list!");
		using type = typename std::conditional<index == 0, typer<T>, at<index-1, Ts ...>>::type::type;
	};

	template <typename X, typename ... Ts> struct has;
	template <typename X> struct has<X> : std::false_type {};
	template <typename X, typename T, typename ... Ts> struct has<X, T, Ts ...> :
		std::conditional<std::is_same<X, T>::value, std::true_type, has<X, Ts ...>>::type {};

	template <typename X, typename ... Ts> struct first;
	template <typename X> struct first<X> {static constexpr std::size_t value = 0;};
	template <typename X, typename T, typename ... Ts> struct first<X, T, Ts ...>
	{
		static constexpr std::size_t value = std::is_same<X, T>::value ? 0 : 1 + first<X, Ts ...>::value;
	};
}
}

#endif

