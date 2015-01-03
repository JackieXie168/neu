// The Neu Java interface works in conjunction with a header such as
// this one which defines the C++ interface which mirrors the Java
// class to be called. All method parameter types and return types
// must be one of:
//
//  -a primitive, e.g: int, double, etc.
//  -a string: nstr or const nstr&
//  -an nvar or: const nvar& or nvar& 
//    (references can only be used with params)
//
// The neu-java command will then be used to automatically generate
// the corresponding .cpp file implementation for this class (see the
// Makefile). The -class option passed to neu-java is the fully
// qualified class name of the Java class, e.g: -class
// net/andrometa/neu/Test. The Java class file (e.g: Test.java) should
// match this header, but may contain additional Java-only methods and
// properties. The class below can then be called as if it were an
// ordinary C++ class.

#include <neu/NJObject.h>

// To use the Neu Java interface, you must always derive from NJObject
class Test : public neu::NJObject{
public:
  // ctor could also take params
  Test();

  // test static method
  static double static1(bool b,
                        char c,
                        short s,
                        int i,
                        long l,
                        float f,
                        double d);

  // test nvar in, returned
  neu::nvar varRet(const neu::nvar& v);

  // test nvar reference - v is in/out
  void varRef(neu::nvar& v);

  // test string as input, returned
  neu::nstr str(const neu::nstr& str);

  // the C++ class does not contain any attributes
};

// note: this class could have been declared within a namespace
