#include "Version.h"
#include <sstream>

using namespace gs::e2;
using namespace std;

string& Version::getVersion() {
	static string v;

	if (v.empty()) {
		ostringstream os;

		os << "GsE2TitanBridge/" << major << '.' << minor << '.' << build;

		v = os.str();
	}

	return v;
}
