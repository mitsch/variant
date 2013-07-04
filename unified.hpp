/// @file unified.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __VARIANT_UNIFIED_HPP__
#define __VARIANT_UNIFIED_HPP__

#include <variant/type_algorithms.hpp>
#include <cassert>
#include <algorithm>

namespace variant
{

	template <typename T, typename ... Ts>
	union unified;

	template <typename T>
	union unified<T>
	{
		private:

			T head;

		public:

			template <typename X> X& get()
			{
				static_assert(std::is_same<T, X>::value, "could not find requested type!");
				return head;
			}
			template <typename X> const X& get() const
			{
				static_assert(std::is_same<T, X>::value, "could not find requested type!");
				return head;
			}

			void destruct(const std::size_t index)
			{
				assert(index == 0);
				head.~T();
			}

			void copy(const std::size_t index, const unified & other)
			{
				assert(index == 0);
				head = other.head;
			}

			void move(const std::size_t index, unified & other)
			{
				assert(index == 0);
				head = std::move(other.head);
			}

			template <typename X>
			void assign(X && x)
			{
				static_assert(std::is_same<T, X>::value, "could not find requested type!");
				head = std::forward<X>(x);
			}

			void initialise_copy(const std::size_t index, unified & other)
			{
				assert(index == 0);
				new (&head) T(other.head);
			}

			void initialise_move(const std::size_t index, unified & other)
			{
				assert(index == 0);
				new (&head) T(std::move(other.head));
			}

			template <typename X> void initialise(X && x)
			{
				static_assert(std::is_same<X, T>::value, "could not find requested type!");
				new (&head) T(std::forward<X>(x));
			}

			bool is_equal(const std::size_t index, const unified & other)
			{
				assert(index == 0);
				return head == other.head;
			}

			bool is_not_equal(const std::size_t index, const unified & other)
			{
				assert(index == 0);
				return head != other.head;
			}

			unified() {}
			~unified() {}
	};

	template <typename T1, typename T2, typename ... Ts>
	union unified<T1, T2, Ts ...>
	{
		private:

			T1 head;
			unified<T2, Ts ...> tail;


		public:

			template <typename X> X& get()
			{
				return get<X>(typename std::is_same<X, T1>::type());
			}
			template <typename X> X& get(std::true_type)
			{
				return head;
			}
			template <typename X> X& get(std::false_type)
			{
				return tail.template get<X>();
			}

			template <typename X> const X& get() const
			{
				return get<X>(typename std::is_same<X, T1>::type());
			}
			template <typename X> const X& get(std::true_type) const
			{
				return head;
			}
			template <typename X> const X& get(std::false_type) const
			{
				return tail.template get<X>();
			}

			void destruct(const std::size_t index)
			{
				if (index == 0)	head.~T1();
				else tail.destruct(index-1);
			}

			void copy(const std::size_t index, const unified & other)
			{
				if (index == 0) head = other.head;
				else tail.copy(index-1, other.tail);
			}

			void move(const std::size_t index, unified & other)
			{
				if (index == 0) head = std::move(other.head);
				else tail.move(index-1, other.tail);
			}

			template <typename X> void assign(X && x)
			{
				assign(std::forward<X>(x), typename std::is_same<X, T1>::type());
			}
			template <typename X> void assign(X && x, std::true_type)
			{
				head = std::forward<X>(x);
			}
			template <typename X> void assign(X && x, std::false_type)
			{
				tail.assign(std::forward<X>(x));
			}

			
			void initialise_copy(const std::size_t index, const unified & other)
			{
				if (index == 0) new (&head) T1(other.head);
				else tail.initialise_copy(index-1, other.tail);
			}

			void initialise_move(const std::size_t index, unified & other)
			{
				if (index == 0) new (&head) T1(std::move(other.head));
				else tail.initialise_move(index-1, other.tail);
			}

			template <typename X> void initialise(X && x)
			{
				initialise(std::forward<X>(x), typename std::is_same<X, T1>::type());
			}
			template <typename X> void initialise(X && x, std::true_type)
			{
				new (&head) T1(std::forward<X>(x));
			}
			template <typename X> void initialise(X && x, std::false_type)
			{
				tail.initialise(std::forward<X>(x));
			}

			bool is_equal(const std::size_t index, const unified & other)
			{
				if (index == 0)	return head == other.head;
				else return tail.is_eqal(index-1, other.tail);
			}

			bool is_not_equal(const std::size_t index, const unified & other)
			{
				if (index == 0) return head != other.head;
				else return tail.is_not_equal(index-1, other.tail);
			}

			unified() {}
			~unified() {}
	};


}

#endif

