#include "Common/Test.h"
#include "LuaCxx/State.h"
#include "LuaCxx/GlobalTable.h"
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
#if 0	
	{
		LuaCxx::State lua;
		lua.loadString("function e(a,b,c) return a+b+c end");
		int e = -1;
		TEST_EQ(lua.get("e")(1,2,3) >> e;
	}
#endif
	return 0;
}

