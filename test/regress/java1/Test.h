// The Neu Java interface works in conjunction with a header such as
// this one which defines the C++ interface which mirrors the Java
// class to be called. All methods parameter types and return types
// must be one of:
//
//  -a primitive, e.g: int, double, etc.
//  -a string: nstr or const nstr&
//  -an nvar or: const nvar& or nvar& (references can only be used with params)
//
// The neu-java command will then be used to automatically generate
// the corresponding .cpp file implementation for this class. The Java
// file should mirror this header (see Test.java), but may contain
// additional Java-only methods. The class below can then be called
// as if it were an ordinary C++ class.

#include <neu/NJObject.h>

// must always derive from NJObject
class Test : public neu::NJObject{
public:
  Test();

  static void foo(int x);

  neu::nvar bar(const neu::nvar& v);
};
