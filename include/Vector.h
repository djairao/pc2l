#ifndef VECTOR_H
#define VECTOR_H

//---------------------------------------------------------------------
//  ____ 
// |  _ \    This file is part of  PC2L:  A Parallel & Cloud Computing 
// | |_) |   Library <http://www.pc2lab.cec.miamioh.edu/pc2l>. PC2L is 
// |  __/    free software: you can  redistribute it and/or  modify it
// |_|       under the terms of the GNU  General Public License  (GPL)
//           as published  by  the   Free  Software Foundation, either
//           version 3 (GPL v3), or  (at your option) a later version.
//    
//   ____    PC2L  is distributed in the hope that it will  be useful,
//  / ___|   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// | |       WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// | |___    PURPOSE.
//  \____| 
//            Miami University and  the PC2Lab development team make no
//            representations  or  warranties  about the suitability of
//  ____      the software,  either  express  or implied, including but
// |___ \     not limited to the implied warranties of merchantability,
//   __) |    fitness  for a  particular  purpose, or non-infringement.
//  / __/     Miami  University and  its affiliates shall not be liable
// |_____|    for any damages  suffered by the  licensee as a result of
//            using, modifying,  or distributing  this software  or its
//            derivatives.
//
//  _         By using or  copying  this  Software,  Licensee  agree to
// | |        abide  by the intellectual  property laws,  and all other
// | |        applicable  laws of  the U.S.,  and the terms of the  GNU
// | |___     General  Public  License  (version 3).  You  should  have
// |_____|    received a  copy of the  GNU General Public License along
//            with MUSE.  If not,  you may  download  copies  of GPL V3
//            from <http://www.gnu.org/licenses/>.
//
// --------------------------------------------------------------------
// Authors:   JD Rudie               rudiejd@miamioh.edu
//---------------------------------------------------------------------
/**
 * @file Vector.h
 * @brief Definition of Vector
 * @author JD Rudie
 * @version 0.1
 * @date 2021-08-30
 * 
 */

#include <unordered_map>
#include "Worker.h"
#include "CacheManager.h"
#include "System.h"
#include "MPIHelper.h"
#include "Message.h"


// namespace pc2l {
BEGIN_NAMESPACE(pc2l);


/**
 * A distributed vector that runs across multiple machines
 * utilizing message passing through MPI. This initial  
 * implementation does not include any caching.
 */
template <typename T>
class Vector {
public:
    /**
     * The default constructor.  Currently, the constructor calls the
     * workhorse
     */
    Vector() : blockSize(System::get().getBlockSize()), siz(0), dsTag(System::get().dsCount++){ }

    /**
     *  Construct a vector by specifying the block size. Currently the
     *  workhorse constructor
     * @param bSize size in bytes of a block
     */
    explicit Vector(unsigned long long bSize) : blockSize(bSize), siz(0), dsTag(System::get().dsCount++) { }
    /**
     * The destructor.
     */
    virtual ~Vector() = default;

    int dsTag;

    // the size (in bytes) of each block in vector. Potentially offer heterogeneous
    // block sizes on different data structures later, but for now it is uniform
    unsigned long long blockSize;

    // The number of elements currently in the vector
    unsigned long long siz;

    /**
     * Returns size (in values, not blocks) of vector
     * @return size (in values) of vector
     */
    unsigned long long size() {
        return siz;
    }

    /**
     * Erase all values from vector
     */
    void clear() {
        for (unsigned long long i = 0; i < siz; i++) {
            erase(i);
        }
    }

    /**
     * Erase the value at \p index
     * @param index the index of the value to be erased
     */
    void erase(unsigned long long index) {
        // move all of the blocks to the right of the index left by one
        for (unsigned long long i = index; i < size() - 1; i++) { swap(i, i + 1); }
        // clear the last index which is now junk
//        size_t blockTag = index * sizeof(T) / blockSize;
//        MessagePtr msg = cm.getBlock(CacheWorker::getKey(dsTag, blockTag));
//        if (msg == nullptr) {
//            // if block on remote cw, we have to fetch it
//            const unsigned long long worldSize = System::get().worldSize();
//            const int storedRank = (blockTag % (worldSize - 1)) + 1;
//            MessagePtr m = Message::create(0, Message::GET_BLOCK, 0);
//            m->dsTag = dsTag;
//            m->blockTag = size() - 1;
//            cm.send(m, destRank);
//            msg = cm.recv(storedRank);
//
//        }
        // I think I actually don't need to clear the last block
        // if it's outside of size it's never going to be used
        // double check with Dr. Rao

        // size() can now be decremented
        siz--;
        //TODO: maybe some check to see if it is successfully deleted?
    }

