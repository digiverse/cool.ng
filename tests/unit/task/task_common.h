
#if !defined(cool_ng_f27b4cb0_dba1_4ce1_b25a_913f5951523a)
#define      cool_ng_f27b4cb0_dba1_4ce1_b25a_913f5951523a

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


using ms = std::chrono::milliseconds;

class my_runner : public cool::ng::async::runner
{
 public:
  void inc()   { ++counter; }
  void clear() { counter = 0;}
  int counter = 0;
};

bool spin_wait(unsigned int msec, const std::function<bool()>& lambda);


struct counted_moveable : public std::vector<int>
{
 public:
  counted_moveable() : vector(), m_instance(++instance_counter)
  {
    ++cnt_def_ctor;
  }
  counted_moveable(int a_, int b_) : vector({ a_, b_ }),  m_instance(++instance_counter)
  {
    ++cnt_el_ctor;
  }
  counted_moveable(const counted_moveable& other) : vector(other),   m_instance(++instance_counter)
  {
    ++cnt_copy_ctor;
  }
  counted_moveable(counted_moveable&& other) : vector(std::move(other)), m_instance(++instance_counter)
  {
    ++cnt_move_ctor;
  }
  ~counted_moveable()
  {
    ++cnt_dtor;
  }
  static void clear();

  static int instance_counter;
  static int cnt_move_ctor;
  static int cnt_copy_ctor;
  static int cnt_def_ctor;
  static int cnt_el_ctor;
  static int cnt_dtor;
  int m_instance;
};

struct counted_copyable : public std::vector<int>
{
 public:
  counted_copyable() : vector(), m_instance(++instance_counter)
  {
    ++cnt_def_ctor;
  }
  counted_copyable(int a_, int b_) : vector({ a_, b_ }),  m_instance(++instance_counter)
  {
    ++cnt_el_ctor;
  }
  counted_copyable(const counted_copyable& other) : vector(other), m_instance(++instance_counter)
  {
    ++cnt_copy_ctor;
  }
//  counted_moveable(counted_moveable&& other) : vector(std::move(other)), m_instance(++instance_counter)
//  {
//    ++cnt_move_ctor;
//  }
  ~counted_copyable()
  {
    ++cnt_dtor;
  }
  static void clear();

  static int instance_counter;
  static int cnt_copy_ctor;
  static int cnt_def_ctor;
  static int cnt_el_ctor;
  static int cnt_dtor;
  int m_instance;
};

struct counted_moveonly : public std::vector<int>
{
 public:
  counted_moveonly() : vector(), m_instance(++instance_counter)
  {
    ++cnt_def_ctor;
  }
  counted_moveonly(int a_, int b_) : vector({ a_, b_ }),  m_instance(++instance_counter)
  {
    ++cnt_el_ctor;
  }
  counted_moveonly(counted_moveonly&& other) : vector(std::move(other)), m_instance(++instance_counter)
  {
    ++cnt_move_ctor;
  }
  ~counted_moveonly()
  {
    ++cnt_dtor;
  }
  counted_moveonly(const counted_moveonly&) = delete;

  static void clear();

  static int instance_counter;
  static int cnt_move_ctor;
  static int cnt_def_ctor;
  static int cnt_el_ctor;
  static int cnt_dtor;
  int m_instance;
};


#endif
