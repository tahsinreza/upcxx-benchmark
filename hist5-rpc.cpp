//SNIPPET
#include <iostream>
#include <random>
#include <vector>
#include <unistd.h>
#include <upcxx/upcxx.hpp>
#include "hist4-rpc-ff.hpp"
#include "timers.hpp"

// uncomment to disable assert()
// #define NDEBUG
#include <cassert>
 
// we will assume this is always used in all examples
using namespace std;

static uint64_t ranks_per_node = 12; //40; // TODO: read from SLURM args

static uint64_t log_global_table_size = 28;
static uint64_t local_table_size = 0;
static uint64_t global_table_size = 0;
static uint64_t local_update_count = 250000;
// 2000000 when ranks_per_node = 6 
// 1000000 when ranks_per_node = 12
// 250000 / 10,000,000 when ranks_per_node = 40 
// 200000 / 11,200,000 when ranks_per_node = 56

int main(int argc, char *argv[]) {
  // setup UPC++ runtime
  upcxx::init();

  // host info	  
  size_t _HOST_NAME_MAX = 256;
  char hostname[_HOST_NAME_MAX];
  gethostname(hostname, _HOST_NAME_MAX);

  if (upcxx::rank_me() == 0) {
    std::cout << "Hostname: " << hostname << " process# " << upcxx::rank_me()
      << " out of " << upcxx::rank_n() << " processes" << std::endl;
  }

  // app

  global_table_size = ((uint64_t)1) << log_global_table_size; // e.g., 2^30
  local_table_size = (global_table_size / upcxx::rank_n()) + 1; 
  
  auto per_node_update_count = local_update_count * ranks_per_node;

  auto global_send_count = local_update_count * upcxx::rank_n();

  if (upcxx::rank_me() == 0) {
    std::cout << upcxx::rank_n() << " " << log_global_table_size << " " << 
      local_table_size << " " << global_table_size << " " << 
      local_update_count << " " << per_node_update_count << std::endl;
  }

  DistrMap dmap(local_table_size);
  //std::cout << dmap.local_table_size() << std::endl;

  assert(dmap.local_table_size() == local_table_size);

  std::vector<uint64_t> random_global_table_indices(local_update_count, 0); 

  std::mt19937 rnd_gen(upcxx::rank_me());
  std::uniform_int_distribution<uint64_t> 
    global_table_index_dist(0, global_table_size - 1);

  for (uint64_t i = 0; i < local_update_count; i++) {
    uint64_t random_global_table_index = global_table_index_dist(rnd_gen);
    random_global_table_indices[i] = random_global_table_index;

    const int target_rank = (int)(random_global_table_index % upcxx::rank_n()); 
    if (target_rank >= upcxx::rank_n()) {
      std::cerr << "Error: Invalid rank " << target_rank << std::endl; 
      //return -1; // TODO     	
    }   
  } // for

  upcxx::barrier();

  // beginning of distributed processing

  auto start_time = timer_start();                                                                                                         
 
  for (uint64_t i = 0; i < random_global_table_indices.size(); i++) {

    const int target_rank = 
      (int)(random_global_table_indices[i] % upcxx::rank_n());

    if (target_rank != upcxx::rank_me()) {
      // remote update
      dmap.remote_update(random_global_table_indices[i], target_rank); 
    } else {
      // local update
      dmap.local_update(random_global_table_indices[i]);
    }

    // periodically call progress to allow incoming RPCs to be processed
    if (i % 10 == 0) {  
      upcxx::progress();
    }

  } // for

  bool done = false;

  do {
    upcxx::progress();
    auto global_receive_count = upcxx::reduce_all(
      DistrMap::get_local_recv_count(), upcxx::op_fast_add).wait();
    done = (global_send_count == global_receive_count); 
  } while (!done);

  size_t max_tmp_count = upcxx::reduce_all(tmp_count, upcxx::op_fast_max).wait();

  upcxx::barrier(); 

  auto elapsed_time = timer_elapsed(start_time);                                                                                           

  // end of distributed processing

  if (upcxx::rank_me() == 0) {
    std::cout << "Time: " << elapsed_time << " s" << std::endl;
  }

  auto updates_per_second = 
    (double)(local_update_count * upcxx::rank_n()) / elapsed_time;

  if (upcxx::rank_me() == 0) {
    std::cout << "Updates per second: " << updates_per_second << std::endl;
  }

  // sanity checks 

  auto global_receive_count = upcxx::reduce_all(
    DistrMap::get_local_recv_count(), upcxx::op_fast_add).wait();
  if (upcxx::rank_me() == 0) {                                                                                                               
    std::cout << "#Msgs sent: "<< global_send_count << ", received: " << 
      global_receive_count << std::endl; 
  }
  assert(global_send_count == global_receive_count);

  //if (upcxx::rank_me() == 0) {
  //  dmap.print_local_table();
  //}

  // app

  upcxx::finalize();

  return 0;
} 
//SNIPPET
