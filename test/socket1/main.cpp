#include <neu/nvar.h>
#include <neu/NSocket.h>
#include <neu/NProgram.h>
#include <neu/NListener.h>
#include <neu/NSys.h>
#include <neu/NListener.h>

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  NListener listener;
  listener.listen(5255);
  NSocket* socket = listener.accept();

  char buf[1];
  buf[0] = 0;

  for(size_t i = 0; i < 1000000; ++i){
    socket->receive(buf, 1);

    cout << "got it: " << i << endl;

    socket->send(buf, 1);
  }

  return 0;
}
