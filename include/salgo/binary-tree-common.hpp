#pragma once

#include "accessors-common.hpp"

namespace salgo {



namespace internal {
namespace Binary_Tree {

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



} // namespace Binary_Tree
} // namespace salgo


