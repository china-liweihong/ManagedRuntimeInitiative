/*
 * Copyright 2003 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *  
 */
// This file is a derivative work resulting from (and including) modifications
// made by Azul Systems, Inc.  The date of such changes is 2010.
// Copyright 2010 Azul Systems, Inc.  All Rights Reserved.
//
// Please contact Azul Systems, Inc., 1600 Plymouth Street, Mountain View, 
// CA 94043 USA, or visit www.azulsystems.com if you need additional information 
// or have any questions.
#ifndef HASHTABLE_INLINE_HPP
#define HASHTABLE_INLINE_HPP


// Inline function definitions for hashtable.hpp.


// --------------------------------------------------------------------------
// Hash function

// We originally used hashpjw, but hash P(31) gives just as good results
// and is slighly faster. We would like a hash function that looks at every
// character, since package names have large common prefixes, and also because
// hash_or_fail does error checking while iterating.

// hash P(31) from Kernighan & Ritchie

inline unsigned int Hashtable::hash_symbol(const char* s, int len) {
  unsigned int h = 0;
  while (len-- > 0) {
    h = 31*h + (unsigned) *s;
    s++;
  }
  return h;
}


// --------------------------------------------------------------------------

// Initialize a table.

inline BasicHashtable::BasicHashtable(int table_size, int entry_size) {
  // Called on startup, no locking needed
  initialize(table_size, entry_size, 0);
  _buckets = NEW_C_HEAP_ARRAY(HashtableBucket, table_size);
  for (int index = 0; index < _table_size; index++) {
    _buckets[index].clear();
  }
}


inline BasicHashtable::BasicHashtable(int table_size, int entry_size,
                                      HashtableBucket* buckets,
                                      int number_of_entries) {
  // Called on startup, no locking needed
  initialize(table_size, entry_size, number_of_entries);
  _buckets = buckets;
}


inline void BasicHashtable::initialize(int table_size, int entry_size,
                                       int number_of_entries) {
  // Called on startup, no locking needed
  _table_size = table_size;
  _entry_size = entry_size;
  _free_list = NULL;
  _first_free_entry = NULL;
  _end_block = NULL;
  _number_of_entries = number_of_entries;
#ifdef ASSERT
  _lookup_count = 0;
  _lookup_length = 0;
#endif
}


// The following method is MT-safe and may be used with caution.
inline BasicHashtableEntry*BasicHashtable::bucket(int i)const{
  return _buckets[i].get_entry();
}


inline void HashtableBucket::set_entry(BasicHashtableEntry* l) {
  // Warning: Preserve store ordering.  The SystemDictionary is read
  //          without locks.  The new SystemDictionaryEntry must be
  //          complete before other threads can be allowed to see it
  //          via a store to _buckets[index].
  OrderAccess::release_store_ptr(&_entry, l);
}


inline BasicHashtableEntry* HashtableBucket::get_entry() const {
  // Warning: Preserve load ordering.  The SystemDictionary is read
  //          without locks.  The new SystemDictionaryEntry must be
  //          complete before other threads can be allowed to see it
  //          via a store to _buckets[index].
  return (BasicHashtableEntry*) OrderAccess::load_ptr_acquire(&_entry);
}


inline void BasicHashtable::set_entry(int index, BasicHashtableEntry* entry) {
  _buckets[index].set_entry(entry);
}


inline void BasicHashtable::add_entry(int index, BasicHashtableEntry* entry) {
  // Concurrent GC threads may unlink symbols, so the insert has to be done with a CAS:
BasicHashtableEntry*e=NULL;
  BasicHashtableEntry** head_addr = _buckets[index].entry_addr();

  while (true) {
    e = *head_addr;
entry->set_next(e);
    
    Atomic::write_barrier();

    intptr_t result = Atomic::cmpxchg_ptr((intptr_t) entry, (intptr_t*) head_addr, (intptr_t) e);

    if (result == (intptr_t) e) break;
  }
Atomic::inc_ptr(&_number_of_entries);
}

inline void BasicHashtable::free_entry(BasicHashtableEntry* entry) {
  entry->set_next(_free_list);
  _free_list = entry;
  Atomic::dec_ptr(&_number_of_entries);
}

#endif // HASHTABLE_INLINE_HPP

