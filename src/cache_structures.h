/*
 * cache_structures.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#ifndef CACHE_STRUCTURES_H_
#define CACHE_STRUCTURES_H_

#include <assert.h>
#include <vector>

#include "primitive_types.h"

//#include "component_types/difference_packed_component.h"
#include "component_types/simple_packed_component.h"


using namespace std;

#define NIL_ENTRY 0

class Component;
class ComponentArchetype;


// GenericCachedComponent Adds Structure to PackedComponent that is
// necessary to store it in the cache
// namely, the descendant tree structure that
// allows for the removal of cache pollutions

template< class T_Component>
class GenericCachedComponent: public T_Component {
public:
  GenericCachedComponent() {
  }

  GenericCachedComponent(Component &comp, unsigned component_stack_id) :
      T_Component(comp), component_stack_id_(component_stack_id) {
  }

  // a cache entry is deletable
  // only if it is not connected to an active
  // component in the component stack
  bool deletable() {
    return component_stack_id_ == 0;
  }
  void eraseComponentStackID() {
    component_stack_id_ = 0;
  }
  void setComponentStackID(unsigned id) {
    component_stack_id_ = id;
  }
  unsigned component_stack_id() {
    return component_stack_id_;
  }

  void clear() {
    // before deleting the contents of this component,
    // we should make sure that this component is not present in the component stack anymore!
    assert(component_stack_id_ == 0);
    if (T_Component::data_)
      delete T_Component::data_;
    T_Component::data_ = nullptr;
  }


  unsigned long SizeInBytes() const {
    return sizeof(GenericCachedComponent<T_Component>)
        + T_Component::data_size() * sizeof(unsigned)
        + T_Component::size_of_model_count();
  }

  // BEGIN Cache Pollution Management

  void set_father(CacheEntryID f) {
    father_ = f;
  }
  const CacheEntryID father() const {
    return father_;
  }

  void set_next_sibling(CacheEntryID sibling) {
    next_sibling_ = sibling;
  }
  CacheEntryID next_sibling() {
    return next_sibling_;
  }

  void set_first_descendant(CacheEntryID descendant) {
    first_descendant_ = descendant;
  }
  CacheEntryID first_descendant() {
    return first_descendant_;
  }

private:
  // the position where this
  // component is stored in the component stack
  // if this is non-zero, we may not simply delete this
  // component
  unsigned component_stack_id_ = 0;

  // theFather and theDescendants:
  // each CCacheEntry is a Node in a tree which represents the relationship
  // of the components stored
  CacheEntryID father_ = 0;
  CacheEntryID first_descendant_ = 0;
  CacheEntryID next_sibling_ = 0;

};


//typedef GenericCachedComponent<DifferencePackedComponent> CachedComponent;
typedef GenericCachedComponent<SimplePackedComponent> CachedComponent;

class CacheBucket: protected vector<CacheEntryID> {
  friend class ComponentCache;

public:

  using vector<CacheEntryID>::size;

  unsigned long getBytesMemoryUsage() {
    return sizeof(CacheBucket) + size() * sizeof(CacheEntryID);
  }
};

#endif /* CACHE_STRUCTURES */