# Why?

* Easy to use modern C++ interface
* Algorithms separated from data structures using templates
* Although mostly not profiled, it's designed with performance in mind:
	* Many parameters are template
	* `vptr`s are avoided

# Using the library

## Accessors

Accessors, or views (or proxy objects), are objects that provide a simple interface for accessing underlying raw structures.

This concept is present in the standard C++ library too, for pseudo-reference objects returned by `std::bitset<N>::operator[]` and `std::vector<bool>::operator[]`.

Where possible, data is exposed using member references instead of member functions, to minimize number of `()`s in code.

```cpp
	Dense_Map<double> dm;
	dm[-2] = 31;
	dm[ 3] = 12;
	dm[ 4] = 23;

	for(auto e : dm) {
		cout << e.key << " -> " << e.val << endl;
	}
```

Note that accessors are constructed on demand and passed **by value**, like in the above `for`-loop.

# Dense_Map

TODO

# Binary_Tree

TODO
