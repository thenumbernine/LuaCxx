#pragma once

/*
code for automatic generation of Lua bindings for any structs
just provide a Bind<T> class and a getFields() method to return a map of fields.

TODO organize better.  move all the impl code into an impl{} namespace or something maybe not idk.

TODO the LuaRW is very similar to LuaCxx/pushToLuaState and readFromLuaState ...
 ... but for this there are cases specific to the light/heavy userdata that the C++ code is using
I'll think of how to merge the two someday...

TODO same with Bind and LuaRW ... they could be the same

Honestly, this Bind.h is separate of all ther est of LuaCxx, which was originally designed just to be a wrapper for using LuaRefs and the global registery in C++...
... maybe this should be in its own repo?
*/

#include "Common/String.h"	//Common::join_v
#include "Common/Function.h"	//Common::Function
#include "Common/MemberPointer.h"	//Common::MemberPointer, Common::MemberMethodPointer
#include <lua.hpp>
#include <type_traits>

inline void * operator new(size_t size, lua_State * L) {
	return lua_newuserdata(L, size);
}

namespace LuaCxx {

// read/write operations

// default case = push full userdata copy for pass-by-value types
template<typename T>
struct LuaRW {
	static void push(lua_State * L, T v);
	static T read(lua_State * L, int index);
};

template<typename T>
requires (std::is_floating_point_v<std::remove_reference_t<T>>)
struct LuaRW<T> {
	static void push(lua_State * L, T v) {
		lua_pushnumber(L, (lua_Number)v);
	}
	static std::remove_reference_t<T> read(lua_State * L, int index) {
		return (std::remove_reference_t<T>)lua_tonumber(L, index);
	}
};

template<typename T>
requires (std::is_integral_v<std::remove_reference_t<T>>)
struct LuaRW<T> {
	static void push(lua_State * L, T v) {
		lua_pushinteger(L, v);
	}

