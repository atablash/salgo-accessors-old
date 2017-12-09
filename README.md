[![Build Status](https://travis-ci.org/atablash/salgo.svg?branch=master)](https://travis-ci.org/atablash/salgo)

# Why?

* Easy to use modern C++ interface
* Algorithms separated from data structures using templates
* Although mostly not profiled, it's designed with performance in mind:
	* Many parameters are template
	* `vptr`s are avoided

# Using the library

## Accessors

Accessors, or views (or proxy objects), are objects that provide a simple interface for accessing underlying raw structures.

This concept is somewhat present in the standard C++ library too, for pseudo-reference objects returned by `std::bitset<N>::operator[]` and `std::vector<bool>::operator[]`.

Where possible, data is exposed using member references instead of member functions, to minimize number of `()`s in code.

```cpp
	Dense_Map<double> dm;
	dm[-2] = 31;
	dm[ 3] = 12;
	dm[ 4] = 23;

	for(auto e : dm) {		// note it's not `auto&`
		e.val = 123;
		cout << e.key << " -> " << e.val << endl;
	}
```

Note that accessors are constructed on demand and passed **by value**, like in the above `for`-loop. This means you can't write `for(auto& e : dm)`.

However, you can use `for(const auto& e : dm)` normally, to mark *const*-ness of the referenced element.

### Story behind accessors

Let's talk about **pointers** in *C*.

*Pointing* object *const*-ness is different than *pointed* object *const*-ness:
 * `const int*` (or `int const*`) is a pointer to `const int`
 * `int* const` is a *const* pointer to `int`

The latter (a *const* pointer) comes in a different semantic form in `C++`: a **reference**. This allows using a declared *reference*'s name as an alias for the pointed object.
 * Note that a *reference* is not only an alias, but also has an associated storage, just like a *pointer* (unless optimized-out by compiler of course). However, contrary to pointers, this storage is not exposed to programmer, and as one consequence it's always *const* conceptually.

Just like *references*, we want to use *accessors* as aliases for underlying objects. So *accessors* pretend to *be* these underlying objects.
 * However (contrary to *references*) *accessors* expose an **interface** to these underlying objects.
 * Additionally, they often store some **context** to keep track of related/parent objects.

### Accessor *const*-ness




Since

Accessors are a bit like references conceptually, but since there's no such concept in *C++*, they're treated like pointers.

Think about accessors as pointers. (not references, because these are always *const* anyway)


However, it's useful

# Dense_Map

TODO

# Binary_Tree

TODO
