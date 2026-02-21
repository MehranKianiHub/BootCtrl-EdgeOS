BootCtrl‑EdgeOS Introduction
============================

BootCtrl‑EdgeOS is an open‑source machine learning (ML) framework that brings AI capabilities to industrial automation. It builds on the IEC 61499 standard for distributed control and leverages the Eclipse 4diac FORTE runtime to provide a platform‑agnostic runtime for ML inference, training orchestration and model management. The project’s vision is to enable edge‑deployed AI across application domains such as refrigeration systems, robotic arms and smart grids. By the end of this document you should be able to understand industrial automation, program in modern C++, design IEC 61499 function blocks, and implement and deploy ML function blocks similar to those provided in BootCtrl‑EdgeOS.

### Document outline

1.  Background: industrial automation and the IEC 61499 standard.
2.  Eclipse 4diac framework and runtime (FORTE/FBE).
3.  BootCtrl‑EdgeOS overview: features, architecture and repository layout.
4.  Setting up the development environment.
5.  C++ fundamentals.
6.  BootCtrl modules and architecture.
7.  Developing a machine‑learning function block.
8.  Training and deploying models.
9.  Edge optimization and deployment.
10.  Example use cases.
11.  Best practices and contributing guidelines.

Each section builds from first principles. Beginners should work through the C++ fundamentals and IEC 61499 concepts before diving into ML function block development.

1 Industrial automation and IEC 61499
-------------------------------------

### 1.1 Why distributed control?

