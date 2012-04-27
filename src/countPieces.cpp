
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

PicArr makePic(char* a, char* b, char* c, char* d) {
	PicArr arr;
	for(int i = 0; i < 25; ++i)
		arr.v[i] = 0;
	arr.set(1,1) = arr.set(1,2) = arr.set(1,3) = arr.set(2,1) = arr.set(2,2) = arr.set(2,3) = arr.set(3,1) = arr.set(3,2) = arr.set(3,3) = 1;
	if (a[0] == '1') arr.set(0,0) = 1;
	if (a[1] == '1') arr.set(1,0) = 1;
	if (a[2] == '1') arr.set(2,0) = 1;
	if (a[3] == '1') arr.set(3,0) = 1;
	if (a[4] == '1') arr.set(4,0) = 1;

	if (b[0] == '1') arr.set(4,0) = 1;
	if (b[1] == '1') arr.set(4,1) = 1;
	if (b[2] == '1') arr.set(4,2) = 1;
	if (b[3] == '1') arr.set(4,3) = 1;
	if (b[4] == '1') arr.set(4,4) = 1;

	if (c[0] == '1') arr.set(4,4) = 1;
	if (c[1] == '1') arr.set(3,4) = 1;
	if (c[2] == '1') arr.set(2,4) = 1;
	if (c[3] == '1') arr.set(1,4) = 1;
	if (c[4] == '1') arr.set(0,4) = 1;

	if (d[0] == '1') arr.set(0,4) = 1;
	if (d[1] == '1') arr.set(0,3) = 1;
	if (d[2] == '1') arr.set(0,2) = 1;
	if (d[3] == '1') arr.set(0,1) = 1;
	if (d[4] == '1') arr.set(0,0) = 1;
	return arr;
}


bool samePic(PicArr a, const PicArr& b) {
	if (a.equalTo(b)) return true;
	a.turn();
	if (a.equalTo(b)) return true;
	a.turn();
	if (a.equalTo(b)) return true;
	a.turn();
	if (a.equalTo(b)) return true;
	a.revY();
	if (a.equalTo(b)) return true;
	a.turn();
	if (a.equalTo(b)) return true;
	a.turn();
	if (a.equalTo(b)) return true;
	a.turn();
	if (a.equalTo(b)) return true;
	return false;
}

string printPic(const PicArr& p) {
	stringstream ss;
	for(int y = 0; y < 5; ++y) {
		for(int x = 0; x < 5; ++x)
			ss << (p.axx(x, y)?'X':' ');
		ss << '\n';
	}
	return ss.str();
}

class PicSet {
public:
	bool isIn(const PicArr& p) {
		for(int iv = 0; iv < v.size(); ++iv) {
			if (samePic(p, v[iv]))
				return true;
		}
		return false;
	}

	void add(const PicArr& p) {
		if (!isIn(p)) {
			v.push_back(p);
			vs.push_back(printPic(p));
		}

	}
	vector<PicArr> v;
	vector<string> vs;
};

bool isBad(const PicArr& p) {
	// lone corners
	if (p.axx(0,0) && (!p.axx(1,0)) && (!p.axx(0,1)))
		return true;
	if (p.axx(4,0) && (!p.axx(3,0)) && (!p.axx(4,1)))
		return true;
	if (p.axx(4,4) && (!p.axx(3,4)) && (!p.axx(4,3)))
		return true;
	if (p.axx(0,4) && (!p.axx(0,3)) && (!p.axx(1,4)))
		return true;
	// whole 3x3 corners
	int x = p.axx(0,0);
	if (x == p.axx(0,2) && x == p.axx(0,1) && x == p.axx(1,0) && x == p.axx(2,0))
		return true;
	x = p.axx(0,4);
	if (x == p.axx(0,2) && x == p.axx(0,3) && x == p.axx(1,4) && x == p.axx(2,4))
		return true;
	x = p.axx(4,0);
	if (x == p.axx(2,0) && x == p.axx(3,0) && x == p.axx(4,1) && x == p.axx(4,2))
		return true;
	x = p.axx(4,4);
	if (x == p.axx(2,4) && x == p.axx(3,4) && x == p.axx(4,3) && x == p.axx(4,2))
		return true;
	return false;
}

void test() {

	char *edges[] = {
	//	"00000",
	//	"00001",
		"00010",
		"00011",
		"00100",
		"00101",
		"00110",
		"00111",
		"01000",
		"01001",
		"01010",
		"01011",
		"01100",
		"01101",
	//	"01110",
	//	"01111",
	//	"10000",
	//	"10001",
		"10010",
		"10011",
		"10100",
		"10101",
		"10110",
		"10111",
		"11000",
		"11001",
		"11010",
		"11011",
		"11100",
		"11101",
	//	"11110",
	//	"11111"
	};

	PicSet ps;
	int count = 0;
	for(int ia = 0; ia < _countof(edges); ++ia) {
		for(int ib = 0; ib < _countof(edges); ++ib) {
			for(int ic = 0; ic < _countof(edges); ++ic) {
				for(int id = 0; id < _countof(edges); ++id) {
					PicArr pa = makePic(edges[ia], edges[ib], edges[ic], edges[id]);
					++count;
					if ((count % 1000) == 0)
						cout << "\r " << count << "  " << ps.v.size();

					if (isBad(pa))
						continue;
					ps.add(pa);
				}
			}
		}
	}

	cout << "count=" << count << endl;
	cout << "set=" << ps.v.size() << endl;

	ofstream osf("c:/temp/allpics.txt");
	for(int i = 0; i < ps.vs.size(); ++i) {
		osf << ps.vs[i];
		osf << "\n";
	}
	osf.close();
}
