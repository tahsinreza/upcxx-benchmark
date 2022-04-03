# upcxx-benchmark
UPC++ benchmark

<h3>Setup details</h3> 

Distributed compiler platform:
https://hpc.llnl.gov/hardware/compute-platforms/catalyst
<br>

Compiler: 
gcc-8.3.1
<br>
(Note: We also tried building using gcc-9.3.1; however, it was unsuccessful.)  

Configuration:
<br>
mkdir /usr/workspace/reza2/upcxx/release4
<br>
cd /usr/workspace/reza2/upcxx/release4
<br>
../configure --prefix=/usr/workspace/reza2/upcxx/release4/ --disable-mpi-compat --with-ibv-spawner=pmi --with-ibv-max-hcas=2
<br>

Build log: https://raw.githubusercontent.com/tahsinreza/upcxx-benchmark/main/catalyst-build-log
<br>

Compile application:
<br>
cd /usr/workspace/reza2/upcxx/release4/
<br>
./bin/upcxx -std=c++17 -O3 -network=ibv -threadmode=seq ../example/prog-guide/hello-world.cpp
<br>
./bin/upcxx -std=c++17 -O3 -network=ibv -threadmode=seq ../example/prog-guide/artw-rpc.cpp
<br>
./bin/upcxx -std=c++17 -O3 -network=ibv -threadmode=seq ../example/prog-guide/hist3-rpc.cpp
<br>

Running application:
cd /usr/workspace/reza2/upcxx/release4/
srun -N16 --ntasks-per-node=12 -t4:00:00 --distribution=block a.out
srun -N64 --ntasks-per-node=6 -t4:00:00 --distribution=block a.out
<br>

<h3>Benchmark details</h3>

Around the world (artw-rpc.cpp):

Assuming there are 10 ranks, first, rank 0 sends a message to rank 1. Upon hearing from rank 0, rank 1 sends a message to rank 2 and so forth. A single trip completes when rank 0 has received a message from rank 9. To complete n trips, each rank must send/receive n messages to/from their immediate neighbor (rank k receives from rank k -1 and sends to rank k+1).  
<br>

Histogram (hist3-rpc.cpp)

(This is a modified version of the histogram exmaple in the <a href="https://bitbucket.org/berkeleylab/upcxx/src/master/example/prog-guide/">prog-guide</a>). n (integer) keys are evenly distributed among r ranks. Each rank stores n/r keys in its local storage - a key is mapped to an index of the local storage of a rank; each index stores an integer initialized to 0. Each rank randomly selects q (out of n) keys: for each of these q keys, a rank first decodes which rank owns the key and sends a message to the rank containing the key. When a rank receives a message containing a key, it first decodes the index (of the local storage) the key is mapped to and increases the value at the index by one. The distributed task ends when all q\*r events have been processed. 

<br>
