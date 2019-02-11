

#define BOOST_TEST_MODULE Task
#include "task_common.h"

int counted_moveable::instance_counter = 0;
int counted_moveable::cnt_move_ctor = 0;
int counted_moveable::cnt_copy_ctor = 0;
int counted_moveable::cnt_def_ctor = 0;
int counted_moveable::cnt_el_ctor = 0;
int counted_moveable::cnt_dtor = 0;

int counted_copyable::instance_counter = 0;
int counted_copyable::cnt_copy_ctor = 0;
int counted_copyable::cnt_def_ctor = 0;
int counted_copyable::cnt_el_ctor = 0;
int counted_copyable::cnt_dtor = 0;


int counted_moveonly::instance_counter = 0;
int counted_moveonly::cnt_move_ctor = 0;
int counted_moveonly::cnt_def_ctor = 0;
int counted_moveonly::cnt_el_ctor = 0;
int counted_moveonly::cnt_dtor = 0;

void counted_moveable::clear()
{
  instance_counter = 0;
  cnt_move_ctor = 0;
  cnt_copy_ctor = 0;
  cnt_def_ctor = 0;
  cnt_el_ctor = 0;
  cnt_dtor = 0;
}

void counted_copyable::clear()
{
  instance_counter = 0;
  cnt_copy_ctor = 0;
  cnt_def_ctor = 0;
  cnt_el_ctor = 0;
  cnt_dtor = 0;
}

void counted_moveonly::clear()
{
  instance_counter = 0;
  cnt_move_ctor = 0;
  cnt_def_ctor = 0;
  cnt_el_ctor = 0;
  cnt_dtor = 0;
}

bool spin_wait(unsigned int msec, const std::function<bool()>& lambda)
{
  auto start = std::chrono::system_clock::now();
  while (!lambda())
  {
    auto now = std::chrono::system_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= msec)
      return false;
  }
  return true;
}