	// ok LuaRW<T> could be an int or an int&
	// T pertains to the type of the C++ data we are read/write-ing
	// so if T is int& then we just want to reutrn an int here, not an int& here ...
	static std::remove_reference_t<T> read(lua_State * L, int index) {
		// TODO what to do if the type doesn't match? i.e. assigning a string to an int?
		// right now it just assigns 0 ...
		return lua_tointeger(L, index);
	}
};

// args conversion does this, for pushing-by-value
// field access is handled in string&, which right now hits the LuaRW case for fields
template<>
struct LuaRW<std::string> {
	static void push(lua_State * L, std::string v) {
		lua_pushlstring(L, v.data(), v.size());
	}
	static std::string read(lua_State * L, int index) {
		size_t n = {};
		auto ptr = lua_tolstring(L, index, &n);
		return std::string(ptr, n);
	}
};


// here' all the generic lua-binding stuff

template<typename Type>
struct Bind;

// infos for prims.  doesn't have lua exposure, only mtname for mtname joining at compile time
// TODO combine this with the LuaRW stuff?  maybe? maybe not?

template<>
struct Bind<char> {
	static constexpr std::string_view mtname = "char";
};

template<>
struct Bind<short> {
	static constexpr std::string_view mtname = "short";
};

template<>
struct Bind<int> {
	static constexpr std::string_view mtname = "int";
};

template<>
struct Bind<float> {
	static constexpr std::string_view mtname = "float";
};

template<>
struct Bind<double> {
	static constexpr std::string_view mtname = "double";
};

template<>
struct Bind<long double> {
	static constexpr std::string_view mtname = "long double";
};

template<>
struct Bind<std::string> {
	static constexpr std::string_view mtname = "std::string";
	static void getMT(lua_State * L) {
		// technically yes I could get string here but ...
		// this function is for setting C++ obj's metatables, not setting builtin Lua data's metatable
		lua_pushnil(L);
	}
};

template<typename T>
struct Bind<T&> {
	static constexpr std::string_view suffix = "&";
	static constexpr std::string_view mtname = Common::join_v<Bind<T>::mtname, suffix>;
	static void getMT(lua_State * L) {
		if constexpr (std::is_class_v<T>) {
			Bind<T>::getMT(L);
		} else {
			lua_pushnil(L);
		}
	}
};

template<typename T>
struct Bind<T*> {
	static constexpr std::string_view suffix = "*";
	static constexpr std::string_view mtname = Common::join_v<Bind<T>::mtname, suffix>;
};

// needs to be a macro and not a C++ typed expression for lua's pushliteral to work
#define LUACXX_BIND_PTRFIELD "ptr"

template<typename T>
static T * lua_getptr(lua_State * L, int index) {
	luaL_checktype(L, index, LUA_TTABLE);
	lua_pushliteral(L, LUACXX_BIND_PTRFIELD);
	lua_rawget(L, index);
	if (!lua_isuserdata(L, -1)) {	// both kinds of userdata plz
		luaL_checktype(L, -1, LUA_TUSERDATA);	// but i like their error so
	}
	// verify metatable is Bind<T>::mtname ... however lua_checkudata() does this but only for non-light userdata ...
	T * o = (T*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	if (!o) luaL_error(L, "tried to index a null pointer");
	return o;
}

// I could just use the Bind static methods themselves, but meh, I cast the object here

template<typename T>
static inline int __tostring(lua_State * L) {
	return Bind<T>::__tostring(L, *lua_getptr<T>(L, 1));
}

template<typename T>
static inline int __index(lua_State * L) {
	return Bind<T>::__index(L, *lua_getptr<T>(L, 1));
}

template<typename T>
static inline int __newindex(lua_State * L) {
	return Bind<T>::__newindex(L, *lua_getptr<T>(L, 1));
}

template<typename T>
static inline int __pairs(lua_State * L) {
	return Bind<T>::__pairs(L, *lua_getptr<T>(L, 1));
}

template<typename T>
static inline int __len(lua_State * L) {
	return Bind<T>::__len(L, *lua_getptr<T>(L, 1));
}

template<typename T>
static inline int __ipairs(lua_State * L) {
	return Bind<T>::__ipairs(L, *lua_getptr<T>(L, 1));
}

template<typename T>
static inline int __call(lua_State * L) {
	return Bind<T>::__call(L, *lua_getptr<T>(L, 1));
}

// default behavior.  child template-specializations can override this.

template<typename T>
static int default__tostring(lua_State * L) {
	auto & o = *lua_getptr<T>(L, 1);
	lua_pushfstring(L, "%s: 0x%x", Bind<T>::mtname.data(), &o);
	return 1;
}

template<typename T>
static int default__index(lua_State * L) {
	auto & o = *lua_getptr<T>(L, 1);
	auto const k = LuaRW<std::string>::read(L, 2);
	auto const & fields = Bind<T>::getFields();
	auto iter = fields.find(k);
	if (iter == fields.end()) {
		lua_pushnil(L);
		return 1;
	}
	iter->second->push(o, L);
	return 1;
}

template<typename T>
static int default__newindex(lua_State * L) {
	assert(lua_gettop(L) == 3);	// t k v
	auto & o = *lua_getptr<T>(L, 1);
	auto const k = LuaRW<std::string>::read(L, 2);
	auto const & fields = Bind<T>::getFields();
	auto iter = fields.find(k);
	if (iter == fields.end()) {
#if 0	// option 1: no new fields
		luaL_error(L, "sorry, this table cannot accept new members");
#endif
#if 1	// option 2: write the Lua value into the Lua table
		lua_rawset(L, 1);
		return 0;
#endif
	}
	iter->second->read(o, L, 3);
	return 0;
}

template<typename T, auto NextPairs = nullptr>
static int default_pairsNext(lua_State * L) {
	auto & o = *lua_getptr<T>(L, 1);
	auto const & fields = Bind<T>::getFields();
	if (lua_isnil(L, 2)) {
		if (fields.empty()) {
			//we are empty ... TODO if next then process what's next ...
			if constexpr(NextPairs != nullptr) {
				return NextPairs(L);
			} else {
				return 0;
			}
		}
		// first key
		// TODO how about string_view map ?
		auto iter = fields.begin();
		LuaRW<std::string>::push(L, iter->first);
		iter->second->push(o, L);
		return 2;
	}
	if (!lua_isstring(L, 2)) {
		// key isn't ours ... TODO if next then process what's next ...
		return 0;
		if constexpr(NextPairs != nullptr) {
			return NextPairs(L);
		} else {
			return 0;
		}
	}
	// better be a string, cuz that's all I'm supporting atm
	auto const k = LuaRW<std::string>::read(L, 2);
	auto iter = fields.find(k);
	if (iter == fields.end()) {
		// key isn't ours ... TODO if next then process what's next ...
		if constexpr(NextPairs != nullptr) {
			return NextPairs(L);
		} else {
			return 0;
		}
	}
	++iter;
	if (iter == fields.end()) {
		//prev key was last key ... TODO if next then process what's next ...
		if constexpr(NextPairs != nullptr) {
			return NextPairs(L);
		} else {
			return 0;
		}
	}
	LuaRW<std::string>::push(L, iter->first);
	iter->second->push(o, L);
	return 2;
}

template<typename T>
static int default__pairs(lua_State * L) {
	lua_pushcfunction(L, default_pairsNext<T>);
	lua_pushvalue(L, 1);
	lua_pushnil(L);
	return 3;
}

// base infos for all structs...
// TODO just template this? or can I?  cuz the template arg needs the member ptr, which then would guarantee it already exists ....
template<typename T> constexpr bool has__index_v = requires(T const & t) { t.__index; };
template<typename T> constexpr bool has__newindex_v = requires(T const & t) { t.__newindex; };
template<typename T> constexpr bool has__pairs_v = requires(T const & t) { t.__pairs; };
template<typename T> constexpr bool has__len_v = requires(T const & t) { t.__len; };
template<typename T> constexpr bool has__ipairs_v = requires(T const & t) { t.__ipairs; };
template<typename T> constexpr bool has__call_v = requires(T const & t) { t.__call; };
template<typename T> constexpr bool has__tostring_v = requires(T const & t) { t.__tostring; };
template<typename T> constexpr bool has_mt_ctor_v = requires(T const & t) { t.mt_ctor; };

template<typename T>
struct BindStructBase {

	// add the class constructor as the call operator of the metatable
	// leaves metatable on the stack
	static void initMtCtor(lua_State * L) {
		static constexpr std::string_view suffix = " metatable";
		static constexpr std::string_view mtname = Common::join_v<Bind<T>::mtname, suffix>;
		if (!luaL_newmetatable(L, mtname.data())) return;

		lua_pushcfunction(L, Bind<T>::mt_ctor);
		lua_setfield(L, -2, "__call");
	}

	// initialize the metatable associated with this type
	// leaves the metatable on the stack
	static void getMT(lua_State * L) {
		auto const & mtname = Bind<T>::mtname;

		if (luaL_newmetatable(L, mtname.data())) {
			// not supported in luajit ...
			lua_pushstring(L, mtname.data());
			lua_setfield(L, -2, "__name");

//std::cout << "building " << mtname << " metatable" << std::endl;

			/*
			for __index and __newindex I'm providing default behavior.
			i used to provide it via static parent class method, but that emant overriding children ha to always 'using' to specify their own impl ver the aprent
			so nw i'm just moving the defult outside the class
			but the downside is - with the if constexpr check here - that there will always be an __index but maybe that wasla ways the case ...
			*/

			if constexpr (has__tostring_v<Bind<T>>) {
				lua_pushcfunction(L, ::LuaCxx::__tostring<T>);
				lua_setfield(L, -2, "__tostring");
//std::cout << "assigning __tostring" << std::endl;
			} else {
				lua_pushcfunction(L, ::LuaCxx::default__tostring<T>);
				lua_setfield(L, -2, "__tostring");
//std::cout << "using default __tostring" << std::endl;
			}

			if constexpr (has__index_v<Bind<T>>) {
				lua_pushcfunction(L, ::LuaCxx::__index<T>);
				lua_setfield(L, -2, "__index");
//std::cout << "assigning __index" << std::endl;
			} else {
				lua_pushcfunction(L, ::LuaCxx::default__index<T>);
				lua_setfield(L, -2, "__index");
//std::cout << "using default __index" << std::endl;
			}

			if constexpr (has__newindex_v<Bind<T>>) {
				lua_pushcfunction(L, ::LuaCxx::__newindex<T>);
				lua_setfield(L, -2, "__newindex");
//std::cout << "assigning __newindex" << std::endl;
			} else {
				lua_pushcfunction(L, ::LuaCxx::default__newindex<T>);
				lua_setfield(L, -2, "__newindex");
//std::cout << "using default __newindex" << std::endl;
			}

			if constexpr (has__pairs_v<Bind<T>>) {
				lua_pushcfunction(L, ::LuaCxx::__pairs<T>);
				lua_setfield(L, -2, "__pairs");
//std::cout << "assigning __pairs" << std::endl;
			} else {
				lua_pushcfunction(L, ::LuaCxx::default__pairs<T>);
				lua_setfield(L, -2, "__pairs");
//std::cout << "using default __pairs" << std::endl;
			}


			// the rest of these are optional to provide

			if constexpr (has__len_v<Bind<T>>) {
				lua_pushcfunction(L, ::LuaCxx::__len<T>);
				lua_setfield(L, -2, "__len");
//std::cout << "assigning __len" << std::endl;
			}

			if constexpr (has__ipairs_v<Bind<T>>) {
				lua_pushcfunction(L, ::LuaCxx::__ipairs<T>);
				lua_setfield(L, -2, "__ipairs");
//std::cout << "assigning __ipairs" << std::endl;
			}

			if constexpr (has__call_v<Bind<T>>) {
				lua_pushcfunction(L, ::LuaCxx::__call<T>);
				lua_setfield(L, -2, "__call");
//std::cout << "assigning __call" << std::endl;
			}

			// ok now let's give the metatable a metatable, so if someone calls it, it'll call the ctor
			if constexpr (has_mt_ctor_v<Bind<T>>) {
				initMtCtor(L);
				lua_setmetatable(L, -2);
//std::cout << "assigning metatable __call ctor" << std::endl;
			}
		}
	}
};

// sets metatable of the obj on top of the stack to the metatable associated with T
// builds T's metatable if necessary
template<typename T>
constexpr inline void setMT(lua_State * L) {
	Bind<T>::getMT(L);
	if (lua_isnil(L, -1)) {
		luaL_error(L, "YOU HAVE NOT YET INITIALIZED METATABLE %s", Bind<T>::mtname.data());
	}
	lua_setmetatable(L, -2);
}

// LuaRW general case
// for return-by-value
// create a full-userdata of the object so that it sticks around in memory after the function, when Lua tries to access it
template<typename T>
void LuaRW<T>::push(lua_State * L, T v) {
	lua_newtable(L);
	setMT<T>(L);
	lua_pushliteral(L, LUACXX_BIND_PTRFIELD);
	new(L) T(v);
	lua_rawset(L, -3);
}

template<typename T>
T LuaRW<T>::read(lua_State * L, int index) {
	luaL_error(L, "this field cannot be overwritten");
	throw std::runtime_error("this field cannot be overwritten");
}

// same as pointer impl
template<typename T>
requires (std::is_reference_v<T> && std::is_class_v<std::remove_reference_t<T>>)
struct LuaRW<T> {
	static void push(lua_State * L, T v) {
		// this is getting ugly ... I need to clean it up ...
		// there is a string and string& override. .. if I add too many string permtuations then the compiler cmplains about ambiguous mathces ...
		// how long until I turn the whole thing into a giant if constexpr condition?
		if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>) {
			lua_pushlstring(L, v.data(), v.size());
		} else {
			// hmm, we want a metatable for what we return
			// but lightuserdata has no metatable
			// so it'll have to be a new lua table that points back to this
			lua_newtable(L);
			setMT<std::remove_reference_t<T>>(L);
			lua_pushliteral(L, LUACXX_BIND_PTRFIELD);
			lua_pushlightuserdata(L, &v);
			lua_rawset(L, -3);
		}
	}
	static decltype(auto) read(lua_State * L, int index) {
		// same complaint as above ...
		if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>) {
			size_t n = {};
			auto ptr = lua_tolstring(L, index, &n);
			return std::string(ptr, n);
		} else {
			// TODO consider C++ coercion?
			auto src = lua_getptr<std::remove_cvref_t<T>>(L, index);
			return *src;
		}
	}
};


