// this example demonstrates Neu's high performance neural networks
// system. NNModule is used to compile a multi-layer feed-forward
// network then execute it. We will execute a network whose layers are
// of size: 2-3000-3000-3000-2

#include <iostream>

#include <neu/NNet.h>
#include <neu/NNModule.h>
#include <neu/NSys.h>
#include <neu/NPLVector.h>

using namespace std;
using namespace neu;

static const double IW = 0.3;
static const double RANGE = 105.0;

int main(int argc, char** argv){
  NNet network;
  network.setSeed(0);

  NNet::Layer* inputLayer = new NNet::Layer(2);
  network.addLayer(inputLayer);
  network.addLayer(NNet::sigmoidFunc, 3000, -IW, IW);
  network.addLayer(NNet::sigmoidFunc, 3000, -IW, IW);
  network.addLayer(NNet::sigmoidFunc, 3000, -IW, IW);
  network.addLayer(NNet::sigmoidFunc, 2, -IW, IW);

  NNet::Layer* outputLayer = network.backLayer(0);

  // normally we would train the network before executing it, for the
  // sake of demonstration, this step is omitted. See examples/nnet_train.

  // test the performance by executing it on the C++ side
  double t1 = NSys::now();
  inputLayer->neuron(0)->input(0.25);
  inputLayer->neuron(1)->input(0.70);
  double dt = NSys::now() - t1;
  cout << "c++ time is: " << dt << endl;

  cout << "output[0]= " << outputLayer->neuron(0)->output() << endl;
  cout << "output[1]= " << outputLayer->neuron(1)->output() << endl;

  // an NNModule holds compiled neural networks
  NNModule module;
  t1 = NSys::now();

  // compile the neural network to run with 8 threads, giving it the
  // name "test"
  module.compile("test", network, 8);
  dt = NSys::now() - t1;
  cout << "compile time is: " << dt << endl;

  // the input and output layers must be a NPLVector of double's and
  // of the length which matches the C++ side network as constructed
  // above
  typedef NPLVector<double, 2> Vec2;

  Vec2 inputs;
  inputs[0] = 0.25;
  inputs[1] = 0.70;

  Vec2 outputs;

  t1 = NSys::now();
  // execute the network
  module.run("test", &inputs, &outputs);
  dt = NSys::now() - t1;
  cout << "run time is: " << dt << endl;

  // verify that these outputs match our first outputs
  cout << "outputs: " << outputs << endl;

  // delete the resources used by the compiled neural network named "test"
  module.remove("test");

  return 0;
}
