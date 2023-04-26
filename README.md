## Usage

Arguments to simulator:
```
-f: number of faulty nodes
-t tracefile with client commands
-b number representing what kind of faulty behaviors of Byzantine nodes
-r reliability (true or false, do good nodes die accidentally)
```

To build:
```bash
mkdir build
cd build
cmake .. 
make
# runs program
./bft-sim
```
## Design

main program:
create nodes, parse arguments
run commands in tracefile

The service is updating one value and keeping track of commands. 

Default value is 0. 

Commands are either `g` (get and return the value) or `s 1` (set the value to 1, for any number).
Return value is `SET 1` for a set request to 1 (and change number as needed), and just the value for a return. 
