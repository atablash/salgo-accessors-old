#pragma once

#include <vector>




namespace salgo {


//
// rotate v once
//
// prerequisites:
// * v has parent
// * v, v.parent and v.parent.parent already have the evert flag propagated
//
template<class V>
void rotate(V& v) {
	DCHECK(v.parent);

	auto p = v.parent();
	v.parent.unlink();

	if(p.parent) {
		auto gp = p.parent();
		if(p.is_left) {
			gp.left.unlink();
			gp.left.link(v);
		}
		else {
			gp.right.unlink();
			gp.right.link(v);
		}
	}

	if(v.is_left) {
		auto b = v.right();
		v.right.unlink();
		v.right.link(p);
		p.left.link(b);
	}
	else {
		auto b = v.left();
		v.left.unlink();
		v.left.link(p);
		p.right.link(b);
	}
}



//
// splay v to the root
//
template<class V>
inline void splay(V& v) {
	if constexpr(V::Eversible)
	{
		std::vector<V::Key> path;
		V::Key key = v.key;
		for(;;) {
			path.push_back(key);
			if(v.tree[key].parent == false) break;

			key = v.tree[key].parent().key;
		}
		while(!path.empty()) v.tree[ path.back() ].evert_propagate();
	}

	while(v.parent) {
		// zig-zig
		if(v.parent().parent && v.is_left == v.parent().is_left) {
			rotate( v.parent() );
			rotate( v );
		}
		else {
			rotate( v );
		}
	}
}



} // namespace salgo


