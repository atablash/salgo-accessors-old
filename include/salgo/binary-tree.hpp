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
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		class AGGREG,
		class PROPAG,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	class Binary_Tree;




	template<
		class T,
		template<Const_Flag,class> class FINAL_ACCESSOR_TEMPLATE,
		class AGGREG,
		class PROPAG,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	struct Context {

		static constexpr auto storage_type = Storage::INDEX_32;

		using Key = typename Storage::Storage_Key<storage_type>;

		using My_Binary_Tree = Binary_Tree<T, FINAL_ACCESSOR_TEMPLATE, AGGREG, PROPAG, ERASABLE, EVERSIBLE, PARENT_LINKS>;


		struct Vert;


		using T_Or_Char = std::conditional_t<std::is_same_v<T,void>, char, T>;

		static constexpr bool HAS_VAL = !std::is_same_v<T,void>;






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





		//
		// val proxy
		//
		template<bool, class VAL, Const_Flag C> struct Add_val_proxy {
			Proxy<VAL,C> val;
			Add_val_proxy(Const<VAL,C>* new_val) : val(*new_val) {}
		};
		template<      class VAL, Const_Flag C> struct Add_val_proxy<false, VAL, C> {
			Add_val_proxy(Const<VAL,C>*) {}
		};





		template<Const_Flag C, class BASE>
		class Accessor_Template;



		template<Const_Flag C, class BASE, bool child, int ith = 0>
		class Link {
		private:
			using Accessor = Accessor_Template<C,BASE>;

		public:
			inline auto operator()() const {
				if constexpr(EVERSIBLE && child) {
					DCHECK(acc().tree_node().evert == false) << "need to evert_propagate first";
				}

				if constexpr(child) return acc().tree[ acc().tree_node().children[ith] ];
				else return acc().tree[ acc().tree_node().parent ];
			}

			template<class... Args>
			inline auto create(Args&&... args) {
				auto v = acc().tree.add(std::forward<Args>(args)... );
				link(v);
				return v();
			}

			template<Const_Flag CC, bool ch = child, class = std::enable_if_t<ch>>
			inline void link(Accessor_Template<CC,BASE>& other) {
				if constexpr(EVERSIBLE) {
					DCHECK(acc().tree_node().evert == false) << "need to evert_propagate first";
				}

				DCHECK_EQ(Key(), acc().tree_node().children[ith]);
				acc().tree_node().children[ith] = other.key;

				if constexpr(PARENT_LINKS) {
					DCHECK_EQ(Key(), other.tree_node().parent);
					other.tree_node().parent = acc().key;
				}
			}

			inline void unlink() {
				DCHECK(*this) << "already not linked";

				if constexpr(child) {
					if constexpr(PARENT_LINKS) {
						DCHECK_EQ(acc().key, (*this)().tree_node().parent)
							<< "child does not link back to us";

						(*this)().tree_node().parent = Key();
					}

					if constexpr(EVERSIBLE && child) {
						DCHECK(acc().tree_node().evert == false) << "need to evert_propagate first";
					}

					acc().tree_node().children[ith] = Key();
				}
				else { // parent
					auto parent = (*this)();
					DCHECK( parent.evert == false )
						<< "need to evert_propagate from the parent first";

					if(acc().is_left) {
						DCHECK( parent.left == acc ) << "parent does not link back to us";
						parent.left.unlink();
					}
					else {
						DCHECK( parent.right == acc) << "parent does not link back to us";
						parent.right.unlink();
					}
				}
			}

			inline operator bool() const {
				if constexpr(child) {
					if constexpr(EVERSIBLE) {
						DCHECK(acc().tree_node().evert == false) << "need to evert_propagate first";
					}
					return acc().tree_node().children[ith] != Key();
				}
				else return acc().tree_node().parent != Key();
			}

		private:
			Link(const Accessor& a) : acc(a) {}
			friend Accessor;
			friend Acc_Add_parent<true,Link>;

		private:
			const Accessor& acc;
		};




		template<Const_Flag C, class BASE, int ith>
		class Link_Test {
		private:
			using Accessor = Accessor_Template<C,BASE>;

		public:
			inline operator bool() const {
				DCHECK( acc().parent ) << "is_left or is_right called on a parent-less vertex";
				DCHECK( acc().parent().tree_node().evert == false)
					<< "need to evert_propagate from the parent vertex first";
				return acc().parent().tree_node().children[ith] == acc().key;
			}

		private:
			Link_Test(const Accessor& a) : acc(a) {}
			friend Accessor;

		private:
			const Accessor& acc;
		};




		template<Const_Flag C, class BASE>
		class Accessor_Template : public BASE,
				public Acc_Add_parent<PARENT_LINKS, Link<C, BASE, false>>,
				public Add_val_proxy<HAS_VAL, T, C> {

		private:
			using BASE_PARENT = Acc_Add_parent<PARENT_LINKS, Link<C, BASE, false>>;
			using BASE_VAL_PROXY = Add_val_proxy<HAS_VAL, T, C>;

			// since the real T value is encapsulated in tree node, expose the T as val
		public:
			using std::conditional_t<HAS_VAL, BASE_VAL_PROXY, BASE>::val;

			// ...and expose aggregate tree node structure as tree_node, only for private use
		private:
			Proxy<Vert,C> tree_node;


		public:
			static constexpr auto Eversible = EVERSIBLE;

		private:
			template<bool child, int ith = 0>
			using My_Link = Link<C, BASE, child, ith>;

			template<int ith>
			using My_Link_Test = Link_Test<C, BASE, ith>;

		public:
			using Context = Const<My_Binary_Tree,C>&;
			using BASE::operator=;
			using typename BASE::Owner;

		public:
			My_Link<true,0> left;
			My_Link<true,1> right;

			My_Link_Test<0> is_left;
			My_Link_Test<1> is_right;


			void aggregate() {
				using A = decltype( BASE::val().aggreg );
				using P = decltype( BASE::val().propag );

				A a = A();
				a.aggregate( BASE::val().val );
				if(left)  a.aggregate(  left().val().aggreg );
				if(right) a.aggregate( right().val().aggreg );

				DCHECK_EQ(P(), BASE::val().propag) << "need to propagate first";

				BASE::val().aggreg = a;
			}


			void propagate() {
				using P = decltype( BASE::val().propag );

				if(left) {
					BASE::val().propag.apply( left().val().aggreg );
					BASE::val().propag.apply( left().val().propag );
				}

				if(right) {
					BASE::val().propag.apply( right().val().aggreg );
					BASE::val().propag.apply( right().val().propag );
				}

				BASE::val().propag = P();
			}


			void evert() {
				static_assert(EVERSIBLE, "can't evert, tree not EVERSIBLE");

				//DCHECK(BASE::val().evert == false) << "subtree already everted";
				BASE::val().evert ^= 1; // toggle
			}


			void evert_propagate() {
				if constexpr(EVERSIBLE) if(BASE::val().evert) {
					BASE::val().evert = false;
					std::swap(BASE::val().children[0], BASE::val().children[1]);

					if(left)  left ().evert();
					if(right) right().evert();
				}
			}


			//template<bool B = false> // does not work in g++ (clang's fine)
			//template<class TT = T, std::enable_if_t<!std::is_same_v<TT,void>>> // doesn't work too in g++
			// g++ doesn't use templated conversion operators when it should
			// so we do the T_Or_Char trick
			operator Const<T_Or_Char,C>&() {
				static_assert(!std::is_same_v<T,void>, "there's no value to access (T==void)");
				return BASE::val().val;
			}

			operator const T_Or_Char&() const {
				static_assert(!std::is_same_v<T,void>, "there's no value to access (T==void)");
				return BASE::val().val;
			}


		private:
			inline auto get_child_idx(int child) const {
				if constexpr(EVERSIBLE) {
					return child ^ BASE::val().evert;
				}
				return child;
			}

		private:
			T* val_or_null() {
				if constexpr(HAS_VAL) return &BASE::val().val;
				else return nullptr;
			}

		protected:
			Accessor_Template( Context& c, Const<Owner,C>& o, int i)
					: BASE(o, i),
					BASE_PARENT( *this ),
					Add_val_proxy<HAS_VAL, T, C>( val_or_null() ),
					tree_node( BASE::val() ),
					left( *this ),
					right( *this ),
					is_left( *this ),
					is_right( *this ),
					tree( c ) {}

		public:
			Context tree;

		private:
			friend My_Link<false>;
			friend My_Link<true,0>;
			friend My_Link<true,1>;
		};




		template<Const_Flag C, class BASE>
		using Aggregate_Accessor_Template = FINAL_ACCESSOR_TEMPLATE<C, Accessor_Template<C,BASE> >;


		using Storage = typename salgo::Storage<Vert>::BUILDER
			:: template Internal_Type <storage_type>
			:: template Accessor_Template <Aggregate_Accessor_Template>
			:: BUILD;





		using Vert_Base_swap   = Vert_Add_evert <EVERSIBLE>;
		using Vert_Base_val    = Vert_Add_val<HAS_VAL, T>;
		using Vert_Base_parent = Vert_Add_parent<PARENT_LINKS, Key>;
		using Vert_Base_aggreg = Vert_Add_aggreg<!std::is_same_v<AGGREG,void>, AGGREG>;
		using Vert_Base_propag = Vert_Add_propag<!std::is_same_v<PROPAG,void>, PROPAG>;

		struct Vert :
				Vert_Base_swap,
				Vert_Base_val,
				Vert_Base_parent,
				Vert_Base_aggreg,
				Vert_Base_propag {

			Vert() {}

			template<class... Args>
			Vert(Args&&... args) : Vert_Base_val(std::forward<Args>(args)... ) {}

			using Vert_Base_val::operator=;

			std::array< Key, 2> children = {{Key(), Key()}};
		};



	}; // Context







	//
	// BINARY TREE
	//
	template<
		class T,
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		class AGGREG,
		class PROPAG,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	class Binary_Tree :
			public Context<
				T, ACCESSOR_TEMPLATE, AGGREG, PROPAG,
				ERASABLE, EVERSIBLE, PARENT_LINKS
			>::Storage {

	private:
		using BASE = typename Context<
			T, ACCESSOR_TEMPLATE, AGGREG, PROPAG,
			ERASABLE, EVERSIBLE, PARENT_LINKS
		>::Storage;

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
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		class AGGREG,
		class PROPAG,
		bool IMPLICIT,
		bool ERASABLE,
		bool EVERSIBLE,
		bool PARENT_LINKS
	>
	class Builder {
	public:
		using BUILD = std::conditional_t< IMPLICIT,
			Implicit_Binary_Tree<T, ACCESSOR_TEMPLATE, /*AGGREG, PROPAG,*/ ERASABLE, EVERSIBLE>, // TODO
			Binary_Tree<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, ERASABLE, EVERSIBLE, PARENT_LINKS>
		>;

		template<template<Const_Flag,class> class NEW_ACCESSOR_TEMPLATE>
		using Accessor_Template =
			Builder<T, NEW_ACCESSOR_TEMPLATE, AGGREG, PROPAG, IMPLICIT, ERASABLE, EVERSIBLE, PARENT_LINKS>;

		using Implicit =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, true,     ERASABLE, EVERSIBLE, PARENT_LINKS>;

		using Linked =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, false,    ERASABLE, EVERSIBLE, PARENT_LINKS>;


		template<class NEW_AGGREG>
		using Aggregate =
			Builder<T, ACCESSOR_TEMPLATE, NEW_AGGREG, PROPAG, IMPLICIT, ERASABLE, EVERSIBLE, PARENT_LINKS>;

		template<class NEW_PROPAG>
		using Propagate =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, NEW_PROPAG, IMPLICIT, ERASABLE, EVERSIBLE, PARENT_LINKS>;




		//
		// erasable
		//
		using Erasable =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, IMPLICIT, true,         EVERSIBLE, PARENT_LINKS>;

		template<bool NEW_ERASABLE>
		using Set_Erasable =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, IMPLICIT, NEW_ERASABLE, EVERSIBLE, PARENT_LINKS>;


		//
		// eversible
		//
		using Eversible =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, IMPLICIT, ERASABLE, true,          PARENT_LINKS>;

		template<bool NEW_EVERSIBLE>
		using Set_Eversible =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, IMPLICIT, ERASABLE, NEW_EVERSIBLE, PARENT_LINKS>;


		//
		// parent links
		//
		using Parent_Links =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, IMPLICIT, ERASABLE, EVERSIBLE, true>;

		template<bool NEW_PARENT_LINKS>
		using Set_Parent_Links =
			Builder<T, ACCESSOR_TEMPLATE, AGGREG, PROPAG, IMPLICIT, ERASABLE, EVERSIBLE, NEW_PARENT_LINKS>;
	};



} // namespace Binary_Tree
} // namespace internal













template<
	class T = void
>
class Binary_Tree : public internal::Binary_Tree::Binary_Tree<
	T,
	internal::Index_Accessor_Template,
	void,  // aggreg
	void,  // propag
	//false, // implicit
	true,  // erasable
	true,  // eversible
	true   // parent links
> {
private:
	using _BASE = internal::Binary_Tree::Binary_Tree<
		T,
		internal::Index_Accessor_Template,
		void,  // aggreg
		void,  // propag
		//false, // implicit
		true,  // erasable
		true,  // eversible
		true   // parent links
	>;

public:
	using BUILDER = internal::Binary_Tree::Builder<
		T,
		internal::Index_Accessor_Template,
		void,  // aggreg
		void,  // propag
		false, // implicit
		false, // erasable
		false, // eversible
		false  // parent links
	>;
};













} // namespace salgo



