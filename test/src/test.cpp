#include "Common/Test.h"
#include "LuaCxx/State.h"
#include <string>

int main() {
	
	//test loading code and reading values
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

	return 0;
}

