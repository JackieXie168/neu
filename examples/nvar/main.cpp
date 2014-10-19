#include <iostream>
#include <sstream>

#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NMLParser.h>

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  // in this example, we will highlight some of the basics of working
  // with nvar's

  // for demonstration purposes, we will first read in an nvar
  // specified in NML
  NMLParser parser;
  nvar planets = parser.parseFile("planets.nml");
  
  // note that if the file failed to parse, we would get
  // back none, and error messages would be written to the parser's
  // error stream

  cout << "planets is: " << planets << endl;

  // we could have constructed this programmatically in C++ with
  // something like this:
  nvar planets2;
  nvar& earth = planets2("Earth");  // put a new keyed item Earth
  earth << npair("mass", 5.97218e+24) << npair("radius", 6371) << 
    npair("volume", 1.08321e+12);
  
  // an alternate way accomplishing what we did in the last step
  nvar& saturn = planets2("Saturn");
  saturn("mass") = 5.6846e+26;
  saturn("radius") = 58232;
  saturn("volume") = 8.2713e+14;

  // here's yet another way
  nvar& jupiter = planets2("Jupiter");
  jupiter.map({"mass", 1.8986e+27, "radius", 69911, "volume", 1.4313e+15});

  // for the remaining planet, let's use the convenience function nml()
  // to parse a string of NML
  
  // note that key names do not have to be lexical symbols, they can
  // be strings, actually any arbitrary kind of nvar 
  nvar& unknown = planets2("Unknown Planet");
  unknown = nml("[mass:1.4336e+23, radius:4211, volume:2.3522e+09]");

  // note that in manipulating an nvar with nested data, it can be
  // convenient to obtain references to the nested components in turn
  // as we need them:
  const nvar& j = planets["Jupiter"];
  const nvar& mass = j["mass"];
  cout << "Jupiter's mass is: " << mass << endl;

  // Oops, we made a mistake, let's correct it
  nvar& mass2 = planets["Unknown Planet"]["mass"];
  mass2 += 1000;
  planets["Unknown Planet"]["radius"] = 4512;

  // let's save our planets database to compressed binary format
  planets.save("planets.var");

  // and read it back in from the file
  nvar planets3;
  planets3.open("planets.var");

  // later, we measured the precision of our unknown planet's mass
  // with ultra-high precision
  planets3["Unknown Planet"]["mass"] = 
    nreal("1.4336789722343243492347234945645654672980982304324374e+23");

  cout << "planets3 is: " << planets3 << endl;

  // let's print out the mass of each planet (using first the slower way)
  nvec keys;
  planets3.keys(keys);
  for(const nstr& key : keys){
    cout << "The mass of " << key << " is: " << 
      planets3[key]["mass"] << endl;
  }

  // in some cases where we need higher performance, since we know our
  // top-level contains a map, we can do:
  const nmap& m = planets3;
  for(auto& itr : m){
    const nvar& k = itr.first;
    const nvar& v = itr.second;

    cout << "Again, the mass of " << k << " is: " << v["mass"] << endl;
  }

  // this concludes the tutorial of nvar, note that nvar's can also
  // hold string data, symbolic and functional data, sequences, and more
  // which can be manipulated in similar ways. We recommend looking at
  // nvar.h and nvar.cpp for further details
  
  return 0;
}
