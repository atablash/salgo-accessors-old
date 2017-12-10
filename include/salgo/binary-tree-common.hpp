#pragma once

#include "accessors-common.hpp"

namespace salgo {



//
// flags
//
enum class Binary_Tree_Flags {
	NONE = 0,
	IMPLICIT       = 0x0001,
	PARENT_LINKS   = 0x0002,
	EVERSIBLE      = 0x0004,
	VERTS_ERASABLE = 0x0008
};

namespace {
	constexpr auto IMPLICIT          = Binary_Tree_Flags::IMPLICIT;
	constexpr auto PARENT_LINKS      = Binary_Tree_Flags::PARENT_LINKS;
	constexpr auto EVERSIBLE         = Binary_Tree_Flags::EVERSIBLE;
	constexpr auto BT_VERTS_ERASABLE = Binary_Tree_Flags::VERTS_ERASABLE;
}

ENABLE_BITWISE_OPERATORS(Binary_Tree_Flags);









//
// default parameters
//
namespace internal {

	constexpr auto default_Binary_Tree_Flags = PARENT_LINKS | EVERSIBLE | BT_VERTS_ERASABLE;

	template<Const_Flag C, class OWNER, class BASE>
	using Default__Binary_Tree_Accessor_Template = Index_Accessor_Template<C, OWNER, BASE>;
}











namespace internal {

	//
	// swap flags
	//
	template<bool> struct Vert_Add_swap       {};
	template<    > struct Vert_Add_swap<true> { bool swap_children = false; };


	//
	// vertex value
	//
	template<bool, class T> struct Vert_Add_val         {};
	template<      class T> struct Vert_Add_val<true,T> {

		T val = T();

		template<class TT>
		inline auto& operator=(TT&& tt) {
			val = std::forward<TT>(tt);
			return *this;
		}

		Vert_Add_val() = default;

		template<class... Args>
		Vert_Add_val(Args&&... args) : val(std::forward<Args>(args)... ) {}
	};
}




} // namespace salgo


