#pragma once

#include "binary-tree-common.hpp"

namespace salgo {





namespace internal {
namespace Binary_Tree {


	//
	// IMPLICIT BINARY TREE
	//
	template<
		class VALUE,
		template<Const_Flag,class,class> class FINAL_ACCESSOR_TEMPLATE,
		bool ERASABLE,
		bool EVERSIBLE
	>
	class Implicit_Binary_Tree {



	//
	// public template arguments
	//
	public:
		using Value = VALUE;
		static constexpr auto Implicit = true;
		static constexpr auto Erasable = ERASABLE;
		static constexpr auto Eversible = EVERSIBLE;
		static constexpr auto Parent_Links = true;





	//
	// Link
	//
	private:
		template<Const_Flag C, class OWNER>
		struct Link {
			auto operator()() {
				return owner[ key ];
			}

			auto operator()() const {
				return Link<CONST,OWNER>(*this).operator()();
			}

			operator bool() const {
				return key != typename OWNER::Key() && (*this)().exists;
			}

			template<class VAL>
			auto operator=(VAL&& v) {
				return (*this)() = std::forward<VAL>(v);
			}

		private:
			Link(Const<OWNER,C>& o, typename OWNER::Key k) : owner(o), key(k) {}
			friend Implicit_Binary_Tree;

		private:
			Link(const Link<MUTAB,OWNER>& o) : owner(o.owner), key(o.key) {}

		protected:
			Const<OWNER,C>& owner = nullptr;
			typename OWNER::Key key = typename OWNER::Key();
		};





	//
	// Accessor_Template   <- Link
	//
	public:
		template<Const_Flag C, class OWNER, class BASE>
		class Accessor_Template : public BASE {
		public:
			using Context = void;

		public:
			using BASE::operator=;

		public:
			using Derived = FINAL_ACCESSOR_TEMPLATE<C, OWNER, Accessor_Template>;


		private:

		public:
			Link<C, OWNER> parent, left, right;

		private:
			inline auto get_child_idx(int child) {
				if constexpr(EVERSIBLE) return child ^ this->val().swap_children;
				return child;
			}

		public:
			operator Const<Value,C>&() {
				return this->val().val;
			}

			operator const Value&() const {
				return this->val().val;
			}


		protected:
			Accessor_Template( Const<OWNER,C>& o, int i) : BASE(o, i),
				parent(o,  i >> 1),
				left  (o, (i << 1) + get_child_idx(0)),
				right (o, (i << 1) + get_child_idx(1)) {}
		};






	//
	// Dense_Map verts   <- Accessor_Template
	//
	private:
		struct Vert :
				Vert_Add_swap <EVERSIBLE>,
				Vert_Add_val<!std::is_same_v<Value,void>, Value> {

			using Vert_Add_val<!std::is_same_v<Value,void>, Value>::operator=;
		};


		template<Const_Flag C, class OWNER, class BASE>
		using Aggregate_Accessor_Template = typename Accessor_Template<C, OWNER, BASE>::Derived;

		using _Builder_1 = typename Dense_Map<Vert> :: BUILDER
			:: Vector
			:: template Accessor_Template<Aggregate_Accessor_Template>;

		using Dense_Map = typename std::conditional_t< ERASABLE,
			typename _Builder_1 :: Erasable,
			_Builder_1
		> :: BUILD;

		Dense_Map verts;











	//
	// root   <- Link
	//
	public:
		using Root_Link = Link<MUTAB,decltype(verts)>;
		Root_Link root = Root_Link(verts, 1);







	//
	// begin(), end()
	//
	public:
		auto begin()       { return verts.begin(); }
		auto begin() const { return verts.begin(); }

		auto end()       { return verts.end(); }
		auto end() const { return verts.end(); }








	public:
		template<Const_Flag C>
		using Iterator = typename Dense_Map::template Iterator<C>;

		template<Const_Flag C, class CRTP>
		using Iterator_Base = typename Dense_Map::template Iterator_Base<C,CRTP>;
	};



} // namespace Binary_Tree
} // namespace internal




} // namespace salgo



