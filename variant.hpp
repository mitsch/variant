/// @file variant.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __VARIANT_BASIC_VARIANT_HPP__
#define __VARIANT_BASIC_VARIANT_HPP__

#include <variant/unified.hpp>
#include <variant/type_algorithms.hpp>
#include <variant/exceptions.hpp>
#include <variant/nullvariant.hpp>
#include <variant/signature.hpp>
#include <boost/optional.hpp>

namespace variant
{

	struct default_constructable_t {};
	constexpr default_constructable_t default_constructable();
	struct not_default_constructable_t {};
	constexpr not_default_constructable_t not_default_constructable();

	/// @class variant
	/// @tpararm DefaulConstructable ....
	template <bool DefaultConstructable, typename T, typename ... Ts>
	class variant
	{
		
		public:

			using signature_type = signature<T, Ts ...>;

			/// @brief default constructor
			/// @note this constructor is only opt in, if class is default constructible
			/// @post instance has no active object, hence is uninitialised
			template <typename = typename std::enable_if<DefaultConstructable>::type>
			constexpr variant() : active_index{none_initialised}
			{}

			/// @brief constructor carries out empty initialisation, that is having no active object
			/// @param n symbol for uninitialisiation
			/// @note this constructor is only opt in, if class is default constructible
			/// @post instance has no active object, hence is uninitialised
			template <typename = typename std::enable_if<DefaultConstructable>::type>
			constexpr variant(nullvariant_t n) : active_index{none_initialised}
			{}

			/// @brief copy constructor
			/// @param other instance to copy from
			variant(const variant & other)
			{
				active_index = other.active_index;
				if (active_index != none_initialised)
					values.copy(active_index, other.values);
			}

			/// @brief move constructor
			/// @param other instance to move from
			variant(variant && other)
			{
				active_index = other.active_index;
				if (active_index != none_initialised)
				{
					values.move(active_index, other.values);
					if (DefaultConstructable)
					{
						other.values.destruct(active_index);
						other.active_index = none_initialised;
					}
				}
			}

			/// @brief initialising constructor
			/// @tparam X type to which variant should be initialised
			/// @param x initialising value
			/// @note this constructor is only opt in, if \a X is part of the type list (T, Ts ...)
			template <typename X, typename = typename std::enable_if<type::has<X, T, Ts ...>::value>::type>
			variant(X && x)
			{
				active_index = type::first<X, T, Ts ...>::value;
				values.initialise(std::forward<X>(x));
			}

			/// @brief destructor destruct the active object if there is so
			~variant()
			{
				if (active_index != none_initialised)
					values.destruct(active_index);
			}

			/// @brief copy assignment operator
			/// @return this instance
			/// @param other instance to copy from
			variant& operator = (const variant & other)
			{
				if (active_index == none_initialised && other.active_index != none_initialised)
					values.initialise_copy(other.active_index, other.values);
				else if (active_index != none_initialised && other.active_index == none_initialised)
					values.destruct(active_index);
				else if (active_index != none_initialised && other.active_index != none_initialised)
				{
					if (active_index == other.active_index)
						values.copy(active_index, other.values);
					else
					{
						values.destruct(active_index);
						values.initialise_copy(other.active_index, other.values);
					}
				}
				active_index = other.active_index;
				return *this;
			}

			/// @brief move assignment operator
			/// @return this instance
			/// @param other instance to move from
			/// @note If variant is default constructible, instance \a other will be uninitialised afterwards.
			variant& operator = (variant && other)
			{
				if (active_index == none_initialised && other.active_index != none_initialised)
					values.initialise_move(other.active_index, other.values);
				else if (active_index != none_initialised && other.active_index == none_initialised)
					values.destruct(active_index);
				else if (active_index != none_initialised && other.active_index != none_initialised)
				{
					if (active_index == other.active_index)
						values.move(active_index, other.values);
					else
					{
						values.destruct(active_index);
						values.initialise_move(other.active_index, other.values);
					}
				}
				active_index = other.active_index;
				if (other.active_index != none_initialised && DefaultConstructable)
				{
					other.values.destruct(other.active_index);
					other.active_index = none_initialised;
				}
				return *this;
			}

			/// @brief assign nullvariant, that is, empty variant so it is not initialised any more
			/// @note this operator is only opt in if class is default constructible
			/// @return this instance
			/// @param n symbol for empty variant
			template <typename = typename std::enable_if<DefaultConstructable>::type>
			variant& operator = (nullvariant_t n)
			{
				if (active_index != none_initialised)
				{
					values.destruct(active_index);
					active_index = none_initialised;
				}
				return *this;
			}

			/// @brief assign active object
			/// @return this instance
			/// @param x value to assign
			/// @note this operator is only opt in, if \a X is part of the type list (T, Ts ...)
			template <typename X, typename = typename std::enable_if<type::has<X, T, Ts ...>::value>::type>
			variant& operator = (X && x)
			{
				auto new_active_index = type::first<X, T, Ts ...>::value;
				if (active_index == none_initialised)
					values.initialise(std::forward<X>(x));
				else if (active_index == new_active_index)
					values.assign(std::forward<X>(x));
				else
				{
					values.destruct(active_index);
					values.initialise(std::forward<X>(x));
				}
				active_index = new_active_index;
				return *this;
			}

