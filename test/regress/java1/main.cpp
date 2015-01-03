#include <iostream>

#include <neu/nvar.h>

#include "Test.h"

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  double d = Test::static1(true, 25, 454, 3214, 99993, 23.4, 1e-23);

  cout << "d ret is: " << d << endl;

  Test t;

  nvar v;
  v("a") = 20;
  v("b") = 30;

  nvar v2 = t.varRet(v);

  cout << "v2 is: " << v2 << endl;

  t.varRef(v2);

  cout << "v2 is now: " << v2 << endl;

  nstr str = t.str("this is a string");

  cout << "str is: " << str << endl;

  return 0;
}
