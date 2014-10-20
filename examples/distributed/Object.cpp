#include "Object.h"

using namespace std;
using namespace neu;

namespace neu{

// the "inner class"
class Object_{
public:
  Object_(Object* o)
  : o_(o),
    count_(0){}

  ndist bar(){
    // if we wanted to compile two separate versions, a client-side
    // and server-side, hiding implemetation code from the client, we
    // could do something like this: 
    // #ifdef SERVER_SIDE
    return count_++;
    //#endif
  }

  ndist baz(int x){
    return count_ += x;
  }

  ndist getCount(nvar& count){
    count = count_;

    return true;
  }

private:
  Object* o_;
  int count_;
};

} // end namespace neu

// this "outer" method cannot be auto-generated and must call the
// NObject constructor passing the broker as in below
Object::Object(NBroker* broker)
  : NObject(broker){
  x_ = new Object_(this);
}

// include the code generated by neu-meta
#ifndef META_GUARD
#include "Object_meta.h"
#endif
