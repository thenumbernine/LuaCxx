#include "Common/Test.h"
#include "LuaCxx/State.h"
#include "LuaCxx/GlobalTable.h"
#include "LuaCxx/Stack.h"
#include <string>

int main() {
	//reference tests
	
	//test loading code and reading optionally-available values
	{
		LuaCxx::State lua;
		
		lua.loadString("a = 1");
		int a = -1;
		TEST_EQ((lua.ref()["a"] >> a).good(), true);
		TEST_EQ(a, 1);

		lua.loadString("b = 2.5");
		double b = -1.;
		TEST_EQ((lua.ref()["b"] >> b).good(), true);
		TEST_EQ(b, 2.5);

		lua.loadString("c = false");
		bool c = true;
		TEST_EQ((lua.ref()["c"] >> c).good(), true);
		TEST_EQ(c, false);

		lua.loadString("d = 'testing'");
		std::string d;
		TEST_EQ((lua.ref()["d"] >> d).good(), true);
		TEST_EQ(d, "testing");
	}


	//testing functions
	{
		LuaCxx::State lua;
		//I've only got callbacks working by direct call.
		// no returning std::function's just yet
		lua.loadString("function e(a,b,c) return a+b+c end");
		int e = -1;
		TEST_EQ((lua.ref()["e"](1,2,4) >> e).good(), true);
		TEST_EQ(e, 7);

	}

	//stack tests
	{
		LuaCxx::State lua;
		LuaCxx::Stack stack = lua.stack();
		
		TEST_EQ(stack.top(), 0);
	
		//pushing and popping single values
		{
			int a = -1;
			stack << 2;
			TEST_EQ(stack.top(), 1);
			stack >> a;
			TEST_EQ(stack.top(), 0);
			TEST_EQ(a, 2);
		}

		//pushing and popping single values
		//order of operations
		{
			int a = -1;
			stack << 2 >> a;
			TEST_EQ(a, 2);
		}

		//pushing and popping multiple values
		{
			int a = -1;
			int b = -1;
			stack << 4 << 5 >> a >> b;
			TEST_EQ(a, 5);
			TEST_EQ(b, 4);
			TEST_EQ(stack.top(), 0);
		}

		//popping globals
		{
			int a = -1;
			lua.loadString("a = 3");
			stack.getGlobal("a");
			TEST_EQ(stack.top(), 1);
			stack >> a;
			TEST_EQ(stack.top(), 0);
			TEST_EQ(a, 3);
		}

		//single return values
		{
			int a = -1;
			lua.loadString("function c(a,b,c) return a+b+c end");
			stack.getGlobal("c") << 1 << 2 << 3;
			TEST_EQ(stack.top(), 4);
			stack.call(3,1);
			TEST_EQ(stack.top(), 1);
			stack >> a;
			TEST_EQ(a, 6);
			TEST_EQ(stack.top(), 0);
		}

		//concise single return value 
		{
			int a = -1;
			stack
			.getGlobal("c")
			.push(1,2,3)
			.call(3,1)
			.pop(a);
			TEST_EQ(a, 6);
		}

		//concise multiple return values
		{
			lua.loadString("function d(a,b,c) return b+c, c+a, a+b end");
			int a = -1;
			int b = -1;
			int c = -1;
			stack
			.getGlobal("d")
			.push(1,2,3)
			.call(3,3)
			.pop(a,b,c);
		}

		//return nested tables
		{
			lua.loadString("function e(a,b,c) return {a={a}, b={{b}}, c={{{c}}}} end");
			int a = -1;
			int b = -1; 
			int c = -1;
			stack
			.getGlobal("e")	//e
			.push(1,2,3)	//e 1 2 3
			.call(3,1)		//t
			.get("a")		//t {a}
			.get(1)			//t {a} a
			.pop(a);		//t {a}
			TEST_EQ(a, 1);
			stack
			.pop()			//t
			.get("b")		//t {{b}}
			.get(1)			//t {{b}} {b}
			.get(1)			//t {{b}} {b} b
			.pop(b);		//t {{b}} {b}
			TEST_EQ(b, 2);
			stack
			.pop()			//t {{b}}
			.pop()			//t
			.get("c")
			.get(1)
			.get(1)
			.get(1)
			.pop(c)
			.pop()
			.pop()
			.pop();
			TEST_EQ(c, 3);
		}

		//table lengths
		{
			lua.loadString("t = {}");
			TEST_EQ(lua.ref()["t"].len(), 0);
			
			lua.loadString("t = {'a'}");
			TEST_EQ(lua.ref()["t"].len(), 1);
			
			lua.loadString("t = {'a', 'b'}");
			TEST_EQ(lua.ref()["t"].len(), 2);
		}
	}
	return 0;
}

