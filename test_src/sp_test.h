/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Copyright (C) 2011,2012 Alexey Veretennikov (alexey dot veretennikov at gmail.com)
 
  This file is part of libspmatrix.

  libspmatrix is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  libspmatrix is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with libspmatrix.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SP_TEST_H_
#define _SP_TEST_H_

#define SP_ADD_TEST(name) sp_add_test(name,#name);
#define SP_ADD_SUITE_TEST(suite,name) sp_add_suite_test(suite,name,#name);

#define ASSERT_TRUE(condition) {if (!(condition))           \
      sp_assertion_failed(__FILE__,__LINE__, #condition);}
#define ASSERT_FALSE(condition) {if ((condition))           \
      sp_assertion_failed(__FILE__,__LINE__, #condition);}

#define EXPECT_TRUE(condition) {if (!(condition))             \
      sp_expectation_failed(__FILE__,__LINE__, #condition);}
#define EXPECT_FALSE(condition) {if ((condition))             \
      sp_expectation_failed(__FILE__,__LINE__, #condition);}

typedef struct sp_test_suite sp_test_suite;

typedef void (*test_func_t)();

/*
 * In command line arguments can be specified particular test
 * to run. No arguments (argc = 1) leads to execution of all tests
 */
void sp_run_tests(int argc, const char* argv[]);


void sp_add_test(test_func_t func, const char* name);
sp_test_suite* sp_add_suite(const char* name,
                            void(*test_suite_init)(),
                            void(*test_suite_fini)());
void sp_add_suite_test(sp_test_suite* suite,
                       test_func_t func,
                       const char* name);

void sp_assertion_failed(const char* file, int line, const char* condition);
void sp_expectation_failed(const char* file, int line, const char* condition);

#endif /* _SP_TEST_H_ */
