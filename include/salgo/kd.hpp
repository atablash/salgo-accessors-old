#pragma once


#include "binary-tree.hpp"
#include "get-aabb.hpp"


namespace salgo {






namespace internal {
namespace Kd {




	//
	// val or void
	//
	template<bool, class VAL> struct Add_val { VAL val = VAL(); };
	template<      class VAL> struct Add_val<false, VAL> {};


	template<bool, class VAL, Const_Flag C> struct Add_val_proxy {
		Proxy<VAL,C> val;
		Add_val_proxy(Const<VAL,C>* new_val) : val(new_val) {}
	};
	template<      class VAL, Const_Flag C> struct Add_val_proxy<false, VAL, C> {
		Add_val_proxy(Const<VAL,C>*) {}
	};


	template<bool> struct Add_exists { bool exists = true; };
	template<>     struct Add_exists <false> {};




	template<
		class SCALAR, int DIM, class KEY, class VAL,
		//template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE
	>
	struct Context {

		static constexpr bool HAS_VAL = !std::is_same_v<VAL,void>;

		// `exists` is added here instead of in the underlying Dense_Map's `exists` flag
		// because erased Kd nodes are still normal underlying tree nodes
		struct Node : Add_val< HAS_VAL, VAL >, Add_exists<ERASABLE> {
			KEY key;
			SCALAR center;
			SCALAR l_to;
			SCALAR r_fr;
		};



		// forward declaration
		class Kd;



		// for accessing Node
		template<Const_Flag C>
		class Accessor : public Add_val_proxy<HAS_VAL, VAL, C> {
		public:
			const bool exists;

			Proxy<KEY,C> key;

			void erase() {
				static_assert(C == MUTAB, "erase() called on CONST Accessor");

				DCHECK(exists);
				DCHECK(node->exists); // not already erased

				node->exists = false;

				node->key = KEY();
				node->val = VAL();
			}

		private:
			inline VAL* val_or_null(Const<Node,C>* a_node) const {

				(void)a_node; // unused variable warning

				if constexpr(HAS_VAL) return a_node != nullptr ? a_node->val : nullptr;
				return nullptr;
			}

		private:
			Accessor(Const<Node,C>* a_node) :
				Add_val_proxy<HAS_VAL, VAL, C>( val_or_null(a_node) ),
				exists( a_node != nullptr ),
				key( exists ? a_node->key : *(KEY*)nullptr ),
				node( a_node ) {}
			friend Kd;

		private:
			Const<Node,C>* node;
		};



		//
		// the actual Kd object
		//
		class Kd {
			using Scalar = SCALAR;
			static constexpr int Dim = DIM;
			using Val = VAL;
			static constexpr bool Has_Val = HAS_VAL;

			//using Accessor_Template = ACCESSOR_TEMPLATE;
			static constexpr bool Erasable = ERASABLE;

		public:
			template<class NEW_KEY>
			Accessor<MUTAB> add(NEW_KEY&& key) {
					
				// root doesn't exist yet
				if(root == typename Bt::Key()) {
					auto new_root = bt.add();
					root = new_root.key;

					auto& node = new_root.val();
					node.key = key;
					// node.val auto default initialized in-class

					node.center = node.l_to = node.r_fr = get_aabb(key).center()[0];

					return Accessor<MUTAB>(&node);
				}

				typename Bt::Key bt_key;

				if constexpr(Has_Val) {
					bt_key = _add( std::forward<NEW_KEY>(key), Val(),   root, 0 );
				}
				else {
					bt_key = _add( std::forward<NEW_KEY>(key), nullptr, root, 0 );
				}

				return Accessor<MUTAB>( &bt[bt_key].val() );
			}



			template<class NEW_KEY, class NEW_VAL>
			Accessor<MUTAB> add(NEW_KEY&& key, NEW_VAL&& val) {
				static_assert(Has_Val, "add(key,val) called on Kd without Val (Val==void)");

				// root doesn't exist yet
				if(root == Bt::Key()) {
					auto new_root = bt.add();
					root = new_root.key;

					auto& node = new_root.val();
					node.key = std::move(key);
					node.val = std::move(val);
					node.center = node.l_to = node.r_fr = get_aabb(node.key).center()[0];

					return Accessor<MUTAB>(&node);
				}

				auto bt_key = _add( std::forward<NEW_KEY>(key), std::forward<NEW_VAL>(val), root, 0 );
				return Accessor<MUTAB>( &bt[bt_key].val() );
			}



			template<class SHAPE, class FUN>
			void each_intersect(SHAPE&& shape, FUN&& fun) {

				// root doesn't exist yet
				if(root == typename Bt::Key()) {
					return;
				}

				_each_intersect( std::forward<SHAPE>(shape), std::forward<FUN>(fun), root, 0 );
			}









		private:
			using Bt = typename salgo::Binary_Tree<Node> :: BUILDER :: BUILD;

