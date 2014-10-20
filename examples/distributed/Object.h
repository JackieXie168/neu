#ifndef OBJECT_H
#define OBJECT_H

#include <neu/nvar.h>
#include <neu/NObject.h>

namespace neu{

// a Neu distributed object must always derive from NObject
class Object : public NObject{
public:
  // this will be the server-side constructor
  Object();

  // this constructor is required for distributed objects - it will
  // be called when the object is obtained on the client-side
  Object(NBroker* broker);

  // methods which can be called remotely always return type ndist - a
  // typedef to nvar
  ndist bar();
  
  // parameters need not be nvar, but must be N-compatible, i.e: it is
  // possible to construct an nvar with them
  ndist baz(int x);

  // for non-const references, where the changes of the arg need to be
  // passed back to the client, we currently must use an nvar&
  ndist getCount(nvar& count);

  // this prototype is required, the neu-meta command will generate
  // its implementation
  NFunc handle(const nvar& n, uint32_t flags);

private:
  // a Neu distributed object must always contain a pointer named "x_"
  // which points to the "inner object" - the "outer methods" which
  // call the inner methods will be generated automatically by the
  // neu-meta command
  class Object_* x_;
};

} // end namespace neu

#endif // OBJECT_H