// pointers work same as reference impl
// (so do refs-to-pointers)
// TODO if it's a pointer to a non-class ....... how to treat pointers?
// in the long run I'd like to just return the luajit cdata.
template<typename T>
requires (std::is_pointer_v<std::remove_reference_t<T>>)
struct LuaRW<T> {
	static void push(lua_State * L, T v) {
		if constexpr (std::is_class_v<std::remove_pointer_t<std::remove_reference_t<T>>>) {
			lua_newtable(L);
			setMT<std::remove_pointer_t<std::remove_reference_t<T>>>(L);
			lua_pushliteral(L, LUACXX_BIND_PTRFIELD);
			// ... one dif between ref and pointer... is there a templated 'get address' method?
			lua_pushlightuserdata(L, v);
			lua_rawset(L, -3);
		} else {
			// pointer-to-primitive ...
			// I feel like I should be returning a boxed type that has all pointer operations and allows reading the memory at this location ...
			// but meh
			lua_pushlightuserdata(L, v);
		}
	}
	static T read(lua_State * L, int index) {
		luaL_error(L, "this field cannot be overwritten");
		throw std::runtime_error("this field cannot be overwritten");
	}
};

// TODO instead of FieldBase & vtable, use variant<> or any<> or something ... supposedly faster?
template<typename Base>
struct FieldBase {
	virtual ~FieldBase() {};
	// o can't be const because o.*field can't be const because in case it's a blob/ptr then it's getting pushed into lua as lightuserdata ... and can't be const
	virtual void push(Base & o, lua_State * L) const = 0;
	virtual void read(Base & o, lua_State * L, int index) const = 0;
};

