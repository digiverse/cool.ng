/*
 * Copyright (c) 2017 Leon Mlakar.
 * Copyright (c) 2017 Digiverse d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. The
 * license should be included in the source distribution of the Software;
 * if not, you may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * The above copyright notice and licensing terms shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "cool/ng/exception.h"
#include "run_queue.h"

namespace cool { namespace ng { namespace async { namespace impl {

namespace {

dispatch_queue_t& get_global_queue()
{
  static dispatch_queue_t global_ = ::dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

  return global_;
}

} // anonymous namespace

run_queue::pointer run_queue::create(const std::string& name_)
{
  return std::make_shared<run_queue>(name_);
}

void run_queue::release(const pointer& q_)
{
  /* noop */
}

run_queue::run_queue(const std::string& name_)
    : named(name_)
    , m_active(true)
{
  m_queue = dispatch_queue_create_with_target(name().c_str(), NULL, get_global_queue());
}

run_queue::~run_queue()
{
  start();
  dispatch_release(m_queue);
}

void run_queue::enqueue(executor exe_, deleter del_, void* data_)
{
  ::dispatch_async_f(m_queue, data_, exe_);
}

void run_queue::start()
{
  bool expect = false;
  if (m_active.compare_exchange_strong(expect, true))
  {
    dispatch_resume(m_queue);
  }
}

void run_queue::stop()
{
  bool expect = true;
  if (m_active.compare_exchange_strong(expect, false))
  {
    dispatch_suspend(m_queue);
  }
}

} } } } // namespace
