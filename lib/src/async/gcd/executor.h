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

#if !defined(cool_ng_d2aa9442_15ec_4748_9d69_a7d096d1b861)
#define      cool_ng_d2aa9442_15ec_4748_9d69_a7d096d1b861

#include <atomic>
#include <memory>
#include <dispatch/dispatch.h>
#include "cool/ng/bases.h"
#include "cool/ng/async/runner.h"
#include "cool/ng/impl/async/context.h"

namespace cool { namespace ng { namespace async { namespace impl {

class executor : public bases::named
{
 public:
  executor(RunPolicy policy_);
  ~executor();

  void start();
  void stop();
  void run(detail::context_stack*);

 private:
  static void task_executor(void*);

 private:
  const bool        m_is_system;
  std::atomic<bool> m_active;
  dispatch_queue_t  m_queue;
};

} } } }// namespace

#endif

