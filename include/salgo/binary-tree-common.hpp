#pragma once

#include "accessors-common.hpp"

namespace salgo {



namespace internal {
namespace Binary_Tree {

	//
	// swap flags
	//
	template<bool> struct Vert_Add_evert       {};
	template<    > struct Vert_Add_evert<true> { bool evert = false; };


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


	//
	// vertex aggreg
	//
	template<bool, class A> struct Vert_Add_aggreg         {};
	template<      class A> struct Vert_Add_aggreg<true,A> {

		A val = A();

		Vert_Add_aggreg() = default;

		template<class... Args>
		Vert_Add_aggreg(Args&&... args) : val(std::forward<Args>(args)... ) {}
	};


	//
	// vertex propag
	//
	template<bool, class P> struct Vert_Add_propag         {};
	template<      class P> struct Vert_Add_propag<true,P> {

		P val = P();

		Vert_Add_propag() = default;

		template<class... Args>
		Vert_Add_propag(Args&&... args) : val(std::forward<Args>(args)... ) {}
	};
}



} // namespace Binary_Tree
} // namespace salgo


