//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"
#include "ANGLETest.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    int rt = RUN_ALL_TESTS();
    return rt;
}
