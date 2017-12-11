#pragma once

#include "common.hpp"
#include "accessors-common.hpp"

#include <glog/logging.h>

#include <deque>


namespace salgo {













namespace internal {
namespace Dense_Map {



	enum class Type {
		VECTOR,
		DEQUE
	};



	// for Dense_Map
	template<bool> struct Add_offset { int offset = 0; };
	template<>     struct Add_offset <false> {};

	// for Dense_Map
	template<bool> struct Add_size { int _size = 0; };
	template<>     struct Add_size <false> {};


	// for Node
	template<bool> struct Add_exists { bool exists = false; };
	template<>     struct Add_exists <false> {};

	// for Dense_Map (can't use this...)
	//template<bool, class T> struct Add_Member_context { T context = T(); };
	//template<      class T> struct Add_Member_context <false,T> {};












	//
	// a vector with some elements marked as deleted - lazy deletion
	//
	// ACCESSOR_TEMPLATE: should derive from second tmeplate argument; it will be Accessor_Base
	//
	// TODO: don't call constructors too early
	//
	// TODO: implement a linked version to jump empty spaces faster
	//
	template<
		class VALUE_OR_KEY,
		class VOID_OR_VALUE,
		Type TYPE,
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE
	>
	class Dense_Map :
			private Add_offset<TYPE == Type::DEQUE>,
			private Add_size<ERASABLE> {

		// forward declarations
		public: template<Const_Flag C> class Accessor_Base;
		private: struct Node;


	public:
		using Key = std::conditional_t<std::is_same_v<VOID_OR_VALUE,void>, int, VALUE_OR_KEY>;
		using Value = std::conditional_t<std::is_same_v<VOID_OR_VALUE,void>, VALUE_OR_KEY, VOID_OR_VALUE>;

		static constexpr auto Erasable = ERASABLE;
		using Container = std::conditional_t<TYPE == Type::VECTOR, std::vector<Node>, std::deque<Node>>;

		template<Const_Flag C>
		using Accessor = ACCESSOR_TEMPLATE<C, Accessor_Base<C>>;


	private:
		static constexpr bool has_context = !std::is_same_v<typename Accessor<MUTAB>::Context,void>;

		// TODO: don't store uint8 here if no context
		using Context = std::conditional_t<has_context, typename Accessor<MUTAB>::Context, uint8_t>;
		Context context;



		//
		// helpers for creating accessors
		// (they take index, not key! there might be offset. so don't expose these functions)
		//
	private:
		template<Const_Flag C, class INT>
		inline auto create_accessor(INT _idx) {
			auto idx = int_cast<Key>(_idx);

			if constexpr(has_context) {
				return Accessor<C>(context, *this, idx);
			}
			else {
				return Accessor<C>(*this, idx);
			}
		}

		template<Const_Flag C, class INT>
		inline auto create_accessor(INT _idx) const {
			auto idx = int_cast<Key>(_idx);

			if constexpr(has_context) {
				return Accessor<C>(context, *this, idx);
			}
			else {
				return Accessor<C>(*this, idx);
			}
		}




	public:
		Dense_Map() {
			static_assert(!has_context, "you have to provide context");
		}

		Dense_Map(const std::initializer_list<Value>& l) {
			static_assert(!has_context, "you have to provide context");

			if constexpr(TYPE == Type::VECTOR) {
				_raw.reserve(l.size());
			}

			if constexpr(ERASABLE) {
				this->_size = l.size();
			}

			for(const auto& e : l) {
				_raw.push_back(e);
			}
		}

		Dense_Map(Key new_offset)
				: Add_offset<TYPE == Type::DEQUE>{new_offset} {

			static_assert(!has_context, "you have to provide context");
		}


		template<class CONTEXT>
		Dense_Map(CONTEXT&& c) : context(std::forward<CONTEXT>(c)) {
			static_assert(has_context, "doesn't use context");
		}




		decltype(auto) operator[](Key key) {
			return create_accessor<MUTAB>(key - get_offset());
		}

		decltype(auto) operator[](Key key) const {
			return create_accessor<CONST>(key - get_offset());
		}



		auto size() const {
			if constexpr(ERASABLE) return this->_size;
			else return _raw.size();
		}



		auto domain_begin() const {
			return get_offset();
		}

		auto domain_end() const {
			return get_offset() + _raw.size();
		}


		auto empty() const {
			return _raw.empty();
		}

		void reserve(Key capacity) {
			_raw.reserve(capacity);
		}

	private:
		template<class INT, class OTHER>
		inline static INT int_cast(const OTHER& other) {
			DCHECK_GE(other, std::numeric_limits<INT>::lowest());
			DCHECK_LE(other, std::numeric_limits<INT>::max());
			return other;
		}

	public:
		template<class VAL>
		inline auto push_back(VAL&& val) {
			_raw.push_back(std::forward<VAL>(val));
			if constexpr(ERASABLE) ++this->_size;
			return create_accessor<MUTAB>(_raw.size()-1);
		}

		template<class... Args>
		inline auto emplace_back(Args&&... args) {
			_raw.emplace_back(std::forward<Args>(args)... );
			if constexpr(ERASABLE) ++this->_size;
			return create_accessor<MUTAB>(_raw.size()-1);
		}





		auto begin()       {  return Iterator<MUTAB>(*this, 0);  }
		auto end()         {  return Iterator<MUTAB>(*this, _raw.size());  }

		auto begin() const {  return Iterator<CONST>(*this, 0);  }
		auto end()   const {  return Iterator<CONST>(*this, _raw.size());  }



		auto& raw(Key key) {
			return _raw[key - get_offset()].value;
		}

		auto& raw(Key key) const {
			return _raw[key - get_offset()].value;
		}












		//
		// ACCESSOR (BASE)
		//

	public:
		template<Const_Flag C>
		class Accessor_Base {
		public:
			using Context = void; // derived can override
			using Owner = Dense_Map;

		public:
			const Key key;

			// 2 names for the same thing...
			//Const<Value,C>& raw;
			Proxy<Value,C> val;


		private:
			struct Exists {
				operator bool() const {
					return _idx < (int)owner._raw.size()
						&& _idx >= 0
						&& owner._raw[_idx].get_exists();
				}

			private:
				Exists(Const<Dense_Map,C>& o, Key i) : owner(o), _idx(i) {}
				friend Accessor_Base;

				Const<Dense_Map,C>& owner;
				Key _idx;
			};

			template<bool forward>
			struct Link {
				operator bool() const {

					//static_assert(!ERASABLE,
					//	"operator bool() is disabled for Erasable Dense_Map (performance) - "
					//	"use prev().exists or next().exists instead");

					if constexpr(ERASABLE) {
						auto idx = _find();
						return idx >= 0 && idx < owner._raw.size();
					}
					else {
						if constexpr(forward) {
							return _idx + 1 < owner._raw.size();
						}
						else {
							return _idx > 0;
						}
					}
				}

				auto operator()() {
					if constexpr(ERASABLE) {
						auto idx = _find();
						DCHECK_NE(Key(), idx);
						return owner.template create_accessor<C>(idx);
					}
					else {
						if constexpr(forward) {
							return owner.template create_accessor<C>(_idx + 1);
						}
						else {
							return owner.template create_accessor<C>[_idx - 1];
						}
					}
				}

				auto operator()() const {
					if constexpr(ERASABLE) {
						auto idx = _find();
						DCHECK_NE(Key(), idx);
						return owner.template create_accessor<CONST>(idx);
					}
					else {
						if constexpr(forward) {
							return owner.template create_accessor<CONST>(_idx + 1);
						}
						else {
							return owner.template create_accessor<CONST>[_idx - 1];
						}
					}
				}

			private:
				auto _find() const {
					DCHECK(ERASABLE);
					
					auto idx = _idx;

					if constexpr(forward) {
						for(;;) {
							++idx;
							if(idx >= (int)owner._raw.size()) {
								DCHECK_EQ(owner._raw.size(), idx);
								//idx = Key();
								break;
							}
							if(owner._raw[idx].exists) break;
						}
					}
					else {
						for(;;) {
							--idx;
							if(idx < 0) {
								DCHECK_EQ(-1, idx);
								//idx = Key();
								break;
							}
							if(owner._raw[idx].exists) break;
						}
					}

					return idx;
				}


			private:
				Link(Const<Dense_Map,C>& o, Key i) : owner(o), _idx(i) {}
				friend Accessor_Base;

				Const<Dense_Map,C>& owner;
				Key _idx;
			};


		public:
			Exists exists;

			Link<false> prev;
			Link<true>  next;

			void erase() {
				static_assert(ERASABLE, "can't erase, Dense_Map declared not Erasable");
				DCHECK(key - owner.get_offset() < (int)owner._raw.size());
				DCHECK(_raw().exists);
				_raw().exists = false;
				--owner._size;
			}

			operator Const<Value,C>&() {
				return val();
			}

			operator const Value&() const {
				return val();
			}


			template<class TT>
			auto operator=(TT&& new_value) {

				static_assert(C == MUTAB, "can't assign to const accessor");

				if constexpr(TYPE == Type::DEQUE) {
					if(owner._raw.empty()) {
							owner.offset = key;
							_idx = 0;
					}

					if(_idx < 0) {
						owner.offset += _idx;

						//owner._raw.insert(owner._raw.begin(), std::max(diff, (int)owner._raw.size()*3/2), Node());
						owner._raw.insert(owner._raw.begin(), -_idx, Node());
						_idx = 0;
					}
				}

				if(_idx >= (int)owner._raw.size()) {
					owner._raw.resize(_idx + 1);
				}

				if constexpr(ERASABLE) {
					if(!_raw().exists) ++owner._size;
					_raw().exists = true;
				}
				_raw().value = std::forward<TT>(new_value);
				return owner.template create_accessor<C>(_idx);
			}



			template<Const_Flag CC>
			inline bool operator==(const Accessor_Base<CC>& o) const {
				DCHECK_EQ(&owner, &o.owner);
				return _idx == o._idx;
			}

			template<Const_Flag CC>
			inline bool operator!=(const Accessor_Base<CC>& o) const {
				return !(*this == o);
			}



		// store environment:
		protected:
			Accessor_Base( Const<Dense_Map,C>& o, Key i) :
					key(i + o.get_offset()),
					val(val.create(o._raw[i].value)),
					//value(o._raw[i].value),
					exists(o,i),
					prev(o,i),
					next(o,i),
					owner(o),
					_idx(i) {}

			// TODO: get rid of member references and remove these:
			auto operator()() const { return update(); }
			auto update()     const { return owner.template create_accessor<C>(_idx); }

			auto& _raw() const {
				return owner._raw[_idx];
			}
			
			Const<Dense_Map,C>& owner;
			mutable Key _idx;
		};


		//friend Accessor_Base<CONST>;
		//friend Accessor_Base<MUTAB>;










		//
		// ITERATOR
		//
	public:

		template<Const_Flag C, class CRTP>
		class Iterator_Base {
		public:
			using Owner = Dense_Map;

		public:
			auto& operator++() {
				increment();
				return *this; }

			auto operator++(int) {
				auto old = *this;
				increment();
				return old; }

			auto& operator--() {
				decrement();
				return *this; }

			auto operator--(int) {
				auto old = *this;
				decrement();
				return old; }


			// WARNING: compares indices only
			template<Const_Flag CC, class CRTP2>
			bool operator==(const Iterator_Base<CC,CRTP2>& o) const {
				DCHECK_EQ(&owner, &o.owner);
				return idx == o.idx;
			}

			template<Const_Flag CC, class CRTP2>
			bool operator!=(const Iterator_Base<CC,CRTP2>& o) const {  return ! (*this == o);  }


			auto operator*()       {  return owner.template create_accessor<C>(idx);  }
			auto operator*() const {  return owner.template create_accessor<C>(idx);  }


			// unable to implement if using accessors:
			//const auto operator->() const {  return &container[idx];  }
			//      auto operator->()       {  return &container[idx];  }

		private:
			inline void increment() {
				do ++idx; while(idx < (int)owner._raw.size() && !_raw().get_exists());
			}

			inline void decrement() {
				do --idx; while(idx > 0 && !_raw().get_exists);
			}


		protected:
			template<Const_Flag CC, class CRTP2> 
			Iterator_Base(const Iterator_Base<CC,CRTP2>& o) : owner(o.owner), idx(o.idx) {}

		private:
			Iterator_Base(Const<Dense_Map,C>& o, Key i) :
					owner(o), idx(i) {
				DCHECK_GE(idx, 0) << "iterator constructor: index out of range";

				// can be equal to container.size() for end() iterators
				DCHECK_LE(idx, owner._raw.size()) << "iterator constructor: index out of range";

				// move forward if element is deleted
				while(idx < (int)owner._raw.size() && !_raw().get_exists()) {
					++idx;
				}
			}

			inline auto& _raw() const {
				return owner._raw[idx];
			}

		protected:
			Const<Dense_Map,C>& owner;
			Key idx;

			friend Dense_Map;
		};


		template<Const_Flag C>
		class Iterator : public Iterator_Base<C, Iterator<C>> {
		public: // g++ won't compile if private... why?
			template<class... Args>
			Iterator(Args&&... args) : Iterator_Base<C, Iterator<C>>(std::forward<Args>(args)... ) {}

			friend Dense_Map;
		};






		// data
	private:

		struct Node : Add_exists<ERASABLE> {

			Node() = default;

			template<class... Args>
			Node(Args&&... args) : value(std::forward<Args>(args)...) {
				if constexpr(ERASABLE) {
					Add_exists<ERASABLE>::exists = true;
				}
			}

			inline bool get_exists() const {
				if constexpr(ERASABLE) {
					return Add_exists<ERASABLE>::exists;
				}
				else return true;
			}

			Value value;
		};

		Container _raw;

		inline Key get_offset() const {
			if constexpr(TYPE == Type::DEQUE) return Add_offset<TYPE == Type::DEQUE>::offset;
			else return 0;
		}

		//int start_idx = 0; // TODO: to avoid performance issues when begin() is called inside a loop
	};









