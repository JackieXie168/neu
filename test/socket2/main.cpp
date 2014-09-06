#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NSys.h>
#include <neu/NSocket.h>

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  NSocket socket;

  assert(socket.connect("localhost", 5255));

  char buf[1];
  buf[0] = 0;

  for(size_t i = 0; i < 1000000; ++i){
    socket.send(buf, 1);

    cout << "got it: " << i << endl;

    socket.receive(buf, 1);
  }

  return 0;
}
