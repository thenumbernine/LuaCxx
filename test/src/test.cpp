#include "Common/Test.h"
#include "Lua/Lua.h"
#include <string>

int main() {
	//test reading values
	{
		Lua::Lua lua;
		
		lua.loadString("a = 1");
		int a = -1;
		TEST_EQ((lua._G()["a"] >> a).good(), true);
		TEST_EQ(a, 1);

		lua.loadString("b = 2.5");
		double b = -1.;
		TEST_EQ((lua._G()["b"] >> b).good(), true);
		TEST_EQ(b, 2.5);

		lua.loadString("c = false");
		bool c = true;
		TEST_EQ((lua._G()["c"] >> c).good(), true);
		TEST_EQ(c, false);

		lua.loadString("d = 'testing'");
		std::string d;
		TEST_EQ((lua._G()["d"] >> d).good(), true);
		TEST_EQ(d, "testing");
	}

	return 0;
}

