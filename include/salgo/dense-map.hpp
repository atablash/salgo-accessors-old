#pragma once

#include "common.hpp"
#include "accessors-common.hpp"

#include <glog/logging.h>

#include <deque>


namespace salgo {








enum class Dense_Map_Type {
	VECTOR,
	DEQUE
};

namespace {
	constexpr auto VECTOR = Dense_Map_Type::VECTOR;
	constexpr auto DEQUE  = Dense_Map_Type::DEQUE;
}






enum class Dense_Map_Flags {
	NONE = 0,
	ERASABLE = 0x0001
};

ENABLE_BITWISE_OPERATORS(Dense_Map_Flags);

namespace {
	constexpr auto ERASABLE = Dense_Map_Flags::ERASABLE;
}


namespace internal {



	// for Dense_Map
	template<bool> class Add_Member_offset {protected: int offset = 0; };
	template<>     class Add_Member_offset <false> {};

	// for Node
	template<bool> struct Add_Member_exists { bool exists = false; };
	template<>     struct Add_Member_exists <false> {};

	// for Dense_Map (can't use this...)
	//template<bool, class T> struct Add_Member_context { T context = T(); };
	//template<      class T> struct Add_Member_context <false,T> {};







	constexpr auto default__Dense_Map_Flags = ERASABLE;
	constexpr auto default__Dense_Map_Type = VECTOR;

	template<Const_Flag C, class OWNER, class BASE>
	using Default__Dense_Map_Accessor_Template = Index_Accessor_Template<C, OWNER, BASE>;






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
		class Value_Or_Key,
		class Void_Or_Value,
		Dense_Map_Flags FLAGS,
		Dense_Map_Type TYPE,
		template<Const_Flag,class,class> class ACCESSOR_TEMPLATE
	>
	class Dense_Map : public internal::Add_Member_offset<TYPE == DEQUE> {

		// forward declarations
		public: template<Const_Flag C> class Accessor_Base;
		private: struct Node;


	public:
		using Key = std::conditional_t<std::is_same_v<Void_Or_Value,void>, int, Value_Or_Key>;
		using Value = std::conditional_t<std::is_same_v<Void_Or_Value,void>, Value_Or_Key, Void_Or_Value>;

		static constexpr auto Flags = FLAGS;
		using Container = std::conditional_t<TYPE == VECTOR, std::vector<Node>, std::deque<Node>>;

		template<Const_Flag C>
		using Accessor = ACCESSOR_TEMPLATE<C, Dense_Map, Accessor_Base<C>>;

		//friend Accessor<CONST>;
		//friend Accessor<MUTAB>;

	private:
		static constexpr bool has_context = !std::is_same_v<typename Accessor<MUTAB>::Context,void>;

		// unfortunately storing uint8_t here
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

		Dense_Map(Key new_offset)
				: internal::Add_Member_offset<TYPE == DEQUE>(new_offset) {

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
			return create_accessor<MUTAB>(_raw.size()-1);
		}

		template<class... Args>
		inline auto emplace_back(Args&&... args) {
			_raw.emplace_back(std::forward<Args>(args)... );
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

		public:
			const Key key;

			// 2 names for the same thing...
			//Const<Value,C>& raw;
			Proxy<Value,C> val;


		private:
			struct Exists {
				operator bool() const {
					return idx < (int)owner._raw.size()
						&& idx >= 0
						&& owner._raw[idx].get_exists();
				}

			private:
				Exists(Const<Dense_Map,C>& o, Key i) : owner(o), idx(i) {}
				friend Accessor_Base;

				Const<Dense_Map,C>& owner;
				mutable Key idx;
			};

		public:
			Exists exists;


			void erase() {
				DCHECK(key - owner.get_offset() < (int)owner._raw.size());
				DCHECK(_raw().exists);
				_raw().exists = false;
			}

			operator Const<Value,C>&() {
				return val();
			}

			operator const Value&() const {
				return val();
			}


			template<class TT>
			auto operator=(TT&& new_value) {

				static_assert(C == MUTAB, "assignment to const accessor");

				if constexpr(TYPE==DEQUE) {
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

				if constexpr(bool(Flags & ERASABLE)) _raw().exists = true;
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
					owner(o),
					_idx(i) {}

			auto operator()() const { return update(); }
			auto update()     const { return owner.template create_accessor<C>(_idx); }

			auto& _raw() const {
				return owner._raw[_idx];
			}
			
			Const<Dense_Map,C>& owner;
			mutable Key _idx;
		};













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

		struct Node : internal::Add_Member_exists<bool(Flags & ERASABLE)> {

			Node() = default;

			template<class... Args>
			Node(Args&&... args) : value(std::forward<Args>(args)...) {
				if constexpr(bool(Flags & ERASABLE)) {
					internal::Add_Member_exists<bool(Flags & ERASABLE)>::exists = true;
				}
			}

			inline bool get_exists() const {
				if constexpr(bool(Flags & ERASABLE)) {
					return internal::Add_Member_exists<bool(Flags & ERASABLE)>::exists;
				}
				else return true;
			}

			Value value;
		};

		Container _raw;

		inline Key get_offset() const {
			if constexpr(TYPE==DEQUE) return internal::Add_Member_offset<TYPE == DEQUE>::offset;
			else return 0;
		}

		//int start_idx = 0; // TODO: to avoid performance issues when begin() is called inside a loop
	};



} // internal










//
// W/O BUILDER
//
template<
	class Value_Or_Key,
	class Void_Or_Value = void
>
using Dense_Map = internal::Dense_Map<
	Value_Or_Key,
	Void_Or_Value,
	internal::default__Dense_Map_Flags,
	internal::default__Dense_Map_Type,
	internal::Default__Dense_Map_Accessor_Template
>;










//
// BUILDER
//
template<
	class Value_Or_Key,
	class Void_Or_Value = void,
	Dense_Map_Flags FLAGS = internal::default__Dense_Map_Flags,
	Dense_Map_Type TYPE = internal::default__Dense_Map_Type,
	template<internal::Const_Flag,class,class> class ACCESSOR_TEMPLATE = internal::Default__Dense_Map_Accessor_Template
>
class Dense_Map_Builder {

public:
	using Build = internal::Dense_Map<Value_Or_Key, Void_Or_Value, FLAGS, TYPE, ACCESSOR_TEMPLATE>;

	template<Dense_Map_Type NEW_TYPE>
	using Type
		= Dense_Map_Builder<Value_Or_Key, Void_Or_Value, FLAGS, NEW_TYPE, ACCESSOR_TEMPLATE>;
	
	template<template<internal::Const_Flag,class,class> class NEW_TMPL>
	using Accessor_Template
		= Dense_Map_Builder<Value_Or_Key, Void_Or_Value, FLAGS, TYPE, NEW_TMPL>;


	template<Dense_Map_Flags NEW_FLAGS>
	using Flags
		= Dense_Map_Builder<Value_Or_Key, Void_Or_Value, NEW_FLAGS, TYPE, ACCESSOR_TEMPLATE>;

	template<Dense_Map_Flags NEW_FLAGS>
	using Add_Flags
		= Dense_Map_Builder<Value_Or_Key, Void_Or_Value, FLAGS |  NEW_FLAGS, TYPE, ACCESSOR_TEMPLATE>;

	template<Dense_Map_Flags NEW_FLAGS>
	using Rem_Flags
		= Dense_Map_Builder<Value_Or_Key, Void_Or_Value, FLAGS & ~NEW_FLAGS, TYPE, ACCESSOR_TEMPLATE>;
};













} // namespace salgo



