[![Build Status](https://travis-ci.org/atablash/salgo.svg?branch=master)](https://travis-ci.org/atablash/salgo)
Salgo
=====

Why?

* Easy to use modern C++ interface
* Algorithms separated from data structures using templates
* Although mostly not profiled, it's designed with performance in mind:
	* Many parameters are template
	* `vptr`s are avoided




Using the library
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


### Story behind accessors

Let's talk about **pointers** in *C*.

*Pointing* object *const*-ness is different than *pointed* object *const*-ness:
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


### Salgo Accessor vs Salgo iterator

Unlike *STL iterators*, *Salgo iterators* are an implementation detail internal to the library, and shouldn't be used directly.

They exist only to support the range-based *for* loop (`for(auto e : v)`) internally.




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
	Dense_Map<int> m = {-1, 2, -3, 4, -12};

	for(auto e : m) if(e < 0) e.erase();

	int sum = 0;
	for(auto e : m) sum += e;

	cout << sum << endl; // prints 6
```


### Type Builder

By default, a full-blown object with all the functionality is constructed.

If you want a custom object, use the `BUILDER`:

```cpp
	Dense_Map<int>::BUILDER::Vector::Erasable::BUILD m;
```

The `BUILDER` supports following settings:
 * `Vector` - use the `std::vector` version of the *Dense_Map*: keys will start from 0
 * `Deque` - use the `std::deque` version of the *Dense_Map*: keys can start from arbitrary integer
 * `Erasable` - elements can be erased, leaving "holes" that still occupy memory

To get the bare minimum (*Vector* version, not *Erasable*), just don't supply any options:

```cpp
	Dense_Map<int>::BUILDER::BUILD m;
```


### Construction

TODO


### Notes

 * Currently iteration doesn't jump over long erased elements sequences, so iterating over erased elements will still take time.
 * It can construct some of your objects before you want it. It's still not implemented properly for usage with objects.



Binary_Tree
-----------

TODO




Performance
===========

It's not some Python library - I know your needs.

Although there's plenty of fancy accessor machinery that definitely works slow in debug mode, the good news is that most of this stuff gets optimized out quite nicely.

Salgo is developed under *g++ 7.2.0*. It optimizes out accessors slightly better than *clang 5.0.1*. At least with the default *CMake* compiler flags for *Release*. You might like to experiment with compiler flags a bit more and let me know.

Surprisingly, a `std::vector<int>` wrapped in a `salgo::Dense_Map<int>` (without `ERASABLE` flag) is faster than a regular `std::vector<int>` by 31%. Check the `Vector_dense_map_noerase` test. How is this possible? I don't know, I haven't inspected this further, but you're welcome to investigate and let me know.

So don't worry about the performance too much. Check the `test` directory or Travis build for details about the performance of different Salgo data structures and algorithms.




Backward compatibility and versioning
=====================================

This project is at a very early stage of implementation, so there's no backward compatibility, and not even a notion of versions. Just pick the latest commit to `master` branch that passes the Travis build.




Contributing
============

As mentioned, Salgo is at an early stage of implementation, but hopefully will be continued and more developers will join the effort.

If you like the library and your C++ is strong, you're more than welcome to join.

When creating pull requests, make sure to include unit and performance tests for all new functionality.
