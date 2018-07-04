#ifndef __LAVA_TESTS_H__
#define __LAVA_TESTS_H__

#define BOOST_TEST_MODULE mb

#define BOOST_CHECK_TRUE(A) BOOST_CHECK_EQUAL(A, true)
#define BOOST_CHECK_FALSE(A) BOOST_CHECK_EQUAL(A, false)
#define BOOST_CHECK_IS_NULL(A) BOOST_CHECK(!A);

#include <boost/test/unit_test.hpp>

#endif /* __LAVA_TESTS_H__ */