// this is a static method
// but exposed as a member of the ANN class
// TODO this for all static methods of any class, and move it to LuaCxx/Bind.h
// and in that case TODO, should static members take . or : ?  probably :, as in ClassMetaTable:staticMethod(...)
//  so that child classes invoking static methods pass into the method a way to identify the class
//  even tho in C that's not needed
//  but just for call convention's sake, I'll accept (and ignore) the 1st argument for class-static methods
// ... very tempting to just wrap a function pointer and remove teh 1st arg, use . instead of :
template<auto func>
int staticMemberMethodWrapper(lua_State * L) {
	// hmm ... interesting that Common::Function takes a function def, but Common::Member* takes a function-pointer or member-pointer
	// ... can you declare a non-pointer member-function?  would you want to?
	using F = Common::Function<std::remove_pointer_t<decltype(func)>>;
	using Args = typename F::Args;
	using Return = typename F::Return;
	// Expect the 1st arg to be the class table <=> object metatable
	//  but don't bother interpret it.
	//auto & o = *lua_getptr<typename MMP::Class>(L, 1);

	// here I'm adding +2 ... +1 to go from C++ to Lua stack index, +1 to offset past the initial object
	if constexpr (std::is_same_v<Return, void>) {
		[&]<auto...j>(std::index_sequence<j...>) -> decltype(auto) {
			func(
				LuaRW<
					std::remove_const_t<std::tuple_element_t<j, Args>>
				>::read(L, j+2)...
			);
		}(std::make_index_sequence<std::tuple_size_v<Args>>{});
		return 0;
	} else {
		LuaRW<Return>::push(L,
			[&]<auto...j>(std::index_sequence<j...>) -> decltype(auto) {
				return func(
					LuaRW<
						std::remove_const_t<std::tuple_element_t<j, Args>>
					>::read(L, j+2)...
				);
			}(std::make_index_sequence<std::tuple_size_v<Args>>{})
		);
		return 1;
	}
}