    /**
     * Returns value at \p index. Note that this only returns a value, not a reference
     * (for now)
     * @param index
     * @return The value at \p index
     */
    T at(unsigned long long index) const {
        CacheManager& cm  = System::get().cacheManager();
        MessagePtr msg;
        size_t blockTag = index*sizeof(T)  / blockSize;
        // if the CacheManager's cache contains this block, just get it
        msg = cm.getBlock(CacheWorker::getKey(dsTag, blockTag));
        if (msg == nullptr) {
            // otherwise, we have to get it from a remote cacheworker
            const unsigned long long worldSize = System::get().worldSize();
            const int storedRank = (blockTag % (worldSize - 1)) + 1;
            msg = Message::create(0, Message::GET_BLOCK, 0);
            msg->dsTag = dsTag;
            msg->blockTag = blockTag;
            cm.send(msg, storedRank);
            msg = cm.recv(storedRank);
            // then put the object at retrieved index into cache
            cm.storeCacheBlock(msg);
        }
        // get array of concatenated T-serializations
        char* payload = msg->getPayload();
        // offset into this array and extract correct portion
        unsigned long long inBlockIdx = ((index * sizeof(T)) % blockSize);
        char serializedObj[sizeof(T)];
        // copy the block we need into a character array then reinterpret and deref it
        std::copy(&payload[inBlockIdx], &payload[inBlockIdx + sizeof(T)], &serializedObj[0]);
        auto ret = reinterpret_cast<T*>(serializedObj);
        return *ret;
    }

    /**
     * Insert \p value at vector index \p index.
     * @param index index where insert should occur
     * @param value value to be inserted
     */
    void insert(unsigned long long index, T value) {
        CacheManager& cm = System::get().cacheManager();
        // always insert into the cache manager's local cache - only move to cache worker on eviction
        size_t blockTag = index * sizeof(T) / blockSize;
        MessagePtr m = cm.getBlock(CacheWorker::getKey(dsTag, blockTag));
        if (m == nullptr) {
            // otherwise construct message and fill the buffer with data to insert
            m = Message::create(blockSize, Message::STORE_BLOCK, 0);
            m->dsTag = dsTag;
            m->blockTag = blockTag;
        }
        char* block = m->getPayload();
        // if we're not inserting at end of list
        if (index < size()) {
            // last value moves up one index
            insert(size(), at(size()-1));
            // all other values shifted right one index
            for (auto i = index; i < size(); i++) {
                replace(i, at(i-1));
            }
            // now we can insert
        }
        // offset into the block array of serializations and insert val
        unsigned long long inBlockIdx = ((index * sizeof(T)) % blockSize);
        char* serialized = reinterpret_cast<char*>(&value);
        std::copy(&serialized[0], &serialized[sizeof(T)], &block[inBlockIdx]);
        // then put the object at retrieved index into cache
        cm.storeCacheBlock(m);

        siz++;
    }

    /**
     * Replace value at \p index with \p value
     * @param index
     * @param value
     */
    void replace(unsigned long long index, T value) {
        // obtain world size() and compute destination rank for replacement
        CacheManager& cm = System::get().cacheManager();
        auto& mgrCache = cm.managerCache();
        size_t blockTag = index * sizeof(T) / blockSize;
        MessagePtr m = cm.getBlock(CacheWorker::getKey(dsTag, blockTag));
        if (m == nullptr) {
            // if the block with this element is not in CM cache, get from remote CW
            const int rank = (index % (System::get().worldSize() - 1)) + 1;
            m = Message::create(0, Message::GET_BLOCK, 0);
            m->dsTag = dsTag;
            m->blockTag = blockTag;
            cm.send(m, rank);
            m = cm.recv(rank);
        }
        char* block = m->getPayload();
        // fill the buffer with new datum at correct in-blok offset
        unsigned long long inBlockIdx = ((index * sizeof(T)) % blockSize);
        char* serialized = reinterpret_cast<char*>(&value);
        std::copy(&serialized[0], &serialized[sizeof(T)], &block[inBlockIdx]);
        cm.storeCacheBlock(m);
    }

    /**
     * Swap value at index \p i with value at index \j
     * @param i first index of swap
     * @param j second index of swap
     */
    void swap(size_t i, size_t j) {
        auto oldI = at(i);
        replace(i, at(j));
        replace(j, oldI);
    }
};

END_NAMESPACE(pc2l);
// }   // end namespace pc2l

#endif

