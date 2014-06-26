#include "LuaCxx/Stack.h"

namespace LuaCxx {

Stack::Stack(State* state_)
: state(state_)
, topIndex(lua_gettop(state->getState()))
{
}

//copy constructor
Stack::Stack(const Stack& other) {
	throw Common::Exception() << "stack copy not allowed";
}

//move constructor
Stack::Stack(Stack&& other) {
	state = other.state;
	topIndex = other.topIndex;
	other.state = NULL;
}

Stack::~Stack() {
	if (&state) lua_settop(state->getState(), topIndex);
}
	
Stack& Stack::call(int nargs, int nret) {
	state->call(nargs, nret);
	return *this;
}

int Stack::top() {
	return lua_gettop(state->getState());
}

}

