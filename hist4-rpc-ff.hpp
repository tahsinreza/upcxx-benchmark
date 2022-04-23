#pragma once

#include <vector>
#include <upcxx/upcxx.hpp>

static uint64_t local_recv_count = 0;

class DistrMap {

private :

  // store the local map in a distributed object to access from RPCs
  using dobj_map_t = upcxx::dist_object<std::vector<uint64_t> >;
  dobj_map_t local_map; // pointer

public :

  // initialize the local map
  DistrMap(uint64_t local_container_size) : local_map({}) {
    local_map->resize(local_container_size, 0);
  }

  // map the key to a target rank
  int get_target_rank(const uint64_t &key) {
    return (int)(key % upcxx::rank_n());
  }

  void remote_update(const uint64_t &key, const uint64_t &target_rank) { 
    upcxx::rpc_ff(target_rank, //get_target_rank(key),
      // lambda 
      [](dobj_map_t &lmap, const uint64_t &key) {
        uint64_t local_table_index = key / upcxx::rank_n();
        //assert(local_table_index < lmap->size()); 
        lmap->operator[](local_table_index)++;
        local_recv_count++;    
      }, 
      local_map, key);
  }

  void local_update(const uint64_t &key) {
    uint64_t local_table_index = key / upcxx::rank_n();
    //assert(local_table_index < local_map->size());
    local_map->operator[](local_table_index)++; 
    local_recv_count++;
  }

  uint64_t local_table_size() {
    return local_map->size();
  }

  void print_local_table() {
    std::cout << std::endl;
    for (uint64_t i = 0; i < local_map->size(); i++) {
      if (local_map->operator[](i) > 0) {
        std::cout << local_map->operator[](i) << " ";
      }
    }
    std::cout << std::endl;
  }

  static uint64_t get_local_recv_count() {
    return local_recv_count;  
  }

};

