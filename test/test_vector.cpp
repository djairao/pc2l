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
// Authors:   JD Rudie                             rudiejd@miamioh.edu
//---------------------------------------------------------------------

#include <iostream>
#include "Environment.h"


class VectorTest : public ::testing::Test {

};

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) std::cout << argv[i] << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    auto env = new PC2LEnvironment();
    env->argc = argc;
    env->argv = argv;
    ::testing::AddGlobalTestEnvironment(env);
    return RUN_ALL_TESTS();
}
TEST_F(VectorTest, test_insert_int) {
    // Test inserting 100 integers
    pc2l::Vector<int> intVec;
    for (int i = 0; i < 100; i++) {
        ASSERT_NO_THROW(intVec.insert(i, i));
    }
    // intVec.clear();
}

TEST_F(VectorTest, test_at) {
    pc2l::Vector<int> intVec;
    for (int i = 0; i < 100; i++) {
        ASSERT_NO_THROW(intVec.insert(i, i));
    }
    // check to see if data is correct (tests deserializtion)
    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(i, intVec.at(i));
    }

}


TEST_F(VectorTest, test_lru_caching) {
    auto& pc2l = pc2l::System::get();
    pc2l::Vector<int> intVec;
    // this produces 20 blocks of integers
    for (int i = 0; i < 100; i++) {
        ASSERT_NO_THROW(intVec.insert(i, i));
    }
    // cache should contain 3 blocks 85-90, 90-95, 95-100
    ASSERT_EQ(pc2l.cacheManager().managerCache().size(), 3);
    // cache should now be the last 3 blocks inserted
    ASSERT_TRUE(pc2l.cacheManager().getBlock(pc2l.cacheManager().getKey(intVec.dsTag, 17)) != nullptr);
    ASSERT_TRUE(pc2l.cacheManager().getBlock(pc2l.cacheManager().getKey(intVec.dsTag, 18)) != nullptr);
    ASSERT_TRUE(pc2l.cacheManager().getBlock(pc2l.cacheManager().getKey(intVec.dsTag, 19)) != nullptr);
    // front of cache is the last block, rear of cache is the third-last (LRU)
    // now retrieve another block
    intVec.at(0);
    // cache should be size 3
    ASSERT_EQ(pc2l.cacheManager().managerCache().size(), 3);
    // the 17th (third to last) block should be removed from cache (contains 85-90)
    ASSERT_EQ(pc2l.cacheManager().managerCache().find(pc2l.cacheManager().getKey(intVec.dsTag, 17)), pc2l.cacheManager().managerCache().end());
    // the 0-5 block should be in the cache
    ASSERT_TRUE(pc2l.cacheManager().managerCache().find(pc2l.cacheManager().getKey(intVec.dsTag, 0)) != pc2l.cacheManager().managerCache().end());
}

TEST_F(VectorTest, test_delete) {
    auto& pc2l = pc2l::System::get();
    pc2l::Vector<int> intVec;
    // this produces 20 blocks of integers
    for (size_t i = 0; i < 100; i++) {
        ASSERT_NO_THROW(intVec.insert(i, i));
    }
    // delete at index 42
    ASSERT_NO_THROW(intVec.erase(42));
    // size should be 99
    ASSERT_EQ(intVec.size(), 99);
    for (size_t i = 0; i < 42; i++) {
        // up to modified index it should be the same
        ASSERT_EQ(intVec.at(i), i);
    }
    for (size_t i = 42; i < intVec.size(); i++) {
        // every other value should be one bigger than it was
        ASSERT_EQ(intVec.at(i), i+1);
    }
}

/*TEST_F(VectorTest, test_caching) {
    // Test caching on the vector
    for (int i = 0; i < 100; i++) {
        ASSERT_NO_THROW(intVec.insert(i, i));
    }
}*/
