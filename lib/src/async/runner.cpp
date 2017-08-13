/* Copyright (c) 2017 Digiverse d.o.o.
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
#include "cool/ng/impl/async/context.h"
#include "cool/ng/impl/async/task.h"
#include "lib/async/executor.h"

namespace cool { namespace ng { namespace async {

runner::runner(RunPolicy policy_)
{
  m_impl = std::make_shared<impl::executor>(policy_);
}

runner::~runner()
{ /* noop */ }

const std::string& runner::name() const
{
  return m_impl->name();
}

const std::shared_ptr<impl::executor>& runner::impl() const
{
  return m_impl;
}

void runner::start()
{
  m_impl->start();
}

void runner::stop()
{
  m_impl->stop();
}

namespace detail {

void kickstart(context_stack* ctx_)
{
  if (!ctx_)
    throw exception::no_context();

  auto aux = ctx_->top()->get_runner().lock();
  if (!aux)
    throw exception::runner_not_available();

  aux->impl()->run(ctx_);
}

}
} } } // namespace
