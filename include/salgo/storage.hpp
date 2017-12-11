#pragma once

#include "dense-map.hpp"

namespace salgo {

















namespace internal {
namespace Storage {


	enum class Type {
		INDEX_16,
		INDEX_32,
		//POINTER // TODO: not implemented
	};

	namespace {
		constexpr auto INDEX_16 = Type::INDEX_16;
		constexpr auto INDEX_32 = Type::INDEX_32;
		//constexpr auto POINTER  = Type::POINTER;
	}



	template<class TT>
	struct Int_Wrapper {
		Int_Wrapper() = default;
		Int_Wrapper(TT val) : value(val) {}

		TT& operator=(TT val) {
			value = val;
			return value;
		}

		operator TT&() {
			return value;
		}

		operator const TT&() const {
			return value;
		}

	private:
		TT value = std::numeric_limits<TT>::lowest();
	};


	template<Type TYPE>
	using Storage_Key = std::conditional_t<TYPE == INDEX_16, Int_Wrapper<uint16_t>, Int_Wrapper</*u*/int32_t>>;

	template<
		class T,
		Type TYPE,
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE
	>
	using Dense_Map_Base = typename std::conditional_t<	ERASABLE,
			typename salgo::Dense_Map<Storage_Key<TYPE>,T>::BUILDER::Erasable,
			typename salgo::Dense_Map<Storage_Key<TYPE>,T>::BUILDER
		>
		::Vector
		::template Accessor_Template<ACCESSOR_TEMPLATE> :: BUILD;












	//
	// currently it's only a proxy between Smesh / graph and Dense_Map
	//
	template<
		class T,
		Type TYPE,
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE
	>
	class Storage : public Dense_Map_Base<T, TYPE, ACCESSOR_TEMPLATE, ERASABLE> {

	public:
		using Dense_Map_Base<T, TYPE, ACCESSOR_TEMPLATE, ERASABLE> :: domain_end;

	private:
		using Dense_Map_Base<T, TYPE, ACCESSOR_TEMPLATE, ERASABLE> :: emplace_back; // use add instead

	public:
		static constexpr auto Type = TYPE;
		static constexpr auto Erasable = ERASABLE;

	public:
		using Key = Storage_Key<TYPE>;


		template<class... Args>
		Storage(Args&&... args)
			: Dense_Map_Base<T, TYPE, ACCESSOR_TEMPLATE, ERASABLE>(std::forward<Args>(args)... ) {}



		// returns accessor
		template<class... Args>
		auto add(Args&&... args) {
			DCHECK_LE(domain_end(), std::numeric_limits<Key>::max());
			//Key h = raw.domain_max();
			//int idx = raw.domain_end();
			return emplace_back(std::forward<Args>(args)... );
			//return h;
		}
	};




	//
	// BUILDER
	//
	template<
		class T,
		Type TYPE,
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE
	>
	class Builder {

	public:
		using BUILD = Storage<T, TYPE, ACCESSOR_TEMPLATE, ERASABLE>;

		using Index_16 = Builder<T, Type::INDEX_16, ACCESSOR_TEMPLATE, ERASABLE>;
		using Index_32 = Builder<T, Type::INDEX_32, ACCESSOR_TEMPLATE, ERASABLE>;

		// this is half-internal actually, e.g. binary_tree uses it:
		template<Type NEW_TYPE>
		using Internal_Type = Builder<T, NEW_TYPE, ACCESSOR_TEMPLATE, ERASABLE>;

		using Erasable = Builder<T, TYPE, ACCESSOR_TEMPLATE, true>;
		
		template<template<Const_Flag,class> class NEW_TMPL>
		using Accessor_Template = Builder<T, TYPE, NEW_TMPL, ERASABLE>;

	};


} // namespace Storage
} // namespace internal






//
// W/O BUILDER
//
template<
	class T
>
class Storage : public internal::Storage::Storage<
	T,
	internal::Storage::Type::INDEX_32,
	internal::Index_Accessor_Template,
	true // erasable
> {
private:
	using _BASE = internal::Storage::Storage<
		T,
		internal::Storage::Type::INDEX_32,
		internal::Index_Accessor_Template,
		true // erasable
	>;

public:
	template<class... Args>
	Storage(Args&&... args) : _BASE(std::forward<Args>(args)... ) {}

	template<class TT>
	Storage(const std::initializer_list<TT>& l) : _BASE(l) {}

	using BUILDER = internal::Storage::Builder<
		T,
		internal::Storage::Type::INDEX_32,
		internal::Index_Accessor_Template,
		false // erasable
	>;
};

















} // namespace salgo




namespace std {
	template<class INT>
	struct numeric_limits<salgo::internal::Storage::Int_Wrapper<INT>> : numeric_limits<INT> {};
}

