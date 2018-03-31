[![Build Status](https://travis-ci.org/atablash/salgo.svg?branch=master)](https://travis-ci.org/atablash/salgo)
Salgo
=====

Why?

* Easy to use modern C++ interface
* Algorithms separated from data structures using templates
* Although mostly not profiled, it's designed with performance in mind:
	* Many parameters are template
	* `vptr`s are avoided




Reference
=========

Below is a description of some of the algorithms and data structures in Salgo.

If that's not enough, you can also use the `test` source code as a reference.



Dense_Map
---------

`Dense_Map` is a wrapper around `std::vector` or `std::deque`, with some differences:
 * It has as interface more similar to `std::map`
 * We refer to its indices as *keys*
 * It can have *negative* keys
 * It can have large keys, as long as they're close together
 * You can assign a value to a non-existing element and it's automatically created
 * It exposes all the accessor machinery of course

```cpp
	Dense_Map<int> m = {-1, 2, -3, 4, -12}; // creates elements at keys 0,1,2,3,4

	m[10] = 1; // ...and one more element at key 10

	for(auto e : m) if(e < 0) e.erase();

	int sum = 0;
	for(auto e : m) sum += e;

	cout << sum << endl; // prints 7
```


### Type Builder

By default, a full-blown object with all the functionality is constructed:

```cpp
	Dense_Map<int> m;
```

If you want a custom object, use the `BUILDER`:

```cpp
	Dense_Map<int>::BUILDER::Vector::Erasable::BUILD m;
```

The `BUILDER` supports following settings:
 * `Vector` - use the `std::vector` version of the *Dense_Map*: keys will start from 0
 * `Deque` - use the `std::deque` version of the *Dense_Map*: keys can start from arbitrary integer
 * `Erasable` - elements can be erased, leaving "holes" that still occupy memory
 * `Accessor_Template` - a template for extending the default accessor (see the relevant section)

To get the bare minimum (*Vector* version, not *Erasable*), just don't supply any options:

```cpp
	Dense_Map<int>::BUILDER::BUILD m;
```


### Construction

Without any arguments, to create an empty `Dense_Map` that will start inserting elements at key *0*:

```cpp
	Dense_Map<int> dm;
```

To start inserting elements at key *42*:

```cpp
	Dense_Map<int> dm( 42 );
```

From an initializer list:

```cpp
	Dense_Map<int> dm = {1,3,-5,100}; // will create elements at keys 0,1,2,3
```

If you use a custom derived accessor with context, you must provide the context on `Dense_Map` construction:

```cpp
	using DM = Dense_Map<int>::BUILDER::Accessor_Template< My_Custom_Accessor >::BUILD;
	DM dm( some_context );
```


### Other Members

* `size()`: returns number of elements
* ~~`front()`: returns the first element~~ (not implemented yet?)
* ~~`back()`: returns the last element~~ (not implemented yet?)


### Notes

 * Currently iteration doesn't jump over long erased elements sequences, so iterating over erased elements will still take time.
 * It can construct some of your objects before you want it. It's still not implemented properly for usage with objects.



Binary_Tree
-----------

It comes in 2 flavours:
* Regular linked binary tree
* Implicit binary tree

### Type Builder

To create a full blown linked binary tree, use:

```cpp
	Binary_Tree<int> tree;
```

To create a custom object, use the `BUILDER`:

```cpp
	Binary_Tree<int>::BUILDER::Linked::Parent_Links::BUILD tree;
```

To create a minimum object (best performance), use:

```cpp
	Binary_Tree<int>BUILDER::Linked::BUILD linked_tree;
	// or:
	Binary_Tree<int>BUILDER::Implicit::BUILD implicit_tree;
```


### Construction

TODO




Library Design
=================

The library is contained in `salgo` namespace, so either type `salgo::` explicitly every time (especially in header files or in case of name conflicts), or just use:

```cpp
	using namespace salgo;
```



Accessors
---------

Accessors, or views (or proxy objects), are objects that provide a simple interface for accessing underlying raw structures.

This concept is somewhat present in the standard C++ library too, for pseudo-reference objects returned by `std::bitset<N>::operator[]` and `std::vector<bool>::operator[]`.

```cpp
	Dense_Map<double> dm;
	dm[-2] = 3.1;
	dm[ 3] = 1.2;
	dm[ 4] = 2.3;

	for(auto e : dm) {		// note it's not `auto&`
		e = 12.3;
		cout << e.key << " -> " << e.val << endl;
	}
```

Note that accessors are constructed on demand and passed **by value**, like in the above `for`-loop. This means you can't write ~~`for(auto& e : dm)`~~.

However, you can use `for(const auto& e : dm)` normally, to mark *const*-ness of the referenced element.

In the above example, `e.val` is still not the underlying `double` or reference to it, but another proxy object. It is implicitly convertible to `double&`, but in case you need it explicit, use `operator()` on it:

```cpp
	struct S {
		int member;
	};

	Dense_Map<S> dm;

	dm[5] = S{42};

	cout << dm[5].val().member << endl;
```

> **NOTE**
>
> The reason of using a proxy object is to make Salgo *const*-correct.


### A Story Behind Accessors

Let's take a look at **pointers** in *C*.

_Pointing_ object *const*-ness is different than *pointed* object *const*-ness:
* `const int*` (or `int const*`) is a pointer to `const int`
* `int* const` is a *const* pointer to `int`

The latter (a *const* pointer) comes in a different semantic form in `C++`: a **reference**. This allows using a declared *reference*'s name as an alias for the pointed object.
 * Note that a *reference* is not only an alias, but also has an associated storage, just like a *pointer* (unless optimized-out by compiler of course). However, contrary to pointers, this storage is not exposed to programmer, and as one consequence it's always *const* conceptually.

Just like *references*, we want to use *accessors* as aliases for underlying objects. So *accessors* pretend to *be* these underlying objects. However, unlike *references*:
 * *accessors* expose some **interface** to these underlying objects
 * *accessors* are self-contained: they store a **context** to keep track of the related/parent objects

However, C++ treats accessors more like *pointers* than *references*. As a resulult, it takes some trickery to make them work like *references*.

There's another analogy: just like *STL iterators* are abstraction over *pointers*, *Salgo accessors* are abstraction over *references*. However, again, unlike *STL iterators*:
 * *accessors* expose some **interface** to the underlying *pointed* objects
 * *accessors* are self-contained: they store a **context** to keep track of the related/parent objects


### Accessor *const*-ness

Since *pointed object* *const*-ness is different from *accessor* *const*-ness, all accessor types come in two flavors:
* `A_Array_Element<MUTAB>`
* `A_Array_Element<CONST>`

The first one allows modification of the *pointed object*, while the latter does not.

*Accessors* often store some public *reference* members that pretend to be the *pointed* object's member variables. As a result of this, *accessors* are immutable conceptually (or, always *const*).

However, suppose you'd like to implement a function that takes a *const* vertex *accessor*, to forbid modifying or erasing it:

```cpp
	template<class VERTEX>
	void compute_neighbors_sum(const VERTEX& vertex) {
		// ...
	}
```

You might think this won't work the same way as *references* do, because it's not really a reference to a vertex, but rather a reference to an *accessor*.

However, Salgo passes *const*-ness of an accessor to the underlying vertex.

Generally, the *pointed object* is considered *const*, if at least one of the conditions is true:
* The *accessor* object is *const*
* Its `Const_Flag` template argument equals `CONST`


### Salgo Accessor vs Salgo Iterator

Unlike *STL iterators*, *Salgo iterators* are an implementation detail internal to the library, and shouldn't be used directly.

They exist only to support the range-based *for* loop (`for(auto e : v)`) internally.


### Extending the Default Accessors

You can extend Salgo containers' accessors with custom functionality.

Below you can see an example from the Smesh library. Dense_Map's accessor is extended to provide an interface for underlying triangle mesh vertices:

```cpp
	template<Const_Flag C, class BASE>
	class A_Vert_Template : public BASE {
	public:
		// Support assignment (handled by BASE):
		using BASE::operator=;

		// inherit Owner alias - this will be the Dense_Map type
		using typename BASE::Owner;

		// Keep a reference to the mesh object
		using Context = Const<Smesh,C>&; // keep a reference to the mesh object

		Proxy<Pos,C> pos;
		Proxy<Vert_Props,C> props;

		A_Poly_Links<C> hverts;

		// Replace the Dense_Map's erase() with a different version
		void erase() {
			// Erase adjacent polygons:
			for(auto hv : hverts) {
				hv.poly.erase();
			}

			// Finally, use the regular Dense_Map's erase()
			// to remove this vertex
			BASE::erase();
		}

		A_Vert_Template( Context m, Const<Owner,C>& o, const int i) : BASE(o, i),
				pos( o.raw(i).pos ),
				props( o.raw(i) ),
				poly_links(m, i) {}
	};
```

Here you can see the a variant of *CRTP* pattern with template injection in action: because the base class (`Dense_Map<A_Vert_Template, ...>::Accessor_Base`) has a template dependency on the derived class that we just defined, `A_Vert_Template` will be passed to Dense_Map as a template template argument, and will be instantiated there later, in the base's context.

This has other advantages too:
* `A_Vert_Template` is a template that takes a `Const_Flag` anyway (and we avoid tricks similar to std::allocator<...>::rebind)
* `A_Vert_Template` can be reused with different base accessors (as is Index_Accessor_Template)

In the above example, you can see an extension to the `Dense_Map` that exposes some additional interface:
* `pos`: a pseudo-reference to vertex position
* `props`: a pseudo-reference to vertex properties
* `hverts`: a pseudo-reference to *vertex->polygon* links list
* `erase()`: replaces the regular `Dense_Map`'s `erase()` with a version specific to our mesh structure

Template arguments are:
* `Const_Flag C`: will be instantiated with either `MUTAB` or `CONST`, for forcing pointed object (vertex in this case) *const*-ness
* `BASE`: that's our base (parent) accessor, `Dense_Map<...>::Accessor_Base` in this case

There are some helper classes / alias templates:
* `Const<class T, Const_Flag>`: resolves to either `T` or `const T`
* `Proxy<class T, Const_Flag>`: pretends to be a reference to T, but passes *const*-ness of _*this_ to *T*
	* Use `operator()` to explicitly convert to `T&`

If you don't want any extra context, remove the `using Context = ...` line, and the first argument of the constructor (`Context m`).

Note that BASE::Owner is conceptually a parent part of the accessor context. However, the design decision was to separate it from the optional user context for simplicity.

If you need multiple level accessor inheritance, see how accessor templates chaining is implemented in *Binary_Tree*. If the most derived accessor needs some additional context, it must aggregate it with the parent context, e.g. `using Context = std::pair<Child_Context, BASE::Context>`.

To use your new accessor with the *Dense_Map*:

```cpp
	using Verts_Map = Dense_Map<Vertex>::BUILDER::Accessor_Template<A_Vert_Templace>::BUILD;
	Verts_Map my_verts_map( my_mesh_context );
```


#### Template Injection

As a side note, here's a simple example of how this resolves circular template dependencies:

```cpp	
	template<class B>
	struct A {
		B* b;
	};


	template<template<class> class A_TEMPLATE>
	struct B {
		using A = A_TEMPLATE< B >;
		A* a;
	};
```

Now you can instantiate `A<B<A>>` and `B<A>` and there's no circular dependency.




Performance
===========

It's not a Python library - I know your needs.

Although there's plenty of fancy accessor machinery that definitely works slow in debug mode, the good news is that most of this stuff gets optimized out quite nicely.

Salgo is developed under *g++ 7.2.0*. It optimizes out accessors slightly better than *clang 5.0.1*. At least with the default *CMake* compiler flags for *Release*. You might want to experiment with compiler flags a bit more and let me know.

Surprisingly, a `std::vector<int>` wrapped in a `salgo::Dense_Map<int>` (without `ERASABLE` flag) is faster than a regular `std::vector<int>` by 56%. Check the `Vector_dense_map_noerase` test. How is this possible? I don't know, I haven't inspected this further, but you're welcome to investigate and let me know.

So don't worry about the performance too much. Check the `test` directory or Travis build for details about the performance of different Salgo data structures and algorithms.




Backward Compatibility and Versioning
=====================================

This project is at a very early stage of implementation, so there's no backward compatibility, and not even a notion of versions. Just pick the latest commit to `master` branch that passes the Travis build.




Contributing
============

As mentioned, Salgo is at an early stage of implementation, but hopefully will be continued and more developers will join the effort.

If you like the library and your C++ is strong, you're more than welcome to join.

When creating pull requests, make sure to include unit and performance tests for all new functionality.