template<
	typename Type,
	auto func
>
struct FieldStatic : public FieldBase<Type> {
	virtual void push(Type & obj, lua_State * L) const override {
		//push a c function that wraps this method (and transforms all the arguments)
		lua_pushcfunction(L, staticMemberMethodWrapper<func>);
	}

	virtual void read(Type & obj, lua_State * L, int index) const override {
		luaL_error(L, "this field is read only");
	}
};



template<auto field>
int memberMethodWrapper(lua_State * L) {
	using MMP = Common::MemberMethodPointer<decltype(field)>;
	using Args = typename MMP::Args;
	using Return = typename MMP::Return;
	auto & o = *lua_getptr<typename MMP::Class>(L, 1);
	// TODO here, and in Matrix returning ThinVector
	// and maybe overall ...
	// ... if we are writing by value then push a dense userdata
	// ... if we are writing by pointer or ref then push a light userdata
	// but mind you, how to determine from LuaRW, unless we only use pass-by-value for values and pass-by-ref for refs ?
	// or maybe another template arg in LuaRW?

	// here I'm adding +2 ... +1 to go from C++ to Lua stack index, +1 to offset past the initial object
	if constexpr (std::is_same_v<Return, void>) {
		[&]<auto...j>(std::index_sequence<j...>) -> decltype(auto) {
			(o.*field)(
				LuaRW<
					std::remove_const_t<std::tuple_element_t<j, Args>>
				>::read(L, j+2)...
			);
		}(std::make_index_sequence<std::tuple_size_v<Args>>{});
		return 0;
	} else {
		LuaRW<Return>::push(L,
			[&]<auto...j>(std::index_sequence<j...>) -> decltype(auto) {
				return (o.*field)(
					LuaRW<
						std::remove_const_t<std::tuple_element_t<j, Args>>
					>::read(L, j+2)...
				);
			}(std::make_index_sequence<std::tuple_size_v<Args>>{})
		);
		return 1;
	}
}


