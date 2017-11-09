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

#if !defined(cool_ng_6c2c783d_18a0_4a18_abf5_908c1f127780)
#define      cool_ng_6c2c783d_18a0_4a18_abf5_908c1f127780

#include "impl/platform.h"

namespace cool { namespace ng {

/**
 * Elements supporting concurrent asynchronous programming.
 *
 * <b>Task Queues</b>
 *  - @ref cool::ng::async::runner "runner" class is a task queue abstraction
 *
 * <b>Tasks</b>
 *  - @ref cool::ng::async::task "task" class template represents a unit of scheduling
 *    into the task queue
 *  - @ref cool::ng::async::factory "factory" is a factory class providing methods
 *    for creating simple tasks and combining them into compound tasks
 *
 * <b>Event Sources</b>
 *  - Data input/output event sources:
 *    - @ref cool::ng::async::reader "reader" provides data input stream of events
 */
namespace async {


} } }

#include "async/runner.h"
#include "async/task.h"
//#include "async/event_sources.h"

#endif
