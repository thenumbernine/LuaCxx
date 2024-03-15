#pragma once

/*
code for automatic generation of Lua bindings for any structs
just provide a Bind<T> class and a getFields() method to return a map of fields.

TODO organize better.  move all the impl code into an impl{} namespace or something maybe not idk.

TODO the LuaRW is very similar to LuaCxx/pushToLuaState and readFromLuaState ...
 ... but for this there are cases specific to the light/heavy userdata that the C++ code is using
I'll think of how to merge the two someday...
Honestly, this Bind.h is separate of all ther est of LuaCxx, which was originally designed just to be a wrapper for using LuaRefs and the global registery in C++...
... maybe this should be in its own repo?
*/

#include "Common/String.h"	//Common::join_v
#include <lua.hpp>
#include <type_traits>

inline void * operator new(size_t size, lua_State * L) {
	return lua_newuserdata(L, size);
}

namespace LuaCxx {

// here' all the generic lua-binding stuff

template<typename Type>
struct Bind;

// infos for prims.  doesn't have lua exposure, only mtname for mtname joining at compile time
// TODO combine this with the LuaRW stuff?  maybe? maybe not?

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
static int default__index(lua_State * L) {
	auto & o = *lua_getptr<T>(L, 1);
	char const * const k = lua_tostring(L, 2);
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
	char const * const k = lua_tostring(L, 2);
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

template<typename T>
static int default__tostring(lua_State * L) {
	auto & o = *lua_getptr<T>(L, 1);
	lua_pushfstring(L, "%s: 0x%x", Bind<T>::mtname.data(), &o);
	return 1;
}

// base infos for all structs...
// TODO just template this? or can I?  cuz the template arg needs the member ptr, which then would guarantee it already exists ....
template<typename T> constexpr bool has__index_v = requires(T const & t) { t.__index; };
template<typename T> constexpr bool has__newindex_v = requires(T const & t) { t.__newindex; };
template<typename T> constexpr bool has__len_v = requires(T const & t) { t.__len; };
template<typename T> constexpr bool has__ipairs_v = requires(T const & t) { t.__ipairs; };
template<typename T> constexpr bool has__call_v = requires(T const & t) { t.__call; };
template<typename T> constexpr bool has__tostring_v = requires(T const & t) { t.__tostring; };
template<typename T> constexpr bool has_mt_ctor_v = requires(T const & t) { t.mt_ctor; };

template<typename T>
struct BindStructBase {

	// add the class constructor as the call operator of the metatable
	static void initMtCtor(lua_State * L) {
		static constexpr std::string_view suffix = " metatable";
		static constexpr std::string_view mtname = Common::join_v<Bind<T>::mtname, suffix>;
		if (!luaL_newmetatable(L, mtname.data())) return;

		lua_pushcfunction(L, Bind<T>::mt_ctor);
		lua_setfield(L, -2, "__call");
	}

	/*
	initialize the metatable associated with this type
	*/
	static void mtinit(lua_State * L) {
		auto const & mtname = Bind<T>::mtname;

		for (auto & pair : Bind<T>::getFields()) {
			pair.second->mtinit(L);
		}

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
		lua_pop(L, 1);
	}
};


// general case just wraps the memory
template<typename T>
struct LuaRW {
	static void push(lua_State * L, T & v) {
		// hmm, we want a metatable for what we return
		// but lightuserdata has no metatable
		// so it'll have to be a new lua table that points back to this
		lua_newtable(L);
#if 1	// if the metatable isn't there then it won't be set
		luaL_setmetatable(L, Bind<T>::mtname.data());
#endif
#if 0 	//isn't this supposed to do the same as luaL_setmetatable ?
		luaL_getmetatable(L, Bind<T>::mtname.data());
std::cout << "metatable " << Bind<T>::mtname << " type " << lua_type(L, -1) << std::endl;
		lua_setmetatable(L, -2);
#endif
#if 0 // this say ssomething is there, but it always returns nil
		lua_getmetatable(L, -1);
std::cout << "metatable " << Bind<T>::mtname << " type " << lua_type(L, -1) << std::endl;
		lua_pop(L, 1);
#endif
		lua_pushliteral(L, LUACXX_BIND_PTRFIELD);
		lua_pushlightuserdata(L, &v);
		lua_rawset(L, -3);
	}

	static T read(lua_State * L, int index) {
		luaL_error(L, "this field cannot be overwritten");
		throw std::runtime_error("this field cannot be overwritten");
		//return {};	// hmm this needs the default ctor to exist, but I'm throwing,
					// so it doesn't really need to exist ...
	}
};


template<typename T>
requires (std::is_floating_point_v<T>)
struct LuaRW<T> {
	static void push(lua_State * L, T v) {
		lua_pushnumber(L, (lua_Number)v);
	}
	static T read(lua_State * L, int index) {
		return (T)lua_tonumber(L, index);
	}
};

template<typename T>
requires (std::is_integral_v<T>)
struct LuaRW<T> {
	static void push(lua_State * L, T v) {
		lua_pushinteger(L, v);
	}

	static T read(lua_State * L, int index) {
		// TODO what to do if the type doesn't match? i.e. assigning a string to an int?
		// right now it just assigns 0 ...
		return lua_tointeger(L, index);
	}
};

template<typename Base>
struct FieldBase {
	virtual ~FieldBase() {};
	// o can't be const because o.*field can't be const because in case it's a blob/ptr then it's getting pushed into lua as lightuserdata ... and can't be const
	virtual void push(Base & o, lua_State * L) const = 0;
	virtual void read(Base & o, lua_State * L, int index) const = 0;
	virtual void mtinit(lua_State * L) const = 0;
};

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

	if constexpr (std::is_same_v<Return, void>) {
		[&]<auto...j>(std::index_sequence<j...>) -> decltype(auto) {
			(o.*field)(
				LuaRW<std::tuple_element_t<j, Args>>::read(L, j+1)...
			);
		}(std::make_index_sequence<std::tuple_size_v<Args>>{});
		return 0;
	} else {
		LuaRW<Return>::push(L,
			[&]<auto...j>(std::index_sequence<j...>) -> decltype(auto) {
				return (o.*field)(
					LuaRW<std::tuple_element_t<j, Args>>::read(L, j+1)...
				);
			}(std::make_index_sequence<std::tuple_size_v<Args>>{})
		);
		return 1;
	}
}

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
			LuaRW<Value>::push(L, obj.*field);
		} else if (std::is_member_function_pointer_v<T>) {
			//push a c function that calls the member method (and transforms all the arguments)
			lua_pushcfunction(L, memberMethodWrapper<field>);
		}
	}

	virtual void read(Base & obj, lua_State * L, int index) const override {
		if constexpr(std::is_member_object_pointer_v<T>) {
			using Value = typename Common::MemberPointer<T>::FieldType;
			obj.*field = LuaRW<Value>::read(L, index);
		} else if (std::is_member_function_pointer_v<T>) {
			luaL_error(L, "this field is read only");
		}
	}

	virtual void mtinit(lua_State * L) const override {
		if constexpr(std::is_member_object_pointer_v<T>) {
			using Value = typename Common::MemberPointer<T>::FieldType;
			if constexpr (std::is_class_v<Value>) {
				Bind<Value>::mtinit(L);
			}
		} else if (std::is_member_function_pointer_v<T>) {
			using Return = typename Common::MemberMethodPointer<T>::Return;
			if constexpr (std::is_class_v<Return>) {
				Bind<Return>::mtinit(L);
			}
			// need to add arg types too or nah?  nah... returns are returned, so they could be a first creation of that type. not args.
		}
	}
};

