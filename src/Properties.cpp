#include "Properties.h"
#include "FileHelper.h"
#include "Util.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace gs::e2;

Properties::Properties() {

}

Properties::~Properties() {

}

bool Properties::read(const string& fileName) {
	m_props.clear();

	ifstream in;

	in.open(fileName.c_str());

	if (in.is_open()) {
		char buf[512];

		int count = 0;
		string line;
		string name;
		string value;
		string::size_type pos;
		string::size_type len;

		while (in.good() && count != -1) {
			count = FileHelper::readLine(in, buf, 512);

			if (count > 0) {
				line = buf;

				if (line[0] == '#') {
					continue; // comment line
				}

				pos = line.find('=');

				if (pos != string::npos) {
					name = line.substr(0, pos);
					
					Util::trim(name);

					len = line.length();

					if (pos < len - 1) {
						value = line.substr(pos + 1, (len - (pos + 1)));
						Util::trim(value);
					} else {
						value.clear();
					}
					
					m_props[name] = value;
				}
			}
		}

		in.close();

		return true;
	} else {
		return false;
	}
}

const string& Properties::get(const string& key) const {
	static string empty;

	map<string, string>::const_iterator it = m_props.find(key);

	if (it != m_props.end()) {
		return it->second;
	} else {
		return empty;
	}
}

map<string, string> Properties::get() {
	return m_props;
}