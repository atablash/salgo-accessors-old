#pragma once


#include "common.hpp"
#include "accessors-common.hpp"

#include "storage.hpp"

#include "binary-tree-common.hpp"

#include "implicit-binary-tree.hpp"


namespace salgo {












namespace internal {
namespace Binary_Tree {




	//
	// FORWARD
	//
	template<
		class T,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	class Binary_Tree;




	template<
		class T,
		template<Const_Flag,class,class> class FINAL_ACCESSOR_TEMPLATE,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	struct Binary_Tree_T {

		static constexpr auto storage_type = Storage::INDEX_32;

		using Key = typename Storage::Storage_Key<storage_type>;

		using My_Binary_Tree = Binary_Tree<T, FINAL_ACCESSOR_TEMPLATE, ERASABLE, EVERSIBLE, PARENT_LINKS>;


		struct Vert;


		using T_Or_Char = std::conditional_t<std::is_same_v<T,void>, char, T>;







		//
		// parent links
		//
		template<bool, class Key> struct Vert_Add_parent            {};
		template<      class Key> struct Vert_Add_parent<true, Key> {
			Key parent;
			Vert_Add_parent() : parent(Key()) {} // can't initialize in-place - G++ bug!
		};





		//
		// parent links
		//
		template<bool, class Key> struct Acc_Add_parent            {
			template<class... Args>
			Acc_Add_parent(Args&&...) {}
		};

		template<      class Key> struct Acc_Add_parent<true, Key> {
			Key parent;

			template<class... Args>
			Acc_Add_parent(Args&&... args) : parent(std::forward<Args>(args)... ) {}
		};





		template<Const_Flag C, class OWNER, class BASE>
		class Accessor_Template;



		template<Const_Flag C, class OWNER, class BASE, bool child, int ith = 0>
		class Link {
		private:
			using Accessor = Accessor_Template<C,OWNER,BASE>;

		public:
			inline auto operator()() const {
				auto idx = acc().get_child_idx(ith);
				return acc()._tree[ child ? acc().val().children[idx] : acc().val().parent ];
			}

			template<class... Args>
			inline auto create(Args&&... args) {
				auto v = acc()._tree.add(std::forward<Args>(args)... );
				link(v);
				return v();
			}

			template<Const_Flag CC, bool ch = child, class = std::enable_if_t<ch>>
			inline void link(Accessor_Template<CC,OWNER,BASE>& other) {
				auto idx = acc().get_child_idx(ith);
				DCHECK_GE(idx, 0);
				DCHECK_LE(idx, 1);
				DCHECK_EQ(Key(), acc().val().children[idx]);
				acc().val().children[idx] = other.key;

				if constexpr(ERASABLE) {
					DCHECK_EQ(Key(), other.val().parent);
					other.val().parent = acc().key;
				}
			}

			inline operator bool() const {
				if constexpr(child) {
					auto idx = acc().get_child_idx(ith);
					return acc().val().children[idx] != Key();
				}
				else return acc().val().parent != Key();
			}

		public:
			Link() {}
			Link(const Accessor& a) : acc(a) {}

		private:
			const Accessor& acc;
		};




		template<Const_Flag C, class OWNER, class BASE>
		class Accessor_Template : public BASE,
				public Acc_Add_parent<PARENT_LINKS, Link<C, OWNER, BASE, false>> {

		private:
			using BASE_PARENT = Acc_Add_parent<PARENT_LINKS, Link<C, OWNER, BASE, false>>;

		private:
			template<bool child, int ith = 0>
			using MyLink = Link<C, OWNER, BASE, child, ith>;

		public:
			using Context = Const<My_Binary_Tree,C>&;
			using BASE::operator=;



		public:
			MyLink<true,0> left;
			MyLink<true,1> right;


			//template<bool B = false> // does not work in g++ (clang's fine)
			//template<class TT = T, std::enable_if_t<!std::is_same_v<TT,void>>> // doesn't work too in g++
			// g++ doesn't use templated conversion operators when it should
			// so we do the T_Or_Char trick
			operator Const<T_Or_Char,C>&() {
				static_assert(!std::is_same_v<T,void>, "there's no value to access (T==void)");
				return this->val().val;
			}

			operator const T_Or_Char&() const {
				static_assert(!std::is_same_v<T,void>, "there's no value to access (T==void)");
				return this->val().val;
			}


		private:
			inline auto get_child_idx(int child) const {
				if constexpr(EVERSIBLE) {
					return child ^ this->val().swap_children;
				}
				return child;
			}

		protected:
			Accessor_Template( Context& c, Const<OWNER,C>& o, int i)
					: BASE(o, i), BASE_PARENT(*this), left(*this), right(*this), _tree(c) {}

		private:
			Context _tree;
			friend MyLink<false>;
			friend MyLink<true,0>;
			friend MyLink<true,1>;
		};




		template<Const_Flag C, class OWNER, class BASE>
		using Aggregate_Accessor_Template = FINAL_ACCESSOR_TEMPLATE<C, OWNER, Accessor_Template<C,OWNER,BASE> >;


		using Storage = typename salgo::Storage<Vert>::BUILDER
			:: template Internal_Type <storage_type>
			:: template Accessor_Template <Aggregate_Accessor_Template>
			:: BUILD;





		using Vert_Base_swap   = Vert_Add_swap <EVERSIBLE>;
		using Vert_Base_val  = Vert_Add_val<!std::is_same_v<T,void>, T>;
		using Vert_Base_parent = Vert_Add_parent<PARENT_LINKS, Key>;

		struct Vert :
				Vert_Base_swap,
				Vert_Base_val,
				Vert_Base_parent {

			Vert() {}

			template<class... Args>
			Vert(Args&&... args) : Vert_Base_val(std::forward<Args>(args)... ) {}

			using Vert_Base_val::operator=;

			std::array< Key, 2> children = {{Key(), Key()}};
		};



	}; // Binary_Tree_T







