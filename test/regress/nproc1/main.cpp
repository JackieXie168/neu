#include <iostream>

#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NProc.h>
#include <neu/NSys.h>

using namespace std;
using namespace neu;

class Proc1 : public NProc{
public:
  Proc1()
    : i_(0){

  }

  bool handle(const nvar& v, nvar& r){
    return true;
  }

  void run(nvar& r){
    ++i_;
    if(i_ % 1000000 == 0){
      //cout << "i is: " << i_ << endl;
    }
    signal(task_, pn_);
  }

  void setNext(NProc* pn){
    pn_ = pn;
  }

  void setTask(NProcTask* task){
    task_ = task;
  }

private:
  NProcTask* task_;
  NProc* pn_;
  size_t i_;
};

class Proc2 : public NProc{
public:
  bool handle(const nvar& v, nvar& r){
    return true;
  }

  void run(nvar& r){
    signal(task_, pn_);
  }

  void setNext(NProc* pn){
    pn_ = pn;
  }

  void setTask(NProcTask* task){
    task_ = task;
  }

private:
  NProcTask* task_;
  NProc* pn_;
};

int main(int argc, char** argv){
  NProgram program(argc, argv);

  NProcTask* task = new NProcTask(1);
  Proc1 p1;
  Proc2 p2;

  p1.setTask(task);
  p2.setTask(task);

  p1.setNext(&p2);
  p2.setNext(&p1);

  nvar r;
  task->queue(&p1, r);

  NSys::sleep(5);

  delete task;

  return 0;
}
