#include <iostream>
#include <deque>
#include <list>

#include <neu/nvar.h>
#include <neu/NSocket.h>
#include <neu/NProgram.h>
#include <neu/NListener.h>
#include <neu/NSys.h>
#include <neu/NSocket.h>
#include <neu/NListener.h>
#include <neu/NProc.h>
#include <neu/NCommunicator.h>
#include <neu/NJSONParser.h>
#include <neu/NJSONGenerator.h>

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  NJSONParser parser;
  nvar v = parser.parseFile("test.json");

  cout << "v is: " << v << endl;

  stringstream sstr;
  NJSONGenerator generator;
  generator.generate(sstr, v);

  cout << "gen is: " << sstr.str() << endl;

  nvar v2;
  v2("a") = none;
  v2("b") = undef;
  v2("c") = true;
  v2("d") = false;
  v2("e") = nrat(1, 3);
  v2("f") = nreal("1.23456789123456789123456789");
  v2("g") = nsym("x");
  v2("h") = nvec({1,2,3});

  nlist l;
  l.push_back(1);
  l.push_back(2);
  l.push_back(3);

  v2("i") = l;

  v2("j") = nqueue({1,2,3});
  v2("k") = nfunc("Add") << 1 << 2;
  
  nset s;
  s.add("1");
  s.add("2");
  s.add("3");

  v2("l") = s;

  nhset hs;
  hs.add("1");
  hs.add("2");
  hs.add("3");

  v2("l") = hs;

  nvar hm = nhmap();
  hm("a") = 1.3;
  hm("b") = 2.7;

  v2("m") = hs;

  v2("n") = hm;
  v2["n"].setHead(29);
  v2["n"].pushBack(21);
  v2["n"].pushBack(22);
  v2["n"].pushBack(23);

  v2(nvec() << 1 << 2 << 3) = 99;

  stringstream sstr2;
  generator.generate(sstr2, v2);

  cout << "gen2 is: " << sstr2.str() << endl;

  nvar v3 = parser.parse(sstr2.str());

  cout << "v3 is: " << v3 << endl;

  return 0;
}
