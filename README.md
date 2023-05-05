# Simple Optimistic State Machine Replication Codebase

This is the codebase with the code for simple optimistic SMR. In it, we have the following components.
- Byzantine Protocol Simulator
- Proof-of-concept implementation for Simple Optimistic SMR

We will talk the usage and codebase of each of these projects below. The details of the implementations of these projects are in our report.

## Byzantine Protocol Simulator

### Usage
The Byzantine Protocol simulator can either be ran by running `bft-sim`, the C++ executable that directly takes the simulator, or it can be ran by running the Python script `./scripts/run_benchmark.py`, which runs `bft-sim` under the hood and measures the time that `bft-sim` takes to execute on different modes.

To build `bft-sim`, we run the following commands:
```bash
mkdir build
cd build
cmake ..
```
After this, we should have an executable called `bft-sim` located in the `build` directory. We can pass the following arguments to `bft-sim`:
- `-f`: the number of faulty nodes; the total number of nodes is then taken as 3f+1
- `-t`: the path of the trace file that contains the file commands. The path to this trace file should be a valid path from the directory you run `bft-sim` in.
- `-b`: An integer argument that is passed to `bft-sim` that customizes the execution of the service you choose to run. In the PBFT simulator, this argument represents the different kinds of Byzantine nodes the service runs with (0 is no bad nodes, 1 is `f` non-responsive nodes, 2 is `f` inconsistent/incorrect but responsive nodes).
- `-m`: The mode of service that the user chooses to run, where 0 is unreplicated service, 1 is PBFT, 2 is our protocol (BBAlgo).

An example of a proper command, ran from the `$PATH_TO_SOURCE_CODE/build` directory, would be:
```
./bft-sim -f 5 -t ../traces/trace1000.txt -b 0 -r true -m 0
```
This command runs ./bft-sim as an unreplicated service (given by `-m 0`), with 5 faulty nodes (for a total number of 16 nodes), on the trace file `traces1000.txt`, with the integer argument 1 (which does nothing in the unreplicated service, but alas).

To run the Python script `./scripts/run_benchmark.py`, we pass the following arguments:
- `-t`: The trace file that contains the file command. This script assumes that the trace file is in the `$PATH_TO_SOURCE_CODE/traces` directory. If it's not then the script will error out.
- `-c`: A correctness flag that indicates whether the user wants to compare results with the unreplicated service (which is very simple in implementation).
- `-f`: the number of faulty nodes; the total number of nodes is then taken as 3f+1
- `-b`: The integer argument `-b` that is forwarded to `bft-sim`.
- `-m`: The mode of service that the user chooses to run. `-m pbft` will run both the unreplicated service and the PBFT service, `-m bbalgo` will run both the unreplicated service and the BBALgo service, and `-m nrep` will run the unreplicated service only.

An example of a proper command, ran from the `$PATH_TO_SOURCE_CODE` directory, would be:
```
python3 ./scripts/run_benchmark.py -t trace1000.txt -c -f 5 -b 0 -m pbft
```
This command runs the benchmarking script on the trace file `traces1000.txt`, with correctness checking enabled, and with 5 faulty nodes (for a total number of 16 nodes). It will run `bft-sim` on both the unreplicated service and the PBFT service, forwarding the integer argument 0 to both services. The benchmarking script will then time the execution of the services (and test for correctness, depending on the correctness flag) and output those results. 

We also include a script called `scripts/generate_trace.py`, which takes in two arguments:
- `-t`: a name for the trace file generated. Trace files are placed in the ./traces/ directory.
- `-n`: the total number of commands in the trace file.
This script generates a trace file with the name specified by the user, with the number of commands specified by the user, with randomized set/get commands.
An example of proper command, ran from the `$PATH_TO_SOURCE_CODE` directory, would be:
```
python3 ./scripts/generate_trace.py -n 25 -t trace25.txt
```
This command would generate a trace file called `trace25.txt` and place it in the `$PATH_TO_SOURCE_CODE/traces` directory. This trace file would have 25 randomized set/get commands.

