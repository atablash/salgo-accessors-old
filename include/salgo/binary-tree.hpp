#pragma once


#include "common.hpp"
#include "accessors-common.hpp"

#include "storage.hpp"

#include "binary-tree-common.hpp"

#include "implicit-binary-tree.hpp"


namespace salgo {












namespace internal {





	//
	// FORWARD
	//
	template<
		class T,
		Binary_Tree_Flags FLAGS,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE
	>
	class Binary_Tree;




	template<class T, Binary_Tree_Flags FLAGS, template<Const_Flag,class,class> class FINAL_ACCESSOR_TEMPLATE>
	struct Binary_Tree_T {

		static constexpr auto storage_type = INDEX_32;

		using Key = typename internal::Storage_Key<storage_type>;

		using My_Binary_Tree = Binary_Tree<T, FLAGS, FINAL_ACCESSOR_TEMPLATE>;


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

				if constexpr(bool(FLAGS & BT_VERTS_ERASABLE)) {
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
				public Acc_Add_parent<bool(FLAGS & PARENT_LINKS), Link<C, OWNER, BASE, false>> {

		private:
			using BASE_PARENT = Acc_Add_parent<bool(FLAGS & PARENT_LINKS), Link<C, OWNER, BASE, false>>;

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
				if constexpr(bool(FLAGS & EVERSIBLE)) {
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


		using Storage = typename Storage_Builder<Vert>
			:: template Type <storage_type>
			:: template Accessor_Template <Aggregate_Accessor_Template>
			:: template Flags <Storage_Flags::NONE> // not erasable - binary tree is erasable anyway
			:: Build;





		using Vert_Base_swap   = Vert_Add_swap <bool(FLAGS & EVERSIBLE)>;
		using Vert_Base_val  = Vert_Add_val<!std::is_same_v<T,void>, T>;
		using Vert_Base_parent = Vert_Add_parent<bool(FLAGS & PARENT_LINKS), Key>;

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
		Binary_Tree_Flags FLAGS,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE
	>
	class Binary_Tree : public internal::Binary_Tree_T<T, FLAGS, ACCESSOR_TEMPLATE>::Storage {
	private:
		using BASE = typename internal::Binary_Tree_T<T, FLAGS, ACCESSOR_TEMPLATE>::Storage;

	public:
		using Value = T;
		static constexpr auto Flags = FLAGS;

	public:
		Binary_Tree() : BASE(*this) {}
	};






} // namespace internal













//
// W/O BUILDER
//
template<class T>
using Binary_Tree = internal::Binary_Tree<
	T,
	internal::default_Binary_Tree_Flags,
	internal::Default__Binary_Tree_Accessor_Template
>;




//
// BUILDER
//
template<
	class T = void,
	Binary_Tree_Flags FLAGS = internal::default_Binary_Tree_Flags,
	template<internal::Const_Flag,class,class> class ACCESSOR_TEMPLATE = internal::Default__Binary_Tree_Accessor_Template
>
class Binary_Tree_Builder {
public:
	using Build = std::conditional_t<bool(FLAGS & IMPLICIT),
		internal::Implicit_Binary_Tree<T, FLAGS, ACCESSOR_TEMPLATE>,
		internal::Binary_Tree<T, FLAGS, ACCESSOR_TEMPLATE>
	>;

	//


	template<template<internal::Const_Flag,class,class> class NEW_ACCESSOR_TEMPLATE>
	using Accessor_Template = Binary_Tree_Builder<T, FLAGS, NEW_ACCESSOR_TEMPLATE>;


	template<Binary_Tree_Flags NEW_FLAGS>
	using Flags           = Binary_Tree_Builder<T, NEW_FLAGS, ACCESSOR_TEMPLATE>;

	template<Binary_Tree_Flags NEW_FLAGS>
	using Add_Flags       = Binary_Tree_Builder<T, FLAGS |  NEW_FLAGS, ACCESSOR_TEMPLATE>;

	template<Binary_Tree_Flags NEW_FLAGS>
	using Rem_Flags       = Binary_Tree_Builder<T, FLAGS & ~NEW_FLAGS, ACCESSOR_TEMPLATE>;
};









} // namespace salgo



