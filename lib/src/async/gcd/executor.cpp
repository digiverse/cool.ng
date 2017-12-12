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
#include "executor.h"

namespace cool { namespace ng { namespace async { namespace impl {

executor::executor(RunPolicy policy_)
    : named("si.digiverse.ng.cool.runner")
    , m_is_system(false)
    , m_active(true)
{
#if defined(OSX_TARGET)
  if (policy_ == RunPolicy::CONCURRENT)
    m_queue = ::dispatch_queue_create(name().c_str(), DISPATCH_QUEUE_CONCURRENT);
  else
#endif
    m_queue = ::dispatch_queue_create(name().c_str(), NULL);
}

executor::~executor()
{
  if (!m_is_system)
    dispatch_release(m_queue);
}

void executor::run(detail::context_stack* ctx_)
{
  ::dispatch_async_f(m_queue, ctx_, task_executor);
}

// executor for task::run()
void executor::task_executor(void* arg_)
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
  

} } } } // namespace
