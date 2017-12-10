#pragma once

#include "dense-map.hpp"

namespace salgo {






enum class Storage_Type {
	INDEX_16,
	INDEX_32,
	//POINTER // TODO: not implemented
};

namespace {
	constexpr auto INDEX_16 = Storage_Type::INDEX_16;
	constexpr auto INDEX_32 = Storage_Type::INDEX_32;
	//constexpr auto POINTER  = Storage_Type::POINTER;
}




enum class Storage_Flags {
	NONE = 0,
	ERASABLE = 0x0001
};

ENABLE_BITWISE_OPERATORS(Storage_Flags);

namespace {
	constexpr auto STORAGE_ERASABLE = Storage_Flags::ERASABLE;
}












namespace internal {
	constexpr auto default__Storage_Flags = STORAGE_ERASABLE;
	constexpr auto default__Storage_Type = INDEX_32;

	template<Const_Flag C, class OWNER, class BASE>
	using Default__Storage_Accessor_Template = Index_Accessor_Template<C, OWNER, BASE>;




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


	template<Storage_Type TYPE>
	using Storage_Key = std::conditional_t<TYPE == INDEX_16, Int_Wrapper<uint16_t>, Int_Wrapper</*u*/int32_t>>;

	template<class T,
		Storage_Flags FLAGS,
		Storage_Type TYPE,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE
	>
	using Dense_Map_Base = typename std::conditional_t<
		bool(FLAGS & STORAGE_ERASABLE),
		typename Dense_Map<Storage_Key<TYPE>,T>::BUILDER::Erasable,
		typename Dense_Map<Storage_Key<TYPE>,T>::BUILDER>
		::Vector
		::template Accessor_Template<ACCESSOR_TEMPLATE> :: BUILD;












	//
	// currently it's only a proxy between Smesh / graph and Dense_Map
	//
	template<
		class T,
		Storage_Flags FLAGS,
		Storage_Type TYPE,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE
	>
	class Storage : public internal::Dense_Map_Base<T, FLAGS, TYPE, ACCESSOR_TEMPLATE> {

	public:
		using internal::Dense_Map_Base<T, FLAGS, TYPE, ACCESSOR_TEMPLATE> :: domain_end;

	private:
		using internal::Dense_Map_Base<T, FLAGS, TYPE, ACCESSOR_TEMPLATE> :: emplace_back; // use add instead

	public:
		static constexpr auto Flags = FLAGS;
		static constexpr auto Type = TYPE;

	public:
		using Key = internal::Storage_Key<TYPE>;


		template<class... Args>
		Storage(Args&&... args)
			: internal::Dense_Map_Base<T, FLAGS, TYPE, ACCESSOR_TEMPLATE>(std::forward<Args>(args)... ) {}



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






} // internal






//
// W/O BUILDER
//
template<
	class T
>
using Storage = internal::Storage<
	T,
	internal::default__Storage_Flags,
	internal::default__Storage_Type,
	internal::Default__Storage_Accessor_Template
>;




//
// BUILDER
//
template<
	class T,
	Storage_Flags FLAGS = internal::default__Storage_Flags,
	Storage_Type TYPE = internal::default__Storage_Type,
	template<internal::Const_Flag,class,class> class ACCESSOR_TEMPLATE = internal::Default__Storage_Accessor_Template
>
class Storage_Builder {

public:
	using Build = internal::Storage<T, FLAGS, TYPE, ACCESSOR_TEMPLATE>;

	template<Storage_Type NEW_TYPE>
	using Type = Storage_Builder<T, FLAGS, NEW_TYPE, ACCESSOR_TEMPLATE>;
	
	template<template<internal::Const_Flag,class,class> class NEW_TMPL>
	using Accessor_Template = Storage_Builder<T, FLAGS, TYPE, NEW_TMPL>;


	template<Storage_Flags NEW_FLAGS>
	using Flags           = Storage_Builder<T, NEW_FLAGS, TYPE, ACCESSOR_TEMPLATE>;

	template<Storage_Flags NEW_FLAGS>
	using Add_Flags       = Storage_Builder<T, FLAGS |  NEW_FLAGS, TYPE, ACCESSOR_TEMPLATE>;

	template<Storage_Flags NEW_FLAGS>
	using Rem_Flags       = Storage_Builder<T, FLAGS & ~NEW_FLAGS, TYPE, ACCESSOR_TEMPLATE>;
};













} // namespace salgo




namespace std {
	template<class INT>
	struct numeric_limits<salgo::internal::Int_Wrapper<INT>> : numeric_limits<INT> {};
}

