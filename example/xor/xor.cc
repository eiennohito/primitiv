// This example shows a small usage of the primitiv library with solving the XOR
// problem by 2-layer (input/hidden/output) perceptron.
//
// Compile:
// g++
//   -std=c++11
//   -I/path/to/primitiv/includes (typically -I../..)
//   -L/path/to/primitiv/libs     (typically -L../../build/primitiv)
//   xor.cc -lprimitiv

#include <iostream>

// "primitiv.h" can be used to include most of features in primitiv.
// Users also can include individual headers to suppress compilation cost.
#include <primitiv/primitiv.h>

// "primitiv_cuda.h" is required to use CUDA backend.
//#include <primitiv/primitiv_cuda.h>

// Shortcuts
using namespace std;
using namespace primitiv;
namespace F = primitiv::operators;
namespace I = primitiv::initializers;
namespace T = primitiv::trainers;

int main() {
  // Setups a computation backend.
  // The device object manages device-specific memories, and must be destroyed
  // after all other objects were gone.
  CPUDevice dev;
  Device::set_default_device(dev);

  // If you want to use CUDA, uncomment below line (and comment out above) with
  // a specific device (GPU) ID.
  //CUDADevice dev(0);

  // Parameters
  Parameter pw1("w1", {8, 2}, I::XavierUniform());
  Parameter pb1("b1", {8}, I::Constant(0));
  Parameter pw2("w2", {1, 8}, I::XavierUniform());
  Parameter pb2("b2", {}, I::Constant(0));

  // Trainer
  T::SGD trainer(0.1);
  trainer.add_parameter(pw1);
  trainer.add_parameter(pb1);
  trainer.add_parameter(pw2);
  trainer.add_parameter(pb2);

  // Fixed input data
  const vector<float> input_data {
     1,  1,  // Sample 1
     1, -1,  // Sample 2
    -1,  1,  // Sample 3
    -1, -1,  // Sample 4
  };

  // Corresponding output data
  const vector<float> output_data {
     1,  // Label 1
    -1,  // Label 2
    -1,  // Label 3
     1,  // Label 4
  };

  // Training loop
  for (unsigned i = 0; i < 10; ++i) {
    Graph g;
    Graph::set_default_graph(g);

    // Builds a computation graph.
    const Node x = F::input<Node>(Shape({2}, 4), input_data);
    const Node w1 = F::parameter<Node>(pw1);
    const Node b1 = F::parameter<Node>(pb1);
    const Node w2 = F::parameter<Node>(pw2);
    const Node b2 = F::parameter<Node>(pb2);
    const Node h = F::tanh(F::matmul(w1, x) + b1);
    const Node y = F::matmul(w2, h) + b2;

    // Calculates values.
    const vector<float> y_val = g.forward(y).to_vector();
    cout << "epoch " << i << ":" << endl;
    for (unsigned j = 0; j < 4; ++j) {
      cout << "  [" << j << "]: " << y_val[j] << endl;
    }

    // Builds an additional computation graph for the mean squared loss.
    const Node t = F::input<Node>(Shape({}, 4), output_data);
    const Node diff = t - y;
    const Node loss = F::batch::mean(diff * diff);

    // Calculates losses.
    // The forward() function performs over only additional paths.
    const float loss_val = g.forward(loss).to_vector()[0];
    cout << "  loss: " << loss_val << endl;

    // Resets cumulative gradients of all registered parameters.
    trainer.reset_gradients();

    // Backpropagation
    g.backward(loss);

    // Updates parameters.
    trainer.update();
  }

  return 0;
}