	//
	// BUILDER
	//
	template<
		class Value_Or_Key,
		class Void_Or_Value,
		Type TYPE,
		template<Const_Flag,class> class ACCESSOR_TEMPLATE,
		bool ERASABLE
	>
	class Builder {

	public:
		using BUILD = internal::Dense_Map::Dense_Map<Value_Or_Key, Void_Or_Value, TYPE, ACCESSOR_TEMPLATE, ERASABLE>;

		using Vector
			= Builder<Value_Or_Key, Void_Or_Value, Type::VECTOR, ACCESSOR_TEMPLATE, ERASABLE>;

		using Deque
			= Builder<Value_Or_Key, Void_Or_Value, Type::DEQUE, ACCESSOR_TEMPLATE, ERASABLE>;

		using Erasable
			= Builder<Value_Or_Key, Void_Or_Value, TYPE, ACCESSOR_TEMPLATE, true>;
		
		template<template<Const_Flag,class> class NEW_TMPL>
		using Accessor_Template
			= Builder<Value_Or_Key, Void_Or_Value, TYPE, NEW_TMPL, ERASABLE>;

	};



} // namespace Dense_Map
} // namespace internal











template<
	class Value_Or_Key,
	class Void_Or_Value = void
>
class Dense_Map : public internal::Dense_Map::Dense_Map<
	Value_Or_Key,
	Void_Or_Value,
	internal::Dense_Map::Type::DEQUE,
	internal::Index_Accessor_Template,
	true // erasable
> {
private:
	using _BASE = internal::Dense_Map::Dense_Map<
		Value_Or_Key,
		Void_Or_Value,
		internal::Dense_Map::Type::DEQUE,
		internal::Index_Accessor_Template,
		true // erasable
	>;

public:
	template<class... Args>
	Dense_Map(Args&&... args) : _BASE(std::forward<Args>(args)... ) {}

	template<class TT>
	Dense_Map(const std::initializer_list<TT>& l) : _BASE(l) {}

	using BUILDER = internal::Dense_Map::Builder<
		Value_Or_Key,
		Void_Or_Value,
		internal::Dense_Map::Type::VECTOR,
		internal::Index_Accessor_Template,
		false // erasable
	>;
};










} // namespace salgo