			Bt bt;
			typename Bt::Key root;








		private:
			template<class NEW_KEY, class NEW_VAL, class LINK>
			inline typename Bt::Key _add_at_link(NEW_KEY&& key, NEW_VAL&& val, LINK link, int next_axis) {
				if(link) {
					// TODO: if node is erased, can insert value into it
					return _add(std::forward<NEW_KEY>(key), std::forward<NEW_VAL>(val),
						link().key, next_axis);
				}
				else {
					auto tree_node = link.create();
					auto& new_node = tree_node.val();
					new_node.key = std::move(key);
					if constexpr(Has_Val) new_node.val = std::move(val);
					new_node.center = new_node.l_to = new_node.r_fr = get_aabb(new_node.key).center()[0];

					return tree_node.key;
				}
			}

			template<class NEW_KEY, class NEW_VAL>
			typename Bt::Key _add(NEW_KEY&& key, NEW_VAL&& val, typename Bt::Key where, const int axis) {
				const int next_axis = (axis + 1) % Dim;

				auto acc = bt[where];

				auto& node = acc.val();

				const auto new_aabb = get_aabb(key);

				const auto l_extend = new_aabb.max()[axis] - node.center;
				const auto r_extend = node.center - new_aabb.min()[axis];

				if(l_extend < r_extend) {
					node.l_to = std::max(node.l_to, new_aabb.max()[axis]);
					return _add_at_link( std::forward<NEW_KEY>(key), std::forward<NEW_VAL>(val), acc.left, next_axis );
				}
				else {
					node.r_fr = std::min(node.r_fr, new_aabb.min()[axis]);
					return _add_at_link( std::forward<NEW_KEY>(key), std::forward<NEW_VAL>(val), acc.right, next_axis );
				}
			}



			template<class SHAPE, class FUN>
			void _each_intersect(const SHAPE& shape, const FUN& fun, typename Bt::Key where, const int axis) {
				const int next_axis = (axis + 1) % Dim;

				auto acc = bt[where];

				auto& node = acc.val();
				
				bool exists = true;
				if constexpr(ERASABLE) exists = node.exists;

				if( exists && get_aabb(shape).intersects(get_aabb( node.key )) ) {
					fun( Accessor<MUTAB>(&node) );
				}

				if( acc.right && node.r_fr <= get_aabb(shape).max()[axis] ) {
					_each_intersect( shape, fun, acc.right().key, next_axis );
				}

				if( acc.left && node.l_to >= get_aabb(shape).min()[axis] ) {
					_each_intersect( shape, fun, acc.left().key, next_axis );
				}
			}

		};



		//
		// BUILDER
		//
		class Builder {
		public:
			using BUILD = Kd;

			template<class NEW_SCALAR>
			using Scalar =
				typename Context<NEW_SCALAR, DIM, KEY, VAL, ERASABLE>::Builder;

			template<int NEW_DIM>
			using Dim =
				typename Context<SCALAR, NEW_DIM, KEY, VAL, ERASABLE>::Builder;

			template<class NEW_KEY>
			using Key =
				typename Context<SCALAR, DIM, NEW_KEY, VAL, ERASABLE>::Builder;

			template<class NEW_VAL>
			using Val =
				typename Context<SCALAR, DIM, KEY, NEW_VAL, ERASABLE>::Builder;

			//template<template<Const_Flag,class> class NEW_ACCESSOR_TEMPLATE>
			//using Accessor_Template =
			//	Builder<SCALAR, DIM, KEY, VAL, NEW_ACCESSOR_TEMPLATE, ERASABLE>;

			using Erasable =
				typename Context<SCALAR, DIM, KEY, VAL, true>::Builder;
		};

	};


	






} // namespace Kd
} // namespace internal







//
// main definition
// with the default values for both pure object and the builder
//
template<class SCALAR, int DIM>
class Kd : public internal::Kd::Context<
	SCALAR,
	DIM,
	Eigen::Matrix<SCALAR, DIM, 1>, // KEY
	void, // VAL
	//internal::Index_Accessor_Template, // ACCESSOR_TEMPLATE
	true // ERASABLE
>::Kd {
private:
	using _BASE = typename internal::Kd::Context<
		SCALAR,
		DIM,
		Eigen::Matrix<SCALAR, DIM, 1>, // KEY
		void, // VAL
		//internal::Index_Accessor_Template, // ACCESSOR_TEMPLATE
		true // ERASABLE
	>::Kd;

public:
	template<class... Args>
	Kd(Args&&... args) : _BASE(std::forward<Args>(args)... ) {}

	using BUILDER = typename internal::Kd::Context<
		SCALAR,
		DIM,
		Eigen::Matrix<SCALAR, DIM, 1>, // KEY,
		void, // VAL
		//internal::Index_Accessor_Template, // ACCESSOR_TEMPLATE
		false // ERASABLE
	>::Builder;
};








} // namespace salgo

