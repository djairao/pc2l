#ifndef CACHE_MANAGER_CPP
#define CACHE_MANAGER_CPP

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
// Authors:   Dhananjai M. Rao          raodm@miamioh.edu
//---------------------------------------------------------------------
#include <thread>
#include "CacheManager.h"
#include "Exception.h"

// namespace pc2l {
BEGIN_NAMESPACE(pc2l);

void
CacheManager::finalize() {
    const auto workers = MPI_GET_SIZE();
    auto finMsg = Message::create(0, Message::FINISH);
    // Send finish message to all of the worker-processes
    for (int rank = 1; (rank < workers); rank++) {
        send(finMsg, rank);
    }
}

MessagePtr CacheManager::getBlock(size_t key) {
    MessagePtr ret = nullptr;
    if (cache.find(key) != cache.end()) {
        ret = cache[key];
        refer(cache[key]);
    }
    return ret;
}


void CacheManager::run() {
    // bgWorker = std::thread(CacheManager::runBackgroundWorker);
}

END_NAMESPACE(pc2l);
// }   // end namespace pc2l

#endif