#if 0
template<auto field>
requires (std::is_member_function_pointer_v<decltype(field)>)
struct Field : public FieldBase<
	typename Common::MemberMethodPointer<decltype(field)>::Class
> {
	using MP = Common::MemberMethodPointer<decltype(field);
	using Base = typename MP::Class;
	using Return = typename MP::Return;

	virtual void push(Base & obj, lua_State * L) const override {
		//LuaRW<Return>::push(L, obj.*field);
		lua_pushcfunction(L, field);
	}

	virtual void read(Base & obj, lua_State * L, int index) const override {
		//obj.*field = LuaRW<Return>::read(L, index);
		luaL_error(L, "field is read-only");
	}

	virtual void mtinit(lua_State * L) const override {
		if constexpr (std::is_class_v<Return>) {
			Bind<Return>::mtinit(L);
		}
	}
};
#endif

// generalized __len, __index, __newindex, and __ipairs access

// child needs to provide IndexAccessRead, IndexAccessWrite, IndexLen
template<typename CRTPChild, typename Type, typename Elem>
struct IndexAccessReadWrite {
	static int __index(lua_State * L, Type & o) {
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
			luaL_error(L, "can only write to index elements");
		}
		// TODO technically tointeger will cast floats to ints
		// whereas true Lua behavior would return nil for non-int floats ...
		int i = lua_tointeger(L, 2);
		--i;
		// using 1-based indexing. sue me.
		if (i < 0 || i >= CRTPChild::IndexLen(o)) {
			luaL_error(L, "index %d is out of bounds", i+1);
		}
		CRTPChild::IndexAccessWrite(L, o, i);
		return 1;
	}

	static int __len(lua_State * L, Type & o) {
		lua_pushinteger(L, CRTPChild::IndexLen(o));
		return 1;
	}

	//private?
	static int IpairsNext(lua_State * L) {
		auto & o = *lua_getptr<Type>(L, 1);
		if (lua_isnil(L, 2)) {
			//first key
			lua_pushinteger(L, 1);
			CRTPChild::IndexAccessRead(L, o, 0);
			return 2;
		}
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
	}

	static int __ipairs(lua_State * L, Type & o) {
		lua_pushcfunction(L, IndexAccessReadWrite::IpairsNext);
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		return 3;
	}

	// TODO pairs...
	// ... but a pairs that overrides the base-class pairs ...
	// ... because TODO pairs in the base class that iterates over the fields ...
};

