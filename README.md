# nao_mumbler

A Chatbot for SoftBank NAO using NOOS API

**NOTE** This is a project meant to be built using `qibuild`.
For more info on how to use the toolchain, please see: https://github.com/aldebaran/qibuild

# BUILDING

```
cd micprocessor
qibuild configure -c naoqi-sdk -DCMAKE_CXX_FLAGS=-DBOOST_SIGNALS_NO_DEPRECATION_WARNING
qibuild make -c naoqi-sdk
```
