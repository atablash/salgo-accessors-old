#pragma once

#include "binary-tree-common.hpp"

namespace salgo {








//
// IMPLICIT BINARY TREE
//
template<
	class T = void,
	Binary_Tree_Flags FLAGS = internal::default_Binary_Tree_Flags,
	template<Const_Flag,class,class> class FINAL_ACCESSOR_TEMPLATE = internal::Default__Binary_Tree_Accessor_Template
>
class Implicit_Binary_Tree {
private:
	struct Vert;

public:
	using Mapped_Type = T;
	static constexpr auto Flags = FLAGS;






	auto root()       { return verts[1]; }
	auto root() const { return verts[1]; }







	auto begin()       { return verts.begin(); }
	auto begin() const { return verts.begin(); }

	auto end()       { return verts.end(); }
	auto end() const { return verts.end(); }











	//
	// ACCESSOR (BASE)
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
		struct Link {
			inline auto operator()() const {
				(void) DCHECK_NOTNULL(owner); // (void) for clang's unused warning
				return (*owner)[ key ];
			}

			inline operator bool() const {
				(void) DCHECK_NOTNULL(owner); // (void) for clang's unused warning
				return key != typename OWNER::Key() && (*this)().exists;
			}

		public:
			Link() {}
			Link(Const<OWNER,C>& o, typename OWNER::Key k) : owner(&o), key(k) {}

		private:
			Const<OWNER,C>* owner = nullptr;
			typename OWNER::Key key = typename OWNER::Key();
		};

	public:
		Link parent, left, right;

	private:
		inline auto get_child_idx(int child) {
			if constexpr(bool(Flags & EVERSIBLE)) return child ^ this->raw.swap_children;
			return child;
		}

	public:
		operator Const<Mapped_Type,C>&() const {
			return this->raw.value;
		}


	protected:
		Accessor_Template( Const<OWNER,C>& o, int i) : BASE(o, i),
			parent(o,  i >> 1),
			left  (o, (i << 1)+get_child_idx(0)),
			right (o, (i << 1)+get_child_idx(1)) {}
	};












private:
	struct Vert :
			internal::Vert_Add_swap <bool(Flags & EVERSIBLE)>,
			internal::Vert_Add_value<!std::is_same_v<Mapped_Type,void>, Mapped_Type> {

		using internal::Vert_Add_value<!std::is_same_v<Mapped_Type,void>, Mapped_Type>::operator=;
	};


	template<Const_Flag C, class OWNER, class BASE>
	using Aggregate_Accessor_Template = typename Accessor_Template<C, OWNER, BASE>::Derived;

	using _Builder_1 = typename Dense_Map_Builder<Vert>
		:: template Type<VECTOR>
		:: template Accessor_Template<Aggregate_Accessor_Template>;

	using Dense_Map = typename std::conditional_t<bool(Flags & BT_VERTS_ERASABLE),
		typename _Builder_1 :: template Add_Flags<ERASABLE>,
		typename _Builder_1 :: template Rem_Flags<ERASABLE>
	> :: Dense_Map;

	Dense_Map verts;

public:
	template<Const_Flag C>
	using Iterator = typename Dense_Map::template Iterator<C>;

	template<Const_Flag C, class CRTP>
	using Iterator_Base = typename Dense_Map::template Iterator_Base<C,CRTP>;
};









} // namespace salgo