// should this go in Common/MemberPointer.h?
// what's the better way to do this?
template<typename T>
struct MemberBaseClass {
	static decltype(auto) value() {
		if constexpr(std::is_member_object_pointer_v<T>) {
			using type = typename Common::MemberPointer<T>::Class;
			return (type*)nullptr;
		} else if constexpr (std::is_member_function_pointer_v<T>) {
			using type = typename Common::MemberMethodPointer<T>::Class;
			return (type*)nullptr;
		} else {
			return nullptr;
		}
	}
	using type = typename std::remove_pointer_t<decltype(value())>;\
};

//generic field is an object, exposed as lightuserdata wrapped in a table
template<auto field>
struct Field
: public FieldBase<typename MemberBaseClass<decltype(field)>::type>
{
	using T = decltype(field);
	using Base = typename MemberBaseClass<T>::type;

	virtual void push(Base & obj, lua_State * L) const override {
		if constexpr(std::is_member_object_pointer_v<T>) {
			using Value = typename Common::MemberPointer<T>::FieldType;
			LuaRW<Value&>::push(L, obj.*field);	// LuaRW<T&> means 'push lightuserdata wrapper'
		} else if constexpr (std::is_member_function_pointer_v<T>) {
			//push a c function that calls the member method (and transforms all the arguments)
			lua_pushcfunction(L, memberMethodWrapper<field>);
		}
	}

	virtual void read(Base & obj, lua_State * L, int index) const override {
		if constexpr(std::is_member_object_pointer_v<T>) {
			using Value = typename Common::MemberPointer<T>::FieldType;
			obj.*field = LuaRW<Value&>::read(L, index);
		} else if constexpr (std::is_member_function_pointer_v<T>) {
			luaL_error(L, "this field is read only");
		}
	}
};

// generalized __len, __index, __newindex, and __ipairs access

// child needs to provide IndexAccessRead, IndexAccessWrite, IndexLen
template<typename CRTPChild, typename Type, typename Elem>
struct IndexAccess {
	static int __index(lua_State * L, Type & o) {
		// see if we have any field access ...
		if (lua_type(L, 2) == LUA_TSTRING) {
			return default__index<Type>(L);
		}

		if (lua_type(L, 2) != LUA_TNUMBER) {
			lua_pushnil(L);
			return 1;
		}
		// TODO technically tointeger will cast floats to ints
		// whereas true Lua behavior would return nil for non-int floats ...
		int i = lua_tointeger(L, 2);
		--i;
		// using 1-based indexing. sue me.
		if (i < 0 || i >= CRTPChild::IndexLen(o)) {
			lua_pushnil(L);
			return 1;
		}
		CRTPChild::IndexAccessRead(L, o, i);
		return 1;
	}

