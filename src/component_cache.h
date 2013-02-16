/*
 * component_cache.h
 *
 *  Created on: Feb 5, 2013
 *      Author: mthurley
 */

#ifndef COMPONENT_CACHE_H_
#define COMPONENT_CACHE_H_


#include "cache_structures.h"
#include "statistics.h"

#include <gmpxx.h>

#include "component_types/component.h"

#include "stack.h"


class ComponentCache {
public:

  ComponentCache(DataAndStatistics &statistics);

  ~ComponentCache() {
    for (auto &pbucket : table_)
      if (pbucket != nullptr)
        delete pbucket;
    for (auto &pentry : entry_base_)
      if (pentry != nullptr)
        delete pentry;
  }

  void init(Component &super_comp);

  // compute the size in bytes of the component cache from scratch
  // the value is stored in bytes_memory_usage_
  uint64_t recompute_bytes_memory_usage();

  CachedComponent &entry(CacheEntryID id) {
    assert(entry_base_.size() > id);
    assert(entry_base_[id] != nullptr);
    return *entry_base_[id];
  }

  CachedComponent &entry(const Component& comp) {
      return entry(comp.id());
  }

  bool hasEntry(CacheEntryID id) {
    assert(entry_base_.size() > id);
    return entry_base_[id];
  }

  // removes the entry id from the hash table
  // but not from the entry base
  inline void removeFromHashTable(CacheEntryID id);

  // we delete the Component with ID id
  // and all its descendants from the cache
  inline void cleanPollutionsInvolving(CacheEntryID id);

  // creates a CCacheEntry in the entry base
  // which contains a packed copy of comp
  // returns the id of the entry created
  // stores in the entry the position of
  // comp which is a part of the component stack
  inline CacheEntryID storeAsEntry(CachedComponent &ccomp,
                            CacheEntryID super_comp_id);

  // check quickly if the model count of the component is cached
  // if so, incorporate it into the model count of top
  // if not, store the packed version of it in the entry_base of the cache
  bool manageNewComponent(StackLevel &top, CachedComponent &packed_comp) {
      statistics_.num_cache_look_ups_++;
      CacheBucket *p_bucket = bucketOf(packed_comp);
      if (p_bucket != nullptr)
        for (auto it = p_bucket->begin(); it != p_bucket->end(); it++)
          if (entry(*it).equals(packed_comp)) {
            statistics_.incorporate_cache_hit(packed_comp);
            top.includeSolution(entry(*it).model_count());
            return true;
          }
      // otherwise, set up everything for a component to be explored
      return false;
    }


  // unchecked erase of an entry from entry_base_
  void eraseEntry(CacheEntryID id) {
    statistics_.incorporate_cache_erase(*entry_base_[id]);
    delete entry_base_[id];
    entry_base_[id] = nullptr;
    free_entry_base_slots_.push_back(id);
  }


  // store the number in model_count as the model count of CacheEntryID id
  inline void storeValueOf(CacheEntryID id, const mpz_class &model_count);

  bool deleteEntries();

  // delete entries, keeping the descendants tree consistent
  inline void removeFromDescendantsTree(CacheEntryID id);

  // test function to ensure consistency of the descendant tree
  inline void test_descendantstree_consistency();

private:

  CacheBucket *bucketOf(const CachedComponent &packed_comp) {
      return table_[packed_comp.hashkey() % table_.size()];
  }

  void add_descendant(CacheEntryID compid, CacheEntryID descendantid) {
      assert(descendantid != entry(compid).first_descendant());
      entry(descendantid).set_next_sibling(entry(compid).first_descendant());
      entry(compid).set_first_descendant(descendantid);
    }

  void remove_firstdescendantOf(CacheEntryID compid) {
      CacheEntryID desc = entry(compid).first_descendant();
      if (desc != 0)
        entry(compid).set_first_descendant(entry(desc).next_sibling());
    }

  vector<CachedComponent *> entry_base_;
  vector<CacheEntryID> free_entry_base_slots_;

  // the actual hash table
  // by means of which the cache is accessed
  vector<CacheBucket *> table_;

  DataAndStatistics &statistics_;

  // unsigned long num_buckets_ = 0;
  unsigned long num_occupied_buckets_ = 0;
  unsigned long my_time_ = 0;
};


#include "component_cache-inl.h"


#endif /* COMPONENT_CACHE_H_ */