#include "Common/Test.h"
#include "LuaCxx/State.h"
#include <string>

int main() {
	//test reading values
	{
		LuaCxx::State L;
		
		L.loadString("a = 1");
		int a = -1;
		TEST_EQ((L._G()["a"] >> a).good(), true);
		TEST_EQ(a, 1);

		L.loadString("b = 2.5");
		double b = -1.;
		TEST_EQ((L._G()["b"] >> b).good(), true);
		TEST_EQ(b, 2.5);

		L.loadString("c = false");
		bool c = true;
		TEST_EQ((L._G()["c"] >> c).good(), true);
		TEST_EQ(c, false);

		L.loadString("d = 'testing'");
		std::string d;
		TEST_EQ((L._G()["d"] >> d).good(), true);
		TEST_EQ(d, "testing");
	}

	return 0;
}

