#if !defined(cool_ng_abcdefb0_dba1_4ce1_b25a_913f5951523a)
#define      cool_ng_abcdefb0_dba1_4ce1_b25a_913f5951523a

#include <functional>
#include <chrono>
#include <thread>
#include <vector>
#include "cool/ng/async.h"

#include <boost/test/unit_test.hpp>

#if !defined(COOL_AUTO_TEST)
#  if BOOST_VERSION < 106200
#    define COOL_AUTO_TEST_CASE(a, b) BOOST_AUTO_TEST_CASE(a)
#  else
#    define COOL_AUTO_TEST_CASE(a, b) BOOST_AUTO_TEST_CASE(a, b)
     namespace utf = boost::unit_test;
#  endif
#endif

#endif
