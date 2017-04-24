// Simple (and stupid) flyweight pattern. Maybe should've used boost::flyweight but it's a heavy header and got a 
// ton of complexity I don't need, plus I will want to make it optionally track time since last use and discard and on 
// next use re-instantiate the actual data with same parameters if it has not been used in a given time
// I however do not want to simply discard on reference count of zero due to usage patterns within my game (enemies spawn, 
// get all killed, spawn more, etc)

// absent functionality:
// reference counted removal, hash maps (but doesn't require keys to be hashable)

// unusual misuses: can modify the object being referred to, modifying all similar objects (necessary for objects that 
// perform lazy initialization etc).
// intentionally inconvenient: iterating over all stored objects (I don't want to turn it into hidden globals)
//
// Potential issues: it may be possible that in some compilers the cache may not be shared among all 
//  compilation units (needs to be investigated)

// Known gotchas:
// .New is not threadsafe (but can be made threadsafe, even in lockless manner if need arises).
// Destruction of objects held will happen on application exit, after things like rendering contexts are closed
// If you use a char literal, the parameter type is const char* and the resource will only be shared if that pointer is the same, 
//  which it is not guaranteed to be (but often is)
// If you use a char literal for one and you use std::string for another constructor the resource will not be shared
// Type coercion from different types will result in not sharing the resource (it doesn't coerce until calling the constructor).
// If you use floats, map can segfault if there is a NAN because floats are not strict weak ordered and map 
//  segfaults if the strong weak ordering is broken.
//  (todo: better comparator or forbid floats. probably forbid floats because you also won't share resource between very similar 
//  floats even when it is desired)
// Also, on good ol x86 floats used to be even more crazy (internally computed in 80 bit precision)
// Constructor parameters are not using std::forward

#include <string>
#include <map>
#include <memory>
#include <tuple>
#include <typeinfo>
#include <typeindex>
#include <assert.h>

class CachedArgsAbstract{
public:
	virtual ~CachedArgsAbstract() {};
};
template<class T>
class CachedArgs : public CachedArgsAbstract {
public:
	T v;
	CachedArgs(const T &v_) :v(v_) {};
	bool operator<(const CachedArgs<T> &other) const { return v < other.v; }
};

template<typename T, int tag, typename... Args>
class MySharedMaps {
public:
	static std::map<CachedArgs<std::tuple<Args...> >, T*> data;
};

template<typename T, int tag, typename... Args> std::map<CachedArgs<std::tuple<Args...> >, T*> MySharedMaps<T, tag, Args... >::data;

// tag can be used to avoid resource sharing between different tags
template <class T, int tag = 0> class SimpleFlyweight {
	struct no_args {
	};
public:
	T *ptr{};
	// Only needed for optimized New, if we need this to be as small as a pointer, this can be removed
	std::type_index current_args_type{ typeid(no_args) };	
	const CachedArgsAbstract *current_args_ptr{};

	SimpleFlyweight() {
	}

	SimpleFlyweight(const SimpleFlyweight &other) : ptr(other.ptr), current_args_type(other.current_args_type), current_args_ptr(other.current_args_ptr) {
	}

	template<typename... Args>
	T *New(Args... args) {
		const auto key = CachedArgs<std::tuple<Args... > >(std::make_tuple(args...));
		std::type_index args_type(typeid(decltype(key)));

		if (ptr) {// don't do map lookup if the arguments hadn't changed
			if (args_type == current_args_type) {
				if (dynamic_cast<decltype(key)* >(current_args_ptr)->v == key.v) {
					return ptr;
				}
			}
		}
		current_args_type = args_type;

		auto i = MySharedMaps< T, tag, Args... >::data.find(key);
		if (i != MySharedMaps< T, tag, Args... >::data.end()) {
			current_args_ptr = &i->first;
			ptr = i->second;
		}
		else {
			auto a = MySharedMaps< T, tag, Args... >::data.insert(std::make_pair(key, ptr = new T(args...))).first;
			current_args_ptr=&a->first;		
		}
		return ptr;
	}

	template<typename... Args>
	SimpleFlyweight(Args... args) {
		New(args...);
	}

	T &operator()() {
		assert(ptr);
		return *ptr;
	};

	T *operator->() {
		assert(ptr);
		return ptr;
	};

	const T *operator->() const {
		assert(ptr);
		return ptr;
	};

	T *GetPtr() {
		return ptr;
	}
	bool ok() {
		return ptr != nullptr;
	}
	bool Ok() {
		return ptr != nullptr;
	}
};