	static int __newindex(lua_State * L, Type & o) {
		if (lua_type(L, 2) != LUA_TNUMBER) {
// TODO implement this as a Lua-table write-protection flag in whoever (Field/Bind) is exposing it?
#if 0	// option 1: no new fields
			luaL_error(L, "can only write to index elements");
#endif
#if 1	// option 2: write the Lua value into the Lua table
			lua_rawset(L, 1);
			return 0;
#endif
		}
		// TODO technically tointeger will cast floats to ints
		// whereas true Lua behavior would return nil for non-int floats ...
		int i = lua_tointeger(L, 2);
		--i;
		// using 1-based indexing. sue me.
		if (i < 0 || i >= CRTPChild::IndexLen(o)) {
			// *technically* you could just write in the Lua index ... but ... ....... I don't want to deal with mixing those two up
			luaL_error(L, "index %d is out of bounds", i+1);
		}
		CRTPChild::IndexAccessWrite(L, o, i);
		return 1;
	}

	static void IndexAccessRead(lua_State * L, Type & o, int i) {
		LuaRW<
			// will this be Elem& operator[] if available?
			//seems to be the case
			decltype(o[i])
		>::push(L, CRTPChild::IndexAt(L, o, i));
	}

	static void IndexAccessWrite(lua_State * L, Type & o, int i) {
		if constexpr (std::is_reference_v<
			decltype(CRTPChild::IndexAt(L, o, i))
		>) {
			// will error if you try to write a non-prim
			CRTPChild::IndexAt(L, o, i) = LuaRW<Elem>::read(L, 3);
		} else {
			luaL_error(L, "cannot write to field");
			throw Common::Exception() << "cannot write to field";
		}
	}

	static int __len(lua_State * L, Type & o) {
		lua_pushinteger(L, CRTPChild::IndexLen(o));
		return 1;
	}

	//private?
	static int IpairsNext(lua_State * L) {
		auto & o = *lua_getptr<Type>(L, 1);
		int keytype = lua_type(L, 2);
		if (keytype == LUA_TNIL) {
			if (CRTPChild::IndexLen(o) == 0) {
				// empty
				return 0;
			}
			//first key
			lua_pushinteger(L, 1);
			CRTPChild::IndexAccessRead(L, o, 0);
			return 2;
		} else if (keytype == LUA_TNUMBER) {
			// TODO technically tointeger will cast floats to ints
			// whereas true Lua behavior would return nil for non-int floats ...
			// ofc if this is ipairs then I can assert I'm providing the state so ...
			int i = lua_tointeger(L, 2);
			if (i >= CRTPChild::IndexLen(o)) {
				return 0;
			}
			lua_pushinteger(L, i+1);
			CRTPChild::IndexAccessRead(L, o, i);
			return 2;
		} else {
			luaL_error(L, "invalid key");
			throw Common::Exception() << "invalid key";
		}
	}

	static int __ipairs(lua_State * L, Type & o) {
		lua_pushcfunction(L, IpairsNext);
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		return 3;
	}

	// TODO pairs...
	// ... but a pairs that overrides the base-class pairs ...
	// ... because TODO pairs in the base class that iterates over the fields ...
	static int __pairs(lua_State * L, Type & o) {
		lua_pushcfunction(L, (default_pairsNext<
			Type,
			IpairsNext	// once default next is done, it hands off to our ipairs
		>));
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		return 3;
	}
};

// infos for stl
// TODO put this in its own file?

template<typename Elem>
struct Bind<std::vector<Elem>>
:	public BindStructBase<std::vector<Elem>>,
	public IndexAccess<
		Bind<std::vector<Elem>>,
		std::vector<Elem>,
		Elem
	>
{
	using Super = BindStructBase<std::vector<Elem>>;
	using Type = std::vector<Elem>;

	static constexpr std::string_view strpre = "std::vector<";
	static constexpr std::string_view strsuf = ">";
	static constexpr std::string_view mtname = Common::join_v<strpre, Bind<Elem>::mtname, strsuf>;

	static Elem & IndexAt(lua_State * L, Type & o, int i) {
		return o[i];
	}

	static int IndexLen(Type const & o) {
		return o.size();
	}

	static auto & getFields() {
		static auto field_data = Field<static_cast<Elem * (Type::*)()>(&Type::data)>();
		static auto field_size = Field<&Type::size>();
		// TODO iterator exposure?
		static std::map<std::string, FieldBase<Type>*> fields = {
			{"data", &field_data},
			{"size", &field_size},
		};
		return fields;
	}
};

}
