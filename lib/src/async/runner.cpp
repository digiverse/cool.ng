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

#if defined(WINDOWS_TARGET)
# include <Windows.h>
#endif

#include "cool/ng/async/runner.h"
#include "cool/ng/exception.h"
#include "cool/ng/impl/async/context.h"
#include "cool/ng/impl/async/task.h"
#include "run_queue.h"

namespace cool { namespace ng { namespace async {

runner::runner()
{
  m_impl = impl::run_queue::create();
}

runner::~runner()
{
  impl::run_queue::release(m_impl);
}

const std::string& runner::name() const
{
  return m_impl->name();
}

const std::shared_ptr<impl::run_queue>& runner::impl() const
{
  return m_impl;
}

namespace detail {

// executor for task::run()
void task_executor(void* arg_)
{
  auto ctx = static_cast<detail::context_stack*>(arg_);

  auto r = ctx->top()->get_runner().lock();
  if (r)
  {
    ctx->top()->entry_point(r, ctx->top());
    if (ctx->empty())
      delete ctx;
    else
      r->impl()->enqueue(task_executor, nullptr, ctx);
  }
  else
    delete ctx;
}

void kickstart(context_stack* ctx_)
{
  if (!ctx_)
    throw exception::no_context();

  auto aux = ctx_->top()->get_runner().lock();
  if (!aux)
  {
    delete ctx_;
    throw exception::runner_not_available();
  }
  aux->impl()->enqueue(task_executor, nullptr, ctx_);
}

}
} } } // namespace