// CRTPChild needs to provide IndexAt, IndexLen
// IndexAt is 0-based despite the Lua indexes being 1-based
template<typename CRTPChild, typename Type, typename Elem>
struct IndexAccess
: public IndexAccessReadWrite<
	IndexAccess<CRTPChild, Type, Elem>,	// pass the IndexAccess as the new CRTPChild so it can see the IndexAccessRead and IndexAccessWrite here
	Type,
	Elem
>
{
	static void IndexAccessRead(lua_State * L, Type & o, int i) {
		LuaRW<Elem>::push(L, CRTPChild::IndexAt(L, o, i));
	}

	static void IndexAccessWrite(lua_State * L, Type & o, int i) {
		// will error if you try to write a non-prim
		CRTPChild::IndexAt(L, o, i) = LuaRW<Elem>::read(L, 3);
	}

	//use CRTPChild's IndexLen
	static int IndexLen(Type const & o) {
		return CRTPChild::IndexLen(o);
	}
};

// infos for stl

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

	// vector needs Elem's metatable initialized
	static void mtinit(lua_State * L) {
		Super::mtinit(L);

		//init all subtypes
		//this test is same as in Field::mtinit
		if constexpr (std::is_class_v<Elem>) {
			Bind<Elem>::mtinit(L);
		}
	}

	static Elem & IndexAt(lua_State * L, Type & o, int i) {
		return o[i];
	}

	static int IndexLen(Type const & o) {
		return o.size();
	}

	static auto & getFields() {
		static std::map<std::string, FieldBase<Type>*> fields = {
		};
		return fields;
	}
};

}
