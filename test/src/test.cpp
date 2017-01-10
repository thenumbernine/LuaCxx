#include "Common/Test.h"
#include "LuaCxx/State.h"
#include "LuaCxx/GlobalTable.h"
#include "LuaCxx/Stack.h"
#include <string>
#include <cassert>

int main() {
	std::cout << "reference tests" << std::endl;
	
	std::cout << "test loading code and reading optionally-available values" << std::endl;
	{
		LuaCxx::State lua;
		
		lua << "a = 1";
		TEST_EQ((int)lua["a"], 1);

		lua << "b = 2.5";
		TEST_EQ((double)lua["b"], 2.5);

		lua << "c = false";
		TEST_EQ((bool)lua["c"], false);

		lua << "d = 'testing'";
		//this fails to build ...
		//TEST_EQ((std::string)lua["d"], "testing");
		TEST_EQ((std::string)lua["d"], "testing");
	}


	std::cout << "testing functions" << std::endl;
	{
		LuaCxx::State lua;
		//I've only got callbacks working by direct call.
		// no returning std::function's just yet
		lua << "function e(a,b,c) return a+b+c end";
		TEST_EQ((int)lua["e"](1,2,4), 7);
	}

	std::cout << "table lengths" << std::endl;
	{
		LuaCxx::State lua;
		
		lua << "t = {}";
		TEST_EQ(lua["t"].len(), 0);
		
		lua << "t = {'a'}";
		TEST_EQ(lua["t"].len(), 1);
		
		lua << "t = {'a', 'b'}";
		TEST_EQ(lua["t"].len(), 2);
	}

	std::cout << "ref eq/ne" << std::endl;
	{
		LuaCxx::State lua;
		lua << "t = {}";
		LuaCxx::Ref t1 = lua["t"];
		LuaCxx::Ref t2 = lua["t"];
		std::cout << "verify equality from separate references works" << std::endl;
		TEST_EQ(t1 == t2, true);
		TEST_EQ(t1 != t2, false);

		lua << "s = {}";
		LuaCxx::Ref s = lua["s"];
		std::cout << "verify inequality works" << std::endl;
		TEST_EQ(t1 == s, false);
		TEST_EQ(t1 != s, true);
		TEST_EQ(t2 == s, false);
		TEST_EQ(t2 != s, true);
	}

	std::cout << "iterators" << std::endl;
	{
		LuaCxx::State lua;
		lua << "t = {a=1, b=2, c=3}";
		LuaCxx::Ref t = lua["t"];
		for (LuaCxx::Ref::iterator iter = t.begin(); iter != t.end(); ++iter) {
			std::cout << (std::string)iter.key << " = " << (int)iter.value << std::endl;
		}
	}

	std::cout << "stack tests" << std::endl;
	{
		LuaCxx::State lua;
		LuaCxx::Stack stack = lua.stack();
		
		TEST_EQ(stack.top(), 0);
	
		std::cout << "pushing and popping single values" << std::endl;
		{
			int a = -1;
			stack << 2;
			TEST_EQ(stack.top(), 1);
			stack >> a;
			TEST_EQ(stack.top(), 0);
			TEST_EQ(a, 2);
		}

		std::cout << "pushing and popping single values" << std::endl;
		std::cout << "order of operations" << std::endl;
		{
			int a = -1;
			stack << 2 >> a;
			TEST_EQ(a, 2);
		}

		std::cout << "pushing and popping multiple values" << std::endl;
		{
			int a = -1;
			int b = -1;
			stack << 4 << 5 >> a >> b;
			TEST_EQ(a, 5);
			TEST_EQ(b, 4);
			TEST_EQ(stack.top(), 0);
		}

		std::cout << "popping globals" << std::endl;
		{
			int a = -1;
			lua << "a = 3";
			stack.getGlobal("a");
			TEST_EQ(stack.top(), 1);
			stack >> a;
			TEST_EQ(stack.top(), 0);
			TEST_EQ(a, 3);
		}

		std::cout << "single return values" << std::endl;
		{
			int a = -1;
			lua << "function c(a,b,c) return a+b+c end";
			stack.getGlobal("c") << 1 << 2 << 3;
			TEST_EQ(stack.top(), 4);
			stack.call(3,1);
			TEST_EQ(stack.top(), 1);
			stack >> a;
			TEST_EQ(a, 6);
			TEST_EQ(stack.top(), 0);
		}

		std::cout << "concise single return value " << std::endl;
		{
			int a = -1;
			stack
			.getGlobal("c")
			.push(1,2,3)
			.call(3,1)
			.pop(a);
			TEST_EQ(a, 6);
		}

		std::cout << "concise multiple return values" << std::endl;
		{
			lua << "function d(a,b,c) return b+c, c+a, a+b end";
			int a = -1;
			int b = -1;
			int c = -1;
			stack
			.getGlobal("d")
			.push(1,2,3)
			.call(3,3)
			.pop(a,b,c);
		}

		std::cout << "return nested tables" << std::endl;
		{
			lua << "function e(a,b,c) return {a={a}, b={{b}}, c={{{c}}}} end";
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
	}
	
	std::cout << "done!" << std::endl;

	return 0;
}

