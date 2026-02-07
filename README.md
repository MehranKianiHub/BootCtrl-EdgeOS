# BootCtrl-EdgeOS: Machine Learning Framework for IEC 61499

> **Bring AI to Industrial Automation. Platform-agnostic. Open Source. Production-Ready.**

---

## 🎯 Vision

BootCtrl-EdgeOS is a **platform-agnostic machine learning framework** that brings AI capabilities to IEC 61499 distributed control systems. Built on top of 4diac FORTE, it enables edge-deployed ML inference, training orchestration, and model management—completely independent of application domains.

Whether you're controlling refrigeration systems, robotic arms, or smart grids, BootCtrl-EdgeOS provides the ML infrastructure you need.

---

## ✨ Key Features

| Feature | Description | Status |
|---------|-------------|--------|
| **🔮 Inference Engine** | TensorFlow Lite Micro & ONNX Runtime integration | 🚧 In Progress |
| **📊 Preprocessing FBs** | Normalization, filtering, feature extraction | 🚧 In Progress |
| **⚡ Edge Optimization** | Quantization, pruning for resource-constrained devices | 📋 Planned |
| **🔍 Explainable AI** | SHAP values, feature importance visualization | 📋 Planned |
| **📈 Training Orchestration** | Federated learning, distributed training | 📋 Planned |
| **🛡️ Security** | Encrypted model storage, integrity verification | 📋 Planned |

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    Application Layer                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  Predictive │  │  Anomaly    │  │  Optimization/Control   │  │
│  │ Maintenance │  │  Detection  │  │  (RL/MPC)               │  │
│  └──────┬──────┘  └──────┬──────┘  └───────────┬─────────────┘  │
└─────────┼────────────────┼─────────────────────┼────────────────┘
          │                │                     │
          ▼                ▼                     ▼
┌─────────────────────────────────────────────────────────────────┐
│                    ML Function Block Library                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  ML_Preproc │  │  ML_Infer   │  │  ML_Postproc            │  │
│  │  (Normalize)│  │  (TFLite)   │  │  (Threshold/Anomaly)    │  │
│  └──────┬──────┘  └──────┬──────┘  └───────────┬─────────────┘  │
│         │                │                     │                │
│  ┌──────┴────────────────┴─────────────────────┴─────────────┐  │
│  │              ML_ModelManager (Versioning)              │  │
│  └────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────────────────────────────────────┐
│                    4diac FORTE Runtime                           │
│         (IEC 61499-1/2 Compliant Distributed Execution)          │
└─────────────────────────────────────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Hardware Abstraction                          │
│     ARM Cortex-M4/M7/A | x86_64 | RISC-V | FPGA (future)       │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🚀 Quick Start

### Prerequisites

- 4diac FORTE >= 2.0.0
- CMake >= 3.16
- C++17 compatible compiler
- (Optional) Python >= 3.8 for model conversion tools

### Installation

```bash
# Clone the repository
git clone https://github.com/MehranKianiHub/BootCtrl-EdgeOS.git
cd BootCtrl-EdgeOS

# Build with 4diac FORTE
mkdir build && cd build
cmake .. -DFORTE_PATH=/path/to/forte
make -j$(nproc)

# Install function blocks to 4diac IDE
cmake --install . --prefix /path/to/4diac-ide

---

## 🎯 Use Cases

### 1. Predictive Maintenance
```
Sensor Data → ML_Preproc → ML_Inference (Anomaly Detection) 
    → ML_Postproc → Maintenance Alert
```

### 2. Process Optimization
```
Process Variables → ML_Preproc → ML_Inference (RL Policy)
    → Control Signals → Actuators
```

### 3. Quality Inspection
```
Vision Data → ML_Preproc → ML_Inference (Classification)
    → Quality Decision → Reject/Accept
```

### 4. Energy Management
```
Load Data + Weather → ML_Preproc → ML_Inference (Forecasting)
    → Setpoint Adjustment → HVAC/Grid
```

---

## 🤝 Contributing

We welcome contributions from the industrial automation and ML communities!

### Ways to Contribute

- 🐛 **Report Bugs**: Open an issue with reproduction steps
- 💡 **Suggest Features**: Propose new FB types or ML methods
- 📝 **Improve Docs**: Help us make documentation clearer
- 🔧 **Submit Code**: Follow our [Development Guide](docs/DEVELOPMENT_GUIDE.md)
- 🧪 **Add Tests**: Increase test coverage
- 🌍 **Translate**: Help translate documentation

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [DEVELOPMENT_GUIDE.md](docs/DEVELOPMENT_GUIDE.md) for detailed guidelines.


---

## 🔧 Platform Compatibility

| Platform | Status | Notes |
|----------|--------|-------|
| 4diac FORTE | ✅ Full | Primary target |
| bootctrl | ✅ Full | Primary target |
| ISaGRAF | 🚧 Partial | FB import supported |
| nxtControl | 📋 Planned | Community request |
| Custom FB Runtime | ✅ Full | Via adapter layer |

---

## 📚 Documentation

- [📖 Development Roadmap](docs/ROADMAP.md)
- [🏗️ System Architecture](docs/ARCHITECTURE.md)
- [🔧 Development Guide](docs/DEVELOPMENT_GUIDE.md)
- [📋 Function Block Reference](docs/FUNCTION_BLOCKS.md)
- [🧪 Testing Guide](docs/TESTING.md)
- [🚀 Deployment Guide](docs/DEPLOYMENT.md)

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

---

## 🙏 Acknowledgments

- [Eclipse 4diac](https://www.eclipse.org/4diac/) - For the excellent IEC 61499 runtime
- [TensorFlow Lite Micro](https://www.tensorflow.org/lite/microcontrollers) - For edge ML inference
- [ONNX Runtime](https://onnxruntime.ai/) - For cross-platform model execution

---

## 📞 Contact

- 💬 [Discussions](https://github.com/MehranKianiHub/BootCtrl-EdgeOS/discussions)
- 🐛 [Issues](https://github.com/MehranKianiHub/BootCtrl-EdgeOS/issues)
- 📧 [Email](mailto:mehran.kiani@bootctrl.com)
- 🌐 [Website](https://bootctrl.com/)

---

<p align="center">
  <strong>⭐ Star us on GitHub if you find this project useful! ⭐</strong>
</p>

<p align="center">
  Made with ❤️ by the industrial automation and ML community
</p>