# upcxx-benchmark
UPC++ benchmark

<h3>Setup details</h3> 
<br>

Distributed compiler platform:
https://hpc.llnl.gov/hardware/compute-platforms/catalyst
<br>

Compiler: 
gcc-8.3.1
(Note: We also tried building using gcc-9.3.1; however, it was unsuccessful.)  

Configuration:
mkdir /usr/workspace/reza2/upcxx/release4
cd /usr/workspace/reza2/upcxx/release4
../configure --prefix=/usr/workspace/reza2/upcxx/release4/ --disable-mpi-compat --with-ibv-spawner=pmi --with-ibv-max-hcas=2
<br>

Build log: 
https://raw.githubusercontent.com/tahsinreza/upcxx-benchmark/main/catalyst\_build\_log
<br>

Compile application:
cd /usr/workspace/reza2/upcxx/release4/
./bin/upcxx -std=c++17 -O3 -network=ibv -threadmode=seq ../example/prog-guide/hello-world.cpp
./bin/upcxx -std=c++17 -O3 -network=ibv -threadmode=seq ../example/prog-guide/artw-rpc.cpp
./bin/upcxx -std=c++17 -O3 -network=ibv -threadmode=seq ../example/prog-guide/hist3-rpc.cpp
<br>

Running application:
cd /usr/workspace/reza2/upcxx/release4/
srun -N16 --ntasks-per-node=12 -t4:00:00 --distribution=block a.out
srun -N64 --ntasks-per-node=6 -t4:00:00 --distribution=block a.out
<br>

<h3>Benchmark details</h3>

Around the world:

<br>

Histogram:

<br>
