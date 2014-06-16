#include "Common/Test.h"
#include "LuaCxx/State.h"
#include "LuaCxx/GlobalTable.h"
#include <string>

int main() {
	//test loading code and reading optionally-available values
	{
		LuaCxx::State lua;
		
		lua.loadString("a = 1");
		int a = -1;
		TEST_EQ((lua["a"] >> a).good(), true);
		TEST_EQ(a, 1);

		lua.loadString("b = 2.5");
		double b = -1.;
		TEST_EQ((lua["b"] >> b).good(), true);
		TEST_EQ(b, 2.5);

		lua.loadString("c = false");
		bool c = true;
		TEST_EQ((lua["c"] >> c).good(), true);
		TEST_EQ(c, false);

		lua.loadString("d = 'testing'");
		std::string d;
		TEST_EQ((lua["d"] >> d).good(), true);
		TEST_EQ(d, "testing");
	}

#if 0
	//test on reading certainly available values
	{
		LuaCxx::State lua;
		
		lua.loadString("a = 1");
		int a = lua["a"];
		TEST_EQ(a, 1);

		lua.loadString("b = 2.5");
		double b = lua["b"];
		TEST_EQ(b, 2.5);

		lua.loadString("c = false");
		bool c = lua["c"];
		TEST_EQ(c, false);

		lua.loadString("d = 'testing'");
		std::string d = lua["d"];
		TEST_EQ(d, "testing");
	}
#endif

	//testing functions
	{
		LuaCxx::State lua;
		//I've only got callbacks working by direct call.
		// no returning std::function's just yet
		lua.loadString("function e(a,b,c) return a+b+c end");
		int e = lua["e"].call<int>(1,2,4);
		TEST_EQ(e, 7);

	}


	return 0;
}

