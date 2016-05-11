/*
 * ========================================================
 *
 *       Filename:  base.test.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/08/2013 06:37:38 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#include "gtest/gtest.h"
#include "base.h"

TEST(chkDir,POSITIVE)
{
	bool ret=chkDir("/proc");
	EXPECT_EQ(ret,true);
	ret=chkDir("/no-exist-dir");
	EXPECT_EQ(ret,false);
}

TEST(createDir,POSITIVE)
{
	char dir[]="/tmp/no-exist-dir";
	bool ret=createDir(dir);
	EXPECT_EQ(ret,true);

	char dir1[]="/tmp";
	ret=createDir(dir1);
	EXPECT_EQ(ret,true);
	
	char dir2[]="/dev/null";
	ret=createDir(dir2);
	EXPECT_EQ(ret,false);
}