Traditional programmable logic controllers (PLCs) defined by IEC 61131‑3 follow a centralized execution model. In modern “smart factory” applications, sensors, actuators and controllers are distributed across networks; a single centralized PLC becomes a bottleneck. IEC 61499 addresses this by defining a domain‑specific modelling language for distributed industrial control [eclipse.dev](https://eclipse.dev/4diac/doc/intro/iec61499.html#:~:text=section%20where%20we%20discuss%20a,newer%20standard). The standard improves encapsulation of software components, provides a vendor‑independent format and simplifies controller‑to‑controller communication [eclipse.dev](https://eclipse.dev/4diac/doc/intro/iec61499.html#:~:text=section%20where%20we%20discuss%20a,newer%20standard). Applications are composed of interconnected **Function Blocks (FBs)** whose inputs and outputs are separated into events and data [eclipse.dev](https://eclipse.dev/4diac/doc/intro/iec61499.html#:~:text=In%20other%20words%2C%20IEC%2061499,mapped%20to%20their%20respective%20devices). When an input event triggers a block, its internal algorithm runs, updates its data outputs and emits output events [eclipse.dev](https://eclipse.dev/4diac/doc/intro/iec61499.html#:~:text=1,at%20the%20FB). IEC 61499 allows applications to be distributed over multiple devices and specifies how FBs are mapped to devices and resources [eclipse.dev](https://eclipse.dev/4diac/doc/intro/iec61499.html#:~:text=In%20other%20words%2C%20IEC%2061499,mapped%20to%20their%20respective%20devices).

### 1.2 Kinds of function blocks

The standard defines several types of FBs. **Basic Function Blocks (BFBs)** implement algorithms (written in C/C++ or languages defined by IEC 61131‑3) and include an _Execution Control Chart (ECC)_ that selects which algorithm to run based on events. **Composite Function Blocks (CFBs)** connect multiple FBs into a reusable component. **Service Interface Function Blocks (SIFBs)** provide access to hardware or external services; only their interfaces are specified and their internal behaviour must be implemented manually in C/C++ [eclipse.dev](https://eclipse.dev/4diac/doc/intro/4diacframework.html#:~:text=,from%20Hilscher%2C%20vxWorks%2C%20and%20freeRTOS). BootCtrl function blocks are implemented as SIFBs because they wrap libraries such as TensorFlow Lite Micro or ONNX Runtime.

2 Eclipse 4diac framework
-------------------------

Eclipse 4diac is an open‑source infrastructure for distributed industrial control based on IEC 61499. Its two main components are:

*   **4diac FORTE** – a small portable C++ implementation of an IEC 61499 run‑time environment [eclipse.dev](https://eclipse.dev/4diac/doc/intro/4diacframework.html#:~:text=,from%20Hilscher%2C%20vxWorks%2C%20and%20freeRTOS). FORTE executes FB networks on small embedded devices and is multi‑threaded with low memory consumption [eclipse.dev](https://eclipse.dev/4diac/doc/intro/4diacframework.html#:~:text=,from%20Hilscher%2C%20vxWorks%2C%20and%20freeRTOS). It runs on various operating systems such as Windows, Linux, FreeRTOS, VxWorks and more [eclipse.dev](https://eclipse.dev/4diac/doc/intro/4diacframework.html#:~:text=,from%20Hilscher%2C%20vxWorks%2C%20and%20freeRTOS). Developers compile FB implementations into FORTE to run them on target hardware.
*   **4diac IDE** – an integrated development environment (IDE) that allows users to graphically design FB networks, configure devices and export generated C code for FORTE [eclipse.dev](https://eclipse.dev/4diac/doc/intro/4diacframework.html#:~:text=,time%20environments).

4diac FORTE decouples the modelling of FBs from their runtime; you create FB types in the IDE, export them, add them to the FORTE source tree and recompile. BootCtrl‑EdgeOS extends FORTE with ML‑specific FBs and modules.

3 BootCtrl‑EdgeOS overview
--------------------------

### 3.1 Vision and scope

BootCtrl‑EdgeOS aims to provide a platform‑agnostic ML framework for IEC 61499 distributed control systems. Built on top of 4diac FORTE, it enables **edge‑deployed ML inference, training orchestration and model management**. The goal is to bring AI to industrial automation while remaining open source and production‑ready. Regardless of whether you are controlling refrigeration systems, robots or smart grids, BootCtrl provides the necessary infrastructure.

### 3.2 Key features

The project defines a roadmap of features organised as function blocks:

*   **Inference engines:** integration of TensorFlow Lite Micro (TFLM) and ONNX Runtime within FBs for on‑device inference.
*   **Preprocessing FBs:** blocks for normalization, filtering and feature extraction.
*   **Edge optimization:** quantization and pruning techniques to deploy models on resource‑constrained devices.
*   **Explainable AI:** SHAP values and feature importance visualization to interpret model decisions.
*   **Training orchestration:** distributed and federated training for continual learning.
*   **Security:** encrypted model storage and integrity verification.

### 3.3 Repository layout

The repository is organised into several directories:

*   **4diacFORTE** – upstream FORTE runtime as a git submodule.
*   **4diacFBE** – upstream function block environment (FBE).
*   **configurations** – sample configurations for building BootCtrl‑EdgeOS.
*   **modules** – ML modules and function blocks.
*   **edgeml** – edge ML utilities and examples.
*   **tests** – unit and integration tests.

### 3.4 Architecture

The architecture comprises multiple layers:

1.  **Application layer** – domain‑specific applications such as predictive maintenance, anomaly detection or control/optimization.
2.  **ML function block library** – BootCtrl FBs providing model pre‑processing, inference (TFLite, ONNX) and post‑processing (thresholding, anomaly detection).
3.  **4diac FORTE runtime** – the IEC 61499‑compliant runtime executing the function blocks.
4.  **Hardware abstraction** – supports ARM Cortex‑M processors, x86\_64, RISC‑V and planned FPGA support.

4 Setting up the development environment
----------------------------------------

BootCtrl‑EdgeOS uses modern C++ and CMake. To build the project you need:

*   4diac FORTE ≥ 2.0.0.
*   CMake ≥ 3.16.
*   A C++17 compiler.
*   (Optional) Python ≥ 3.8 for model conversion tools.

Clone the repository with its submodules:

```sh
git clone --recurse-submodules https://github.com/MehranKianiHub/BootCtrl-EdgeOS.git
cd BootCtrl-EdgeOS
```

If you cloned without `--recurse-submodules`, initialise submodules and update them:

```sh
git submodule update --init --recursive
```

To build the project on a POSIX host use CMake presets:

```sh
cmake --preset posix-debug
cmake --build build/posix-debug
```

BootCtrl uses the same build system as 4diac FORTE; cross‑compilation to embedded targets (e.g., ARM Cortex‑M) requires appropriate toolchains.

#### Submodules and upstream updates

If you need to add the upstream FORTE or FBE modules after cloning, run:

```sh
git submodule add https://github.com/eclipse-4diac/4diac-forte.git 4diacFORTE
git submodule add https://github.com/eclipse-4diac/4diac-fbe.git 4diacFBE
git commit -m "Add submodules"
```

To update upstream runtimes to the latest version:

```sh
git submodule update --remote --merge
```

5 C++ fundamentals
------------------

BootCtrl‑EdgeOS is written in C++17. This section introduces essential C++ concepts.

### 5.1 Classes and objects

A **class** is a blueprint for creating objects [programiz.com](https://www.programiz.com/cpp-programming/object-class#:~:text=A%20class%20is%20a%20blueprint,for%20the%20object). A class definition starts with the `class` keyword followed by the class name and a body enclosed in braces [programiz.com](https://www.programiz.com/cpp-programming/object-class#:~:text=A%20class%20is%20defined%20in,the%20name%20of%20the%20class). Data members store state and member functions define behaviour. For example:

```cpp
class Room {
 public:
    double length;
    double breadth;
    double height;

    double calculate_area() {
        return length * breadth;
    }

    double calculate_volume() {
        return length * breadth * height;
    }
};
```

Objects are instances of a class. No memory is allocated until you create an object [programiz.com](https://www.programiz.com/cpp-programming/object-class#:~:text=C%2B%2B%20Objects). You can create objects in any function by declaring variables of the class type [programiz.com](https://www.programiz.com/cpp-programming/object-class#:~:text=Syntax%20to%20Define%20Object%20in,C) and access members using the dot operator (`.`) [programiz.com](https://www.programiz.com/cpp-programming/object-class#:~:text=C%2B%2B%20Access%20Data%20Members%20and,Member%20Functions). For example:

```cpp
Room room1;
room1.length  = 42.5;
room1.breadth = 30.8;
room1.height  = 19.2;
double area  = room1.calculate_area();
double volume= room1.calculate_volume();
```

### 5.2 Dynamic memory

C++ allows manual memory management. The `new` expression allocates memory at runtime and returns a pointer to the allocated space [programiz.com](https://www.programiz.com/cpp-programming/memory-management#:~:text=C%2B%2B%20new%20Expression). It is recommended to initialise pointers when allocating memory [programiz.com](https://www.programiz.com/cpp-programming/memory-management#:~:text=We%20can%20also%20allocate%20memory,in%20the%20same%20step%20as):

```cpp
int* point_var = new int{45};  // allocates and initialises an int
```

After using dynamically allocated memory, free it with the `delete` operator to avoid memory leaks [programiz.com](https://www.programiz.com/cpp-programming/memory-management#:~:text=delete%20Expression):

```cpp
delete point_var;
point_var = nullptr;  // avoid dangling pointers
```

For arrays, use `new[]` and `delete[]`[programiz.com](https://www.programiz.com/cpp-programming/memory-management#:~:text=Example%202%3A%20C%2B%2B%20new%20and,delete%20Expression%20for%20Arrays).

Dynamic memory is essential for BootCtrl because ML inference libraries (e.g., ONNX Runtime) may allocate large buffers. However, TensorFlow Lite Micro enforces static memory allocation (see Section 6.3).

### 5.3 Other essentials

*   **Functions** – reusable blocks of code with return types and parameters.
*   **Control flow** – `if`, `for`, `while` loops, `switch` statements.
*   **Standard Template Library (STL)** – collections (`std::vector`, `std::map`, etc.), algorithms and iterators.
*   **Namespaces** – avoid name collisions (e.g., `std`).

A solid understanding of these concepts is necessary before writing BootCtrl modules.

6 BootCtrl modules and architecture
-----------------------------------

BootCtrl‑EdgeOS encapsulates ML tasks into modular function blocks. Each module provides a group of FBs. Below are the major categories.

### 6.1 Inference engine modules

**TensorFlow Lite Micro module.** TFLite Micro (TFLM) is a C++‑only inference framework designed for microcontrollers [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=Updated%2031%20January%202026). It uses static memory allocation and a single tensor arena to ensure predictable, low‑latency performance [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=Updated%2031%20January%202026). Post‑training quantization and vendor‑optimised kernels like ARM CMSIS‑NN accelerate inference [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=Updated%2031%20January%202026). Within BootCtrl, TFLM FBs load `.tflite` models (converted from trained TensorFlow models) embedded as C arrays. When an input event arrives, the FB copies input data into the tensor arena, invokes the interpreter, and writes results to its output ports.

**ONNX Runtime module.** ONNX Runtime (ORT) is a high‑performance cross‑platform inference engine that runs models from frameworks such as PyTorch, TensorFlow and Hugging Face on different hardware and software stacks [onnxruntime.ai](https://onnxruntime.ai/inference#:~:text=ONNX%20Runtime%20for%20Inferencing). ORT exploits hardware accelerators and supports APIs in multiple languages (C++, C#, Python, Java, etc.) [onnxruntime.ai](https://onnxruntime.ai/inference#:~:text=ONNX%20Runtime%20for%20Inferencing). BootCtrl’s ORT FBs initialise an inference session with a compiled `.onnx` model and run inference in response to events.

When implementing an inference FB, use C++ RAII to manage session/arena lifetimes. For TFLM, allocate the tensor arena as a static array; for ORT, use smart pointers to release resources automatically. All operations must execute in deterministic time to meet real‑time constraints.

### 6.2 Pre‑processing and post‑processing modules

Pre‑processing FBs perform operations such as normalization, filtering, feature extraction and signal transformation. For example, a **Low‑Pass Filter FB** may implement a moving average filter to remove high‑frequency noise:

```cpp
class LowPassFilterFB : public forte::core::CFB {
  double cutoff_;
  double state_;
  // interface specifications...
  void executeEvent(int pa_nEIID) override {
    if(pa_nEIID == scm_nEventREQID) {
      double input = IN();
      state_ = state_ + (input - state_) * alpha();
      OUT() = state_;
      sendOutputEvent(scm_nEventCNFID);
    }
  }
  double alpha() const {
     double dt = 1.0 / sampleRate();
     double rc = 1.0 / (2 * 3.14159 * cutoff_);
     return dt / (rc + dt);
  }
};
```

Post‑processing FBs can perform thresholding, anomaly detection, or converting model outputs into control signals. Each FB must define its interface specification, internal state and event handler.

### 6.3 Edge optimization modules

Edge devices often have kilobytes to megabytes of RAM. To deploy ML models efficiently, BootCtrl plans to include FBs for model quantization and pruning. Quantization converts floating‑point weights and activations to int8 or int16, reducing flash and RAM requirements without significant accuracy loss [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=3,NN%20Integration). Pruning removes redundant weights to reduce model size. These techniques are typically applied during training or model conversion (Python), and the resulting compact model is then used in inference FBs.

### 6.4 Explainable AI modules

Industrial operators need to trust ML decisions. Planned Explainable AI (XAI) FBs compute feature importance (e.g., SHAP values) and visualise which inputs most influenced a decision. The FB would take as input the model, the input data and the inference result, and output explanations that can be displayed via a human‑machine interface.

### 6.5 Training orchestration

Future BootCtrl versions will support federated learning and distributed training. A training orchestrator would coordinate model updates from multiple devices, aggregate gradients and update the global model. Since on‑device training can be resource intensive, BootCtrl may offload heavy computations to edge servers while preserving data locality.

### 6.6 Security

Security FBs will handle encrypted model storage and integrity verification. In safety‑critical systems, ensuring that the ML models deployed on controllers have not been tampered with is essential. Techniques include storing model hashes and verifying them at startup.

7 Developing a machine‑learning function block
----------------------------------------------

This section guides you through writing a custom ML FB using C++ and 4diac FORTE. We will implement a simple inference block using TFLite Micro. The same principles apply to ONNX Runtime.

### 7.1 Define the FB interface

Each FB has an interface specification describing its event and data ports. In FORTE, this is defined via arrays of `CStringDictionary::TStringId`. For a generic inference FB you might define:

```cpp
class MLPredictionFB : public forte::core::CFunctionBlock {
  DECLARE_FIRMWARE_FB(MLPredictionFB)

  static const TForteInt16 scm_nEventREQID = 0;
  static const TForteInt16 scm_nEventCNFID = 0;

  static const TForteInt16 scm_anEIWithIndexes[];
  static const TDataIOID scm_anEIWith[];
  static const CStringDictionary::TStringId scm_anEventInputNames[];

  static const TForteInt16 scm_anEOWithIndexes[];
  static const TDataIOID scm_anEOWith[];
  static const CStringDictionary::TStringId scm_anEventOutputNames[];

  static const SFBInterfaceSpec scm_stFBInterfaceSpec;

  // data input and output variables
  CIEC_REAL IN1;
  CIEC_REAL OUT1;

 public:
  void executeEvent(TEventID pa_nEIID) override;
};
```

The static arrays map input events to their associated data ports. FORTE generates much of this boilerplate when you export a FB from 4diac IDE, but understanding the underlying C++ structure helps when integrating external libraries.

### 7.2 Implement the event handler

Inside `executeEvent()`, handle the `REQ` event: copy input data into the model’s input tensor, invoke the TFLite interpreter, and write the result to the output. Assume `g_model` and `g_interpreter` are already initialised.

```cpp
void MLPredictionFB::executeEvent(TEventID pa_nEIID) {
  if(pa_nEIID == scm_nEventREQID) {
    // copy input to tensor
    float input = IN1(); 
    memcpy(input_tensor->data.f, &input, sizeof(float));
    // run inference
    TfLiteStatus status = g_interpreter->Invoke();
    if(status != kTfLiteOk) {
      // handle error (e.g., set fault output)
    }
    // read result
    float result = output_tensor->data.f[0];
    OUT1() = result;
    sendOutputEvent(scm_nEventCNFID);
  }
}
```

This code runs in response to an event; therefore it should return quickly. Complex algorithms (e.g., large CNNs) might violate timing constraints on small PLCs, so choose models carefully and consider quantization.

### 7.3 Initialising TFLite Micro

In your FB constructor (or a dedicated initialisation function), allocate the tensor arena and set up the interpreter:

```cpp
constexpr int kTensorArenaSize = 5 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];
const tflite::Model* model = tflite::GetModel(model_data);
// build OpResolver with only required operators
static tflite::MicroMutableOpResolver<5> micro_op_resolver;
micro_op_resolver.AddFullyConnected();
micro_op_resolver.AddRelu();
// ...
static tflite::MicroInterpreter static_interpreter(
    model, micro_op_resolver, tensor_arena, kTensorArenaSize, &error_reporter);
g_interpreter = &static_interpreter;
g_interpreter->AllocateTensors();
input_tensor  = g_interpreter->input(0);
output_tensor = g_interpreter->output(0);
```

Allocate the tensor arena statically because TFLite Micro forbids dynamic memory allocation at runtime [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=Updated%2031%20January%202026). Only include the kernels needed by your model to reduce binary size.

### 7.4 Integrating ONNX Runtime

An ONNX inference FB would create an `Ort::Session` with the embedded `.onnx` model and call `session.Run()` when a `REQ` event arrives. Use `Ort::AllocatorWithDefaultOptions` to allocate inputs and outputs and ensure that you call `session.Release()` in the FB destructor. Because ONNX Runtime uses dynamic memory and threads internally, be mindful of the target’s memory footprint.

8 Training and deploying models
-------------------------------

Training is performed outside the PLC on powerful hardware using Python frameworks. The typical workflow is:

1.  **Collect data** relevant to the industrial process (sensor readings, images, time series).
2.  **Pre‑process data** (e.g., normalization, noise filtering).
3.  **Define and train** a model in TensorFlow or PyTorch.
4.  **Convert** the trained model to TFLite or ONNX format. For TFLite Micro, apply post‑training quantization and ensure all operators are supported.
5.  **Embed the model** as a C array in your FB source (for TFLM) or include the `.onnx` file in your project resources (for ORT).
6.  **Deploy** the updated firmware to the target device.

Edge devices often require over‑the‑air update mechanisms. BootCtrl plans to include model versioning and secure storage to manage deployments.

9 Edge optimization and deployment
----------------------------------

Optimising ML on edge devices involves balancing accuracy, latency and resource consumption. Techniques include:

*   **Static memory planning:** TFLite Micro’s interpreter allocates a single tensor arena and schedules operators to minimise peak memory [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=TFLM%20enforces%20a%20static%20memory,and%20unpredictability%20in%20embedded%20workloads).
*   **Quantization and pruning:** Reduce model size without large accuracy loss [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=3,NN%20Integration).
*   **Operator reordering:** Reorder computation to reduce memory peaks (advanced) [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=4,Reordering).
*   **Hardware acceleration:** Use CMSIS‑NN or device‑specific neural‑network accelerators for speedup [emergentmind.com](https://www.emergentmind.com/topics/tensorflow-lite-micro-tflm#:~:text=Operator%20implementations%20are%20modularized%20into,both%20RAM%20and%20flash%20footprints).
*   **Real‑time constraints:** Ensure inference completes within the control cycle time; consider offloading heavy computations to edge servers.

BootCtrl’s planned edge optimisation modules will automate some of these tasks.

10 Example use cases
--------------------

BootCtrl targets various industrial scenarios:

1.  **Predictive maintenance.** Sensor data are fed into pre‑processing FBs and anomaly‑detection models to predict equipment failures.
2.  **Process optimization.** Process variables are processed and passed through reinforcement‑learning policies to optimise control signals.
3.  **Quality inspection.** Vision data are classified to decide whether products are accepted or rejected.
4.  **Energy management.** Load and weather data are combined to forecast energy demand and adjust HVAC or grid set‑points.

Each use case combines pre‑processing, inference and post‑processing blocks with domain‑specific logic.

11 Best practices and contributing
----------------------------------

### 11.1 Development workflow

Contributions follow a standard workflow:

1.  Fork the repository.
2.  Create a feature branch (`git checkout -b feature/amazing-feature`).
3.  Commit your changes.
4.  Push the branch and open a Pull Request.
5.  Add tests and documentation.

Refer to the Development Guide in `docs/DEVELOPMENT_GUIDE.md` for details.

### 11.2 Platform compatibility

BootCtrl aims to support multiple IEC 61499 runtimes. The primary targets are 4diac FORTE and BootCtrl itself; other runtimes such as ISaGRAF and nxtControl are partially supported. Custom FB runtimes can be supported via an adapter layer.

### 11.3 Community contributions

The project welcomes bug reports, feature suggestions, documentation improvements and translations. See the `CONTRIBUTING` and `DEVELOPMENT_GUIDE` documents for guidelines.

Conclusion
----------

This document introduced industrial automation with IEC 61499, the Eclipse 4diac framework, and the BootCtrl‑EdgeOS project. We covered C++ fundamentals, memory management, and how to develop machine‑learning function blocks using TensorFlow Lite Micro or ONNX Runtime. You learned how to set up the development environment, design function block interfaces, implement inference logic, and consider edge optimisation techniques. BootCtrl’s roadmap includes advanced features like explainable AI, federated learning and security, making it a promising platform for bringing AI to industrial automation. Continue exploring the repository and documentation, experiment with building your own FBs, and contribute to the community to advance this open‑source project.
---

## 📄 License

```
Copyright 2026 BootCtrl Automation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

## 📞 Contact

- 💬 [Discussions](https://github.com/MehranKianiHub/BootCtrl-EdgeOS/discussions)
- 🐛 [Issues](https://github.com/MehranKianiHub/BootCtrl-EdgeOS/issues)
- 📧 [Email](mailto:mehran.kiani@bootctrl.com)
- 🌐 [Website](https://bootctrl.com/)

<p align="center">
  <strong>⭐ Star us on GitHub if you find this project useful! ⭐</strong>
</p>

<p align="center">
  Made with ❤️ by the industrial automation and ML community
</p>
---