#include <iostream>

#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NObject.h>
#include <neu/NClass.h>

using namespace std;
using namespace neu;

class StoredObject: public NObject{
public:
  // implement the restore constructor
  // we always call the base class restore constructors
  StoredObject(const nvar& v, RestoreFlag)
    : NObject(v, Restore){
    
    // the data for this class should be stored under the key for this
    // class
    const nvar& sv = v["StoredObject"];
    
    // shorthand for storing the field a_ into the below - equivalent
    // to the below for b
    nget(sv, a_);
    b_ = sv["b_"];

    // we could have restored any arbitary members, e.g: maps,
    // vectors, etc. so long as the restore constructor is consistent
    // with store() we can choose any type of encoding we wish
  }

  // ordinary constructor
  StoredObject(){
    a_ = 0;
    b_ = 0;
  }

  // generated by neu-meta, but we need this prototype
  NFunc handle(const neu::nvar& n, uint32_t flags);

  // store method
  void store(nvar& v){
    // because this could potentially be a subclass, only set the type
    // if not already set
    if(!v.has("type")){
      v("type") = "StoredObject";
    }
    
    // encode the data members under a key of the name of the class
    nvar& sv = v("StoredObject");
    
    // these two achieve the same
    nput(sv, a_);
    sv("b_") = b_;

    // call store on the super class
    NObject::store(v);
  }

  int sum(){
    return a_ + b_;
  }

  void setA(int a){
    a_ = a;
  }

  void setB(int b){
    b_ = b;
  }

private:
  int a_;
  int b_;
};

#ifndef META_GUARD
#include "StoredObject_meta.h"
#endif

int main(int argc, char** argv){
  NProgram program(argc, argv);

  // create a new object
  StoredObject o;
  o.setA(2);
  o.setB(25);

  // run some code through the interpreter

  // define a function which will persist
  o.run(nml("foo(x, y){ return x*y; }"));

  // define a symbol which will persist
  o.run(nml("bar = 5;"));

  // store and save to a file
  nvar v;
  o.store(v);
  v.save("StoredObject.nvar");

  // restore the object from the file
  nvar rv;
  rv.open("StoredObject.nvar");
  StoredObject ro(rv, StoredObject::Restore);

  // verify that our data persisted
  cout << "sum is: " << ro.sum() << endl;

  // alternate way of restoring an object
  NObject* ro2 = static_cast<NObject*>(NClass::recreate(rv));

  nvar f = nfunc("Call") << (nfunc("foo") << 3 << nsym("bar"));

  cout << "foo is: " << ro2->run(f) << endl;

  return 0;
}
