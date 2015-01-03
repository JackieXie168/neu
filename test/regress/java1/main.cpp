#include <iostream>

#include <neu/nvar.h>

#include "Test.h"

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  Test t;

  nvar v;
  v("a") = 20;
  v("b") = 30;

  nvar v2 = t.bar(v);

  cout << "v2 is: " << v2 << endl;

  return 0;
}