			/// @brief test whether type \a X is currently active
			/// @tparam X type to test on
			/// @retval true currently type \a X is active
			/// @retval false currently type \a X is not active
			/// @pre type \a X must be one of type list (T, Ts ...). If class is default constructable,
			///				type \a X can be nullvariant_t too.
			template <typename X>
			constexpr bool is()
			{
				static_assert(DefaultConstructable || type::has<X, T, Ts ...>::value, "type X must be part of type list (T, Ts ...)!");
				static_assert(!DefaultConstructable || std::is_same<X, nullvariant_t>::value || type::has<X, T, Ts ...>::value,
					"type X must be either nullvariant_t or one of type list (T, Ts ...)!");
				return active_index == (std::is_same<X, nullvariant_t>::value ? none_initialised : type::first<X, T, Ts ...>::value);
			}

			/// @brief get object
			/// @tparam X type of requested object
			/// @return constant reference to object
			/// @pre type \a X must be one of type list (T, Ts ...)
			/// @exception bad_variant_access object with type \a X is currently not active
			template <typename X>
			const X& get() const
			{
				static_assert(type::has<X, T, Ts ...>::value, "type X must be part of type list (T, Ts ...)!");
				auto x_index = type::first<X, T, Ts ...>::value;
				if (x_index != active_index) throw bad_variant_access("invalid access on currently not active object!");
				return values.template get<X>();
			}			

			/// @brief get object
			/// @tparam X type of requested object
			/// @return reference to object
			/// @pre type \a X must be one of type list (T, Ts ...)
			/// @exception bad_variant_access object with type \a X is currently not active
			template <typename X>
			X& get()
			{
				static_assert(type::has<X, T, Ts ...>::value, "type X must be part of type list (T, Ts ...)!");
				auto x_index = type::first<X, T, Ts ...>::value;
				if (x_index != active_index) throw bad_variant_access("invalid access on currently not active object!");
				return values.template get<X>();
			}

			/// @brief get object
			/// @tparam X type of requested object
			/// @return constant reference to object
			/// @pre type \a X must be one of type list (T, Ts ...)
			/// @pre object of type \a X must be active, otherwise behaviour is undefined
			template <typename X>
			constexpr const X& get_unsafe() const noexcept
			{
				static_assert(type::has<X, T, Ts ...>::value, "type X must be part of type list (T, Ts ...)!");
				return values.template get<X>();
			}			

			/// @brief get object
			/// @tparam X type of requested object
			/// @return reference to object
			/// @pre type \a X must be one of type list (T, Ts ...)
			/// @pre object of type \a X must be active, otherwise behaviour is undefined
			template <typename X>
			X& get_unsafe() noexcept
			{
				static_assert(type::has<X, T, Ts ...>::value, "type X must be part of type list (T, Ts ...)!");
				return values.template get<X>();
			}			

			/// @brief tries to get object
			/// @tparam X type of requested object
			/// @return optional reference to object, depending on whether the requested object is the active one.
			/// @pre type \a X must be on of the type list (T, Ts ...)
			template <typename X>
			boost::optional<X> try_get()
			{
				static_assert(type::has<X, T, Ts ...>::value, "type X must be part of type list (T, Ts ...)!");
				constexpr size_t x_index = type::first<X, T, Ts ...>::value;
				return boost::make_optional(x_index == active_index, std::move(values.template get<X>()));
			}

			/// @brief compare on whether variant is uninitialised
			/// @retval true \a v is not initialised, that is it has no active object right now
			/// @retval false \a v is initialised, that is it has an active object right now
			/// @param v variant instance which will be tested on having no active object
			/// @param n constant instance which represents having no active object
			friend constexpr bool operator == (const variant & v, nullvariant_t n)
			{
				return v.active_index == none_initialised;
			}

			/// @brief compare on whether variant is initialised
			/// @retval false \a v is not initialised, that is it has no active object right now
			/// @retval true \a v is initialised, that is it has an active object right now
			/// @param v variant instance which will be tested on having an active object
			/// @param n constant instance which represents having no active object
			friend constexpr bool operator != (const variant & v, nullvariant_t n)
			{
				return v.active_index != none_initialised;
			}

			/// @brief compare on equality
			/// @retval true both instances have equal active objects or both are uninitialised
			/// @retval false either one of both instances is not initialised whereas the other one is or
			///					both instances have different active objects or they have same active objects
			///					but these are not equal
			/// @param first first variant instance
			/// @param second second variant instance
			friend constexpr bool operator == (const variant & first, const variant & second)
			{
				return first.active_index == second.active_index &&
						(first.active_index == none_initialised ||
						first.values.is_equal(first.active_index, second.values));
			}

			/// @brief compare on inequality
			/// @retval false both instances have equal active objects or both are uninitialised
			/// @retval true either one of both instances is not initialised whereas the other one is or
			///					both instances have different active objects or they have same active objects
			///					but these are not equal
			/// @param first first variant instance
			/// @param second second variant instance
			friend constexpr bool operator != (const variant & first, const variant & second)
			{
				return first.active_index != second.active_index || first.values.is_not_equal(first.active_index, second.values);
			}			

		private:

			static constexpr std::size_t none_initialised = type::length<T, Ts ...>::value;
			
			size_t active_index;
			unified<T, Ts ...> values;
	};


	template <typename X, typename T, typename ... Ts>
	variant<true, T, Ts ...> make_variant(X && x, signature<T, Ts ...>, default_constructable_t)
	{
		return variant<true, T, Ts ...>(std::forward<X>(x));
	}

	template <typename X, typename T, typename ... Ts>
	variant<false, T, Ts ...> make_variant(X && x, signature<T, Ts ...>, not_default_constructable_t)
	{
		return variant<false, T, Ts ...>(std::forward<X>(x));
	}


}

#endif

