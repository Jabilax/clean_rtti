#pragma once
#include <array>
#include <tuple>

template<typename T>
struct reflect_internal
{
	template<typename U>
	struct Data {};
};

template <typename T>
using reflect = typename reflect_internal<T>::Data<T>;

template<typename T>
struct deduce_type_member {
	
};

template<typename T, typename U>
struct deduce_type_member<T U::*> {
	using type = T;
};
template<typename T>
using deduce_type_member_t = typename deduce_type_member<T>::type;


/*
struct person;
template<>
struct reflect_internal<person>
{
	template <typename T>
	struct Data {

		static constexpr std::array<const char*, 2> members_names{ "age", "name" };
		static constexpr std::tuple<int T::*, const char * T::*> member_values{ &T::age, &T::name };
		
		template <unsigned Num>
		static constexpr 
			deduce_type_member_t<std::decay_t<decltype(std::get<Num>(member_values))>>&
			value(T&& obj) {
			return obj.*std::get<Num>(member_values);
		}

		template <unsigned Num>
		static constexpr
			const deduce_type_member_t<std::decay_t<decltype(std::get<Num>(member_values))>>
			value(const T obj) {
			return obj.*std::get<Num>(member_values);
		}
	};
};*/

#define reflect_struct(type)																				\
struct type;																								\
template<>																									\
struct reflect_internal<type>																				\
{																											\
	template <typename T>																					\
	struct Data {																							\
																											\
		DATA																								\
																											\
		template <unsigned Num>																				\
		static constexpr																					\
			deduce_type_member_t<std::decay_t<decltype(std::get<Num>(member_values))>>&						\
			value(T&& obj) {																				\
			return obj.*std::get<Num>(member_values);														\
		}																									\
																											\
		template <unsigned Num>																				\
		static constexpr																					\
			const deduce_type_member_t<std::decay_t<decltype(std::get<Num>(member_values))>>				\
			value(const T obj) {																			\
			return obj.*std::get<Num>(member_values);														\
		}																									\
	};																										\
};

#define DATA																					\
static constexpr std::array<const char*, 2> members_names{ "age", "name" };						\
static constexpr std::tuple<int T::*, const char * T::*> member_values{ &T::age, &T::name };
reflect_struct(person)


enum PersonNames;
template<>
struct reflect_internal<PersonNames>
{
	template<typename T>
	struct Data {
		static constexpr std::array<const char*, 2> name{ "steve", "peter" };
		static constexpr std::array<T, 2> value{ T::steve, T::peter };
	};
};
