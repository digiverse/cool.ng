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

#include "cool/ng/async/runner.h"
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
  auto ret = std::make_shared<run_queue>(name_);
  ret->m_self = ret;
  return ret;
}

void run_queue::release(const pointer& q_)
{
  q_->m_self.reset();
  q_->start();
}

run_queue::run_queue(const std::string& name_)
    : named(name_)
    , m_status(EMPTY_ACTIVE_NOT_BUSY)
{
}

run_queue::~run_queue()
{
  /* noop */
}

void run_queue::enqueue(executor exe_, deleter del_, void* data_)
{
  {
    std::unique_lock<std::mutex> l(m_mutex);
    m_fifo.push_back(context(exe_, del_, data_, m_self));
    m_status &= ~EMPTY;
  }

  check_submit_next();
}

bool run_queue::check_submit_next()
{
  int expect = NOT_EMPTY_ACTIVE_NOT_BUSY;
  if (m_status.compare_exchange_strong(expect, NOT_EMPTY_ACTIVE_BUSY))
  {
    std::unique_lock<std::mutex> l(m_mutex);
    m_running = m_fifo.front();
    m_fifo.pop_front();
    if (m_fifo.empty())
      m_status |= EMPTY;
    ::dispatch_async_f(get_global_queue(), &m_running, run_next);

    return true;
  }

  return false;
}

void run_queue::run_next(void* data_)
{
  auto ctx = static_cast<context*>(data_);
  auto queue = ctx->m_queue;

  // try/catch to intercept all exceptions thrown from the user code
  try { (*(ctx->m_executor))(ctx->m_data); } catch (...) { /* noop */ }
  ctx->clear();

  queue->m_status &= ~BUSY;
  queue->check_submit_next();
}

void run_queue::stop()
{
  m_status &= ~ACTIVE;
}

void run_queue::start()
{
  m_status |= ACTIVE;
  check_submit_next();
}

#if 0
// executor for task::run()
void run_queue::task_executor(void* arg_)
{
  auto ctx = static_cast<detail::context_stack*>(arg_);

  auto r = ctx->top()->get_runner().lock();
  if (r)
  {
    ctx->top()->entry_point(r, ctx->top());
    if (ctx->empty())
      delete ctx;
    else
      r->impl()->run(ctx);
  }
  else
    delete ctx;
}
#endif

} } } } // namespace
