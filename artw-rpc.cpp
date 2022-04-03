//SNIPPET
#include <iostream>
#include <unistd.h>
#include <upcxx/upcxx.hpp>
#include "timers.hpp"

// we will assume this is always used in all examples
using namespace std;

static uint64_t num_trips = 10000;
static uint64_t trip_count = 0;

void visit_next(void) {
  const int next = (upcxx::rank_me() + 1) % upcxx::rank_n();
  if (trip_count < num_trips) {
    upcxx::rpc_ff(next, visit_next);
    trip_count++;
  } 
} 

int main(int argc, char *argv[])
{
  // setup UPC++ runtime
  upcxx::init();

  // app

  auto start_time = timer_start();	

  const int nranks = upcxx::rank_n();
  const int me = upcxx::rank_me();
  const int next = (me + 1) % nranks;
  //const int prev = (me + nranks - 1) % nranks;

  if (upcxx::rank_me() == 0) { 
    upcxx::rpc_ff(next, visit_next); 
    trip_count++; 
  }

  while(trip_count < num_trips) {
    upcxx::progress();
  }
 
  upcxx::barrier();
 
  auto elapsed_time = timer_elapsed(start_time);	

  if (upcxx::rank_me() == 0) {
    std::cout << "#Trips: " << trip_count << std::endl;
  }
 
  if (upcxx::rank_me() == 0) {
    std::cout << "Time: " << elapsed_time << " s" << std::endl;
  }

  auto rate = (double)(num_trips * upcxx::rank_n()) / elapsed_time;
  if (upcxx::rank_me() == 0) {
    std::cout << "Rate: " << rate << " hops/s" << std::endl;
  }
  
  // app 

  upcxx::finalize();
  return 0;
} 
//SNIPPET