### Other Important Things to Know

In the simulator, the service updates one value via set and get commands. The commands in the trace files given to the service are either `g` (get and return the value) or `s 1` (set the value to 1, where any integer value can replace `1`). The return value from the service should be `SET VAL` for a set request to `VAL`, where `VAL` is an integer. For a get request, the return value should be just the value stored. The default value stored initially is 0.

## Proof-of-concept Implementation for Simple Optimistic SMR

The simple optimistic SMR implementation is a Python script that can be ran by running `python3 main.py` from the `$PATH_TO_SOURCE_CODE/scripts` directory. One can toggle the arguments by modifying the 

## Codebase

The Simple Optimistic State Machine Replication codebase is laid out in the following way:
```
├── CMakeLists.txt
├── README.md
├── scripts
│   ├── bb.py
│   ├── client.py
│   ├── generate_trace.py
│   ├── main.py
│   ├── osmr.py
│   └── run_benchmark.py
├── src
│   ├── basic_good_node.cpp
│   ├── basic_good_service.cpp
│   ├── include
│   │   ├── basic_good_node.h
│   │   ├── basic_good_service.h
│   │   ├── lib.h
│   │   ├── node.h
│   │   ├── non_replicated_service.hpp
│   │   ├── pbft_good_node.h
│   │   ├── pbft_node.h
│   │   ├── pbft_not_responsive_node.h
│   │   ├── pbft_service.h
│   │   ├── pbft_wrong_node.h
│   │   └── service.h
│   ├── lib.cpp
│   ├── node.cpp
│   ├── pbft_good_node.cpp
│   ├── pbft_node.cpp
│   ├── pbft_not_responsive_node.cpp
│   ├── pbft_service.cpp
│   ├── pbft_wrong_node.cpp
│   ├── service.cpp
│   └── simulator.cpp
└── traces
    ├── trace1.txt
    ├── trace100.txt
    └── trace1000.txt
```
We organize our discussion of the codebase by file/directory.

### CMakeLists.txt
This is the CMake configuration file for the Byzantine simulator. 

### README.md
This is the README file (the one you're reading right now) for the codebase.

### scripts
This directory contains two types of scripts. First, it contains the scripts used to implement the proof of concept implementation of our algorithm's state machine replication protocol. It also contains the scripts used for trace generation and benchmarking for the simulator.

### src
This directory contains the source code for the simulator. A few things to note are:
- The main method of the simulator is in `simulator.cpp`. This method parses the arguments and runs the command processing directly on the service.
- The virtual classes for nodes and services are in `node.*` and `service.*` respectively.
- `lib.cpp` contains a method for `sha256`, which is used in the PBFT service.
- The implementation of the PBFT service is in the files `pbft_*.h` and `pbft_*.cpp`. The implementation of good (non-Byzantine) nodes is located in `pbft_good_node.*`, and the implementations of non-responsive and incorrect nodes are in `pbft_not_responsive_node.*` and `pbft_wrong_node.*` respectively. The PBFT service implementation is in `pbft_service.*`.
- The implementation of the non-replicated service is in `non_replicated_service.hpp`.
- The implementation of the BBAlgo service is in TODO: FILL IN
- `basic_good_node.*` and `basic_good_service.*` are examples to the team to show how to use virtual classes to implement nodes/services. They are not used by the simulator codebase.

### traces
This directory contains the trace files used by the simulator. It contains three tracefiles:
- `trace1.txt`: The name for this is kind of deceptive. It contains two commands, a `s 2`, then a `g`. This is used for initial testing.
- `trace100.txt`: This file contains 100 commands, and is randomly generated. It is used for stress testing.
- `trace1000.txt`: This file contains 1000 commands, and is randomly generated. It is used mainly for benchmarking.