	//
	// BINARY TREE
	//
	template<
		class T,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	class Binary_Tree : public Binary_Tree_T<T, ACCESSOR_TEMPLATE, ERASABLE, EVERSIBLE, PARENT_LINKS>::Storage {
	private:
		using BASE = typename Binary_Tree_T<T, ACCESSOR_TEMPLATE, ERASABLE, EVERSIBLE, PARENT_LINKS>::Storage;

	public:
		using Value = T;
		static constexpr auto Implicit     = false;
		static constexpr auto Erasable     = ERASABLE;
		static constexpr auto Eversible    = EVERSIBLE;
		static constexpr auto Parent_Links = PARENT_LINKS;

	public:
		Binary_Tree() : BASE(*this) {}
	};






	//
	// BUILDER
	//
	template<
		class T,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE,
		bool IMPLICIT,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	class Builder {
	public:
		using BUILD = std::conditional_t< IMPLICIT,
			Implicit_Binary_Tree<T, ACCESSOR_TEMPLATE, ERASABLE, EVERSIBLE>,
			Binary_Tree<T, ACCESSOR_TEMPLATE, ERASABLE, EVERSIBLE, PARENT_LINKS>
		>;

		template<template<Const_Flag,class,class> class NEW_ACCESSOR_TEMPLATE>
		using Accessor_Template = Builder<T, NEW_ACCESSOR_TEMPLATE, IMPLICIT, ERASABLE, EVERSIBLE, PARENT_LINKS>;

		using Implicit     = Builder<T, ACCESSOR_TEMPLATE, true,     ERASABLE, EVERSIBLE, PARENT_LINKS>;
		using Linked       = Builder<T, ACCESSOR_TEMPLATE, false,    ERASABLE, EVERSIBLE, PARENT_LINKS>;

		using Erasable     = Builder<T, ACCESSOR_TEMPLATE, IMPLICIT, true,     EVERSIBLE, PARENT_LINKS>;
		using Eversible    = Builder<T, ACCESSOR_TEMPLATE, IMPLICIT, ERASABLE, true,      PARENT_LINKS>;
		using Parent_Links = Builder<T, ACCESSOR_TEMPLATE, IMPLICIT, ERASABLE, EVERSIBLE, true>;
	};



} // namespace Binary_Tree
} // namespace internal













//
// W/O BUILDER
//
template<
	class T = void
>
class Binary_Tree : public internal::Binary_Tree::Binary_Tree<
	T,
	internal::Index_Accessor_Template,
	//false, // implicit
	true,  // erasable
	true,  // eversible
	true   // parent links
> {
private:
	using _BASE = internal::Binary_Tree::Binary_Tree<
		T,
		internal::Index_Accessor_Template,
		//false, // implicit
		true,  // erasable
		true,  // eversible
		true   // parent links
	>;

public:
	using BUILDER = internal::Binary_Tree::Builder<
		T,
		internal::Index_Accessor_Template,
		false, // implicit
		false, // erasable
		false, // eversible
		false  // parent links
	>;
};













} // namespace salgo



