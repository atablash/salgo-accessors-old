#pragma once


namespace salgo {
namespace internal {
namespace Splay_Map {

template<
	class KEY,
	class VAL,
	class COMP = std::less<KEY>,
	template<Const_Flag,class> class FINAL_ACCESSOR_TEMPLATE
>
class Splay_Map {
public:
	using Key = KEY;
	using Val = VAL;
	using Comp = COMP;

private:
	template<Const_Flag C, class OWNER, class BASE>
	class Accessor_Template : public BASE {
	public:
		using BASE::operator=;
		//using Context = ;

	protected:
		Accessor_Template(const Context& context, Const<OWNER,C>& o, int i) : BASE(context, o, i) {}
	};

	struct Node {
		Key key;
		Val val;
	};

	template<Const_Flag C, class OWNER, class BASE>
	using Aggregate_Accessor_Template = FINAL_ACCESSOR_TEMPLATE<C, OWNER, Accessor_Template<C,OWNER,BASE> >;

	using Binary_Tree = Binary_Tree<Node> :: BUILDER
		:: Linked
		:: Accessor_Template< Aggregate_Accessor_Template >
		:: BUILD;

	Binary_Tree tree;


public:
	auto begin() {
		return Inorder(tree).begin();
	}

	auto begin() const {
		return Inorder(tree).begin();
	}


	auto end() {
		return Inorder(tree).end();
	}

	auto end() const {
		return Inorder(tree).end();
	}
};


} // namespace Splay_Map
} // namespace internal




template<class KEY, class COMP = std::less<KEY>>
using Splay_Set = Splay_Map<KEY, void, COMP>;





} // namespace salgo



