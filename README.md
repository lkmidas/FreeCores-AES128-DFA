# DFA attack on FreeCores simple AES-128 implementation

PoC for a Differential Fault Analysis attack on a simple AES-128 hardware implementation in [tiny_aes](https://github.com/freecores/tiny_aes) by **FreeCores**.

## About DFA
Differential Fault Analysis (DFA) is side-channel techniques developed to break hardware cryptographic implementations. Basically it consists of the following steps:
1. Using hardware power analysis on vulnearable hardware implementation to precisely identify the execution of each steps in the AES algorithm.
2. Precisely inject faults to the appropriate signals at appropriate times to collect useful faulty outputs.
3. From the collected outputs, follow the DFA attack algorithm to recover the secret cryptographic key.

As for what signals to inject, when to inject and how to recover the secret key from faulty outputs, they are covered in details in [this blog post](https://blog.quarkslab.com/differential-fault-analysis-on-white-box-aes-implementations.html) by **QuarksLab**.

## About the PoC
The process of building this PoC:
1. Static analysis: since I don't use hardware to do power analysis, I have to rely entirely on static analyzing the source code. By reading the source code of `aes_128`, I realized that this code is a naive implementation of AES and doesn't have any countermeasure to side-channel attack (such as inserting noisy calculations to mess with the power consumption, inserting self error checks, etc.). Therefore, I can conclude that this code is vulnearable to side-channel attack.
2. Dynamic analysis: to inject faults to the signals, I do it in a simulation environment with **Verilator**. I can simulate the fault injection in precise locations based on the clock cycles when each round of AES is processed.
3. DFA: After collecting faulty outputs from Verilator, I can feed them into python DFA tools such as `phoenixAES` and `aeskeyschedule` to recover the secret key.

### Requirements
- [Verilator](https://verilator.org/guide/latest/install.html)
- [phoenixAES](https://pypi.org/project/phoenixAES/)
- [aeskeyschedule](https://pypi.org/project/aeskeyschedule/)

### Run
```
make
python3 pwn.py
```

