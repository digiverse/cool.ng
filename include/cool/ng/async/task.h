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

#if !defined(cool_ng_f36abcb0_dda1_4ce1_b25a_943f5951523a)
#define      cool_ng_f36abcb0_dda1_4ce1_b25a_943f5951523a

#include <string>
#include <functional>
#include <iostream>

#include "cool/ng/impl/platform.h"
#include "cool/ng/exception.h"
#include "cool/ng/traits.h"
#include "cool/ng/async/runner.h"
#include "cool/ng/impl/async/task.h"

namespace cool { namespace ng {

namespace async {

/**
 * Tags marking the task kinds.
 */
struct tag
{
/**
 * Simple task tag.
 *
 * Simple task contains a user's @em Callable and is associated with a
 * specific @ref runner, which will, when requested, schedule and execute the 
 * @em Callable. A simple task may accept one input parameter and may or may 
 * not return a value. Both the input parameter and the return value of the
 * simple task are determined by inspecting the user @em Callable, as follows:
 *  * the input paramter of the simple task is the second parameter of the
 *    @em Callable. If the @em Callable does not have the second parameter the
 *    simple task will accept no input parameter
 *  * the return value of the simple task is the return value of the user supplied
 *    @em Callable. If the @em Callable does not return value the simple task
 *    will not return value either.
 *
 * The user @em Callable may be a function pointer, lambda closure,
 * @c std::function, or any other functor object as long as it provides a 
 * single overload of the function call operator '<tt>()</tt>' with the
 * following signature:
 *  * the first parameter must be a <tt>std::shared_ptr</tt>, or a <tt>const</tt>
 *    @em lvalue reference to one, to the @ref runner type the simple task is 
 *    associated with
 *  * The optional second parameter of any type; if present it willl determine
 *    the input paramer of the simple task.
 *
 * Whe the @em Callable is executed its first argument will be set to the @ref
 * runner executing the simple task and the second parameter, if present, will
 * be se to either the value passed to the task's @c run() method
 * or to the value determined by the rules of the compound task, if executed as
 * a part of a compound task.
 *
 * <b>Member Types And Requirements</b>@n
 *
 * When created with a call to:
 * @code
 *   ...
 *   auto task = factory::create(runner, callable);
 *   ...
 * @endcode
 * the resulting task type of object @c task exposes the following public type
 * declarations:
 *
 *  <table><tr><th>Member type <th>Declared as
 *    <tr><td><tt>this_type</tt>       <td><tt>decltype(@em task)</tt>
 *    <tr><td><tt>runner_type</tt>     <td><tt>decltype(@em runner)::%element_type</tt>
 *    <tr><td><tt>tag</tt>             <td><tt>tag::simple</tt>
 *    <tr><td><tt>input_type</tt>      <td>type of the second arg to @em callable, @c void if none
 *    <tr><td><tt>result_type</tt>     <td>return type of @em callable
 *  </table>
 *
 * The following requirements are imposed on the user @em callable:
 *   - the first input parameter to @em callable must be of type <tt>const std::shared_ptr<decltype(@em runner)>&</tt>.
 *   - the user @em callable may accept one or two input parameters
 *
 * <b>Examples</b>@n
 *
 * The following code fragment will create a simple task which accepts a
 * parameter of type @c double and returns result of type @c bool:
 * @code
 *   #include <cool/ng/async.h>
 *   using cool::ng::async::runner;
 *   using cool::ng::async::factory;
 *
 *   runnner run;
 *   auto t = factory::create(run,
 *     [] (const std::shared_ptr<runner>& r, double arg)
 *     {
 *       return arg < 0;
 *     });
 * @endcode
 * When run, the task @c t would require a parameter of type @c double, or
 * a type convertible to @c double:
 * @code
 *    t.run(3.14);
 * @endcode
 *
 * The following code fragment will create a simple task with no input parameter
 * and which returns no result:
 * @code
 *   class my_runner : public runner {
 *      ....
 *   };
 *      ....
 *   my_runner run;
 *   auto t2 = factory::create(run,
 *     [] (const std::shared_ptr<runner>& r)
 *     {
 *       r->do_something();
 *     });
 * @endcode
 * When run, the task @c t2 requires no input parameter:
 * @code
 *    t2.run();
 * @endcode
 *
 * @note Objects created by <tt>std::bind</tt> usually provide several overloads
 * of operator '<tt>()</tt>' and cannot be directly used to create simple tasks.
 * Convert them to an appropriate <tt>std::function</tt> object first.
 */
   using simple = detail::tag::simple;
/**
 * Sequential compound task tag.
 *
 * Sequential tasks are compound tasks that consist of two or more subtasks. The
 * order of the subtasks is determined at the sequential task creation and is 
 * the same as they are provided to the call to @ref factory::sequence factory
 * method.
 * <br>
 * The input parameter of the sequential compund task is the input parameter of
 * its first subtask. The result value and the type of the result of the
 * sequential compound task is the result of of its last subtask. All subtasks
 * in a sequential compound tasks must be chained, meaning that the type of the
 * result of the preceding subtask must match the type of the input of the next
 * subtask. During the execution the sequential task will, in the same order as 
 * they were provided at creation, schedule each subtask for execution, wait for
 * its execution to complete, and fetch its result and pass it as an input parameter
 * into the next subtask.
 *
 * <b>Member Types And Requirements</b>@n
 *
 * When created with a call to:
 * @code
 *   ...
 *   auto task = factory::sequence(task_1, task_2, .... , task_n);
 *   ...
 * @endcode
 * the resulting task type of object @c task exposes the following public type
 * declarations:
 *
 *  <table><tr><th>Member type <th>Declared as
 *    <tr><td><tt>this_type</tt>       <td><tt>decltype(@em task)</tt>
 *    <tr><td><tt>runner_type</tt>     <td><tt>detail::default_runner_type</tt>
 *    <tr><td><tt>tag</tt>             <td><tt>tag::sequential</tt>
 *    <tr><td><tt>input_type</tt>      <td>decltype(@em task_1)::%input_type
 *    <tr><td><tt>result_type</tt>     <td>decltype(@em task_n)::%result_type
 *  </table>
 * Note that sequential task, as all compound tasks, is not associated with any
 * runner and uses @c detail::default_runner_type as a filler type.
 *
 * The following requirements are imposed on the subtasks of the sequential task:
 *  - sequential task must have at least two subtasks
 *  - for every @em i in range 1&ndash;(<i>n</i>-1):
 *    <tt>std::is_same<decltype(task_<i>i</i>)::%result_type, decltype(task_<i>(i+1)</i>)::%input_type>::%value</tt> must yield @c true
 *
 * <b>Exception Handling</b>@n
 *
 * If any of he subtasks in sequence, when run, throws an uncontained exception,
 * the sequential task will terminate the sequence and propagate this exception as
 * its own exception. The subtasks that follow the subtasks that threw the
 * uncontained exception will not be scheduled to run.
 *
 * <b> Examples</b>@n
 *
 * The following code fragmet will create a @em sequential compound task consisting
 * of three simple sub-tasks:
 * @code
 *   #include <cool/ng/async.h>
 *   using cool::ng::async::runner;
 *   using cool::ng::async::factory;
 *   class my_runner_class_1 : public runner { };
 *   class my_runner_class_2 : public runner { };
 *     ...
 *   auto r1 = std::make_shared<my_runner_class_1>();
 *   auto r2 = std::make_shared<my_runner_class_2>();
 *
 *   auto t1 = factory::create(r1,
 *     [] (const std::shared_ptr<runner>& r, double input) -> int
 *     {
 *       ...
 *       return some_integer;
 *     });
 *   auto t2 = factory::create(r2,
 *     [] (const std::shared_ptr<runner>& r, int input) -> my_class
 *     {
 *       ...
 *       return my_class;
 *     });
 *   auto t3 = factory::create(r1,
 *     [] (const std::shared_ptr<runner>& r, const my_class& input) -> void
 *     {
 *       ...
 *     });
 *
 *   auto sequence = factory::sequential(t1, t2, t3);
 * @endcode
 * From the functional perspective, running the sequential task @c sequence:
 * @code
 *   sequence.run(42.0);
 * @endcode
 * would functionally correspond to the following synchronous code:
 * @code
 * int t1(double input) 
 * { 
 *    ... 
 *    return some_integer;
 * }
 * my_class t2(int input)
 * {
 *    ...
 *    return my_class;
 * }
 * void t3(const my_class& input)
 * {
 *    ...
 * }
 *
 * t3( t2( t1( 42.0 ) ) );
 * @endcode
 * The only difference is that tasks @c t1 and @c t3 would run in the context
 * of @ref runner @c r1 and task @c t2 would run in the context of @ref runner
 * @c r2, thus serializing the access to the data grouped around these runners.
 */
   using sequential = detail::tag::sequential;
//using detail::tag::parallel;
/**
 * Conditional compound task.
 *
 * Conditional task is a compund that consists of a predicate task which returns
 * a boolean value, a task which is scheduled for execution when the predicate
 * task returns @c true, and an optionl third task whcih, if present, is
 * scheduled for execution when the predicate task returns @c false. The following
 * are the constrains imposed on subtasks that constitute the conditional compound
 * task (see the next section for more formal expression of the requirements):
 *  - the predicate task must return value of @c bool type
 *  - the predicate task, the second, @em if task, and the optional third, @em else
 *    task must all accept the input parameter of the same type (which can be
 *    @c void if none is desired).
 *  - if the @em else task is not present the result type of the @em if task
 *    must be @c void
 *  - if the @em else task is present the result types of both @em if and
 *    @em else tasks must be the same
 *
 * When run, the conditional compund task first schedules the predicate subtask
 * for execution. When the predicate task is complete and reports the result,
 * it schedules the @em if subtask for execution, if the result of the predicate
 * subtask was @c true. If it was @c false and the @em else subtask is present,
 * it schedules the @em else subtask for execution. If the result of the predicate
 * subtask was @c false and the @em else subtask is not present, no further
 * action is taken and the conditional compound task terminates.
 *
 * <b>Member Types And Requirements</b>@n
 *
 * When created with a call to:
 * @code
 *   ...
 *   auto task = factory::conditional(predicate, if_task);             // (1)
 *   auto task = factory::conditional(predicate, if_task, else_task);  // (2)
 *   ...
 * @endcode
 * the resulting task type of object @c task exposes the following public type
 * declarations:
 *
 *  <table><tr><th>Member type         <th>Declared as
 *    <tr><td><tt>this_type</tt>       <td><tt>decltype(@em task)</tt>
 *    <tr><td><tt>runner_type</tt>     <td><tt>detail::default_runner_type</tt>
 *    <tr><td><tt>tag</tt>             <td><tt>tag::conditional</tt>
 *    <tr><td><tt>input_type</tt>      <td><tt>decltype(@em predicate)::&input_type</tt>
 *    <tr><td><tt>result_type</tt>     <td>if (1): @c void<br>if (2): <tt>decltype(if_task)::%result_type</tt>
 *  </table>
 * Note that sequential task, as all compound tasks, is not associated with any
 * runner and uses @c detail::default_runner_type as a filler type.
 *
 * The following are the requirements for use (1):
 *  - <tt>std::is_same<decltype(predicate)::result_type, bool>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(predicate)::input_type, decltype(if_task)::input_type>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(if_task)::result_type, void>::value</tt> must yield @c true
 *
 * The following are the requirements for use (2):
 *  - <tt>std::is_same<decltype(predicate)::result_type, bool>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(predicate)::input_type, decltype(if_task)::input_type>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(if_task)::input_type, decltype(else_task)::input_type>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(if_task)::result_type, decltype(else_task)::result_type>::value</tt> must yield @c true
 *
 * <b>Exception Handling</b>@n
 *
 * If the predicate subtask, when run, throws an uncontained exception,
 * the conditional task will terminate immediately and propagate this exception as
 * its own exception. Neither @em if_task nor @em else_task, if present, will be
 * scheduled for execution. If either @em if_task or @em else_task, when run,
 * throw and exception, the conditional task will propagate this exception as its
 * own exception.
 *
 * <b>Example</b>@n
 *
 * The following code fragment will create a conditional compound task consisting
 * of the predicate task, the @em if_task and the @em else_task:
 * @code
 *   #include <cool/ng/async.h>
 *   using cool::ng::async::runner;
 *   using cool::ng::async::factory;
 *   class my_runner_class_1 : public runner { };
 *   class my_runner_class_2 : public runner { };
 *     ...
 *   auto r1 = std::make_shared<my_runner_class_1>();
 *   auto r2 = std::make_shared<my_runner_class_2>();
 *
 *   auto predicate = factory::create(r1,
 *     [] (const std::shared_ptr<my_runner_class_1>& r, double input) -> bool
 *     {
 *       ...
 *     });
 *   auto if_task = factory::create(r2,
 *     [] (const std::shared_ptr<my_runner_class_2>& r, double input) -> int
 *     {
 *       ...
 *     });
 *   auto else_task = factory::create(r1,
 *     [] (const std::shared_ptr<my_runner_class_1>& r, double input) -> int
 *     {
 *       ...
 *     });
 *
 *   auto task = factory::conditional(predicate, if_task, else_task);
 *   task.run(3.14);
 * @endcode
 *
 * From the functional perspective this would correspond to the following
 * @c if statement (assuming methods instead of tasks):
 * @code
 *   int result;
 *   if (predicate(3.14))
 *     result = if_task(3.14);
 *   else
 *     result = else_task(3.14);
 * @endcode
 */
  using conditional = detail::tag::conditional;

/**
 * Loop compound task tag.
 *
 * The loop task is a compound task that consits of the predicate task and an
 * optional body task and iterativelly schedules them for the execution
 * until the predicate task returns @c false. When run, the loop task will first
 * schedule the predicate task, wait for its completion and evaluate the result
 * of the predicate task. If @c true, and if the body task is present,
 * it will schedule the body task for execution, wait for its completion and
 * schedule the predicate task again, using the result of the body task as an
 * input to the predicate task. If the predicate task returs @c true but the body
 * task is not present, it will immediatelly schedule the predicate task again.
 * The loop will terminate if the predicate task return @c false and, if @em
 * body_task was present, return the return value of its last iteration as
 * the return value of the loop task.
 *
 * If the body task is present and returns a value, and if the body task is
 * never run (that is if the predicate returns @c false the first time), the
 * return value of the loop compound task is equal to its input. Otherwise the
 * return value of the loop compound task is equal to the last return value of
 * the body task.
 *
 * <b>Member Types And Requirements</b>@n
 *
 * When created with a call to:
 * @code
 *   ...
 *   auto task = factory::loop(predicate);             // (1)
 *   auto task = factory::loop(predicate, body_task);  // (2)
 *   ...
 * @endcode
 * the resulting task type of object @c task exposes the following public type
 * declarations:
 *
 *  <table><tr><th>Member type         <th>Declared as
 *    <tr><td><tt>this_type</tt>       <td><tt>decltype(@em task)</tt>
 *    <tr><td><tt>runner_type</tt>     <td><tt>detail::default_runner_type</tt>
 *    <tr><td><tt>tag</tt>             <td><tt>tag::loop</tt>
 *    <tr><td><tt>input_type</tt>      <td>if (1): @c void<br>if (2):<tt>decltype(<i>predicate</i>)::%input_type</tt>
 *    <tr><td><tt>result_type</tt>     <td>if (1): @c void<br>if (2):<tt>decltype(<i>body_task</i>)::%result_type</tt>
 *  </table>
 * Note that loop task, as all compound tasks, is not associated with any
 * runner and uses @c detail::default_runner_type as a filler type.
 *
 * The following are the requirements for use (1):
 *  - <tt>std::is_same<decltype(predicate)::result_type, bool>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(predicate)::input_type, void>::value</tt> must yield @c true
 *
 * The following are the requirements for use (2):
 *  - <tt>std::is_same<decltype(predicate)::result_type, bool>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(predicate)::input_type, decltype(body_task)::input_type>::value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(predicate)::input_type, decltype(body_task)::result_type>::value</tt> must yield @c true
 *
 * <b>Exception Handling</b>@n
 *
 * If the predicate subtask, when run, throws an uncontained exception,
 * the loop task will terminate immediately and propagate this exception as
 * its own exception. The @em body_task  will not be
 * scheduled for execution. If the @em body_task subtask, when run,
 * throws and exception, the loop task will terminate and propagate
 * this exception as its own exception.
 *
 * <b>Example</b>@n
 *
 * The following code fragment will create a loop compound task consisting
 * of the predicate subtask and the @em body subttask:
 * @code
 *   #include <cool/ng/async.h>
 *   using cool::ng::async::runner;
 *   using cool::ng::async::factory;
 *   class my_runner_class_1 : public runner { };
 *   class my_runner_class_2 : public runner { };
 *     ...
 *   auto r1 = std::make_shared<my_runner_class_1>();
 *   auto r2 = std::make_shared<my_runner_class_2>();
 *
 *   auto predicate = factory::create(r1,
 *     [] (const std::shared_ptr<my_runner_class_1>& r, double input) -> bool
 *     {
 *       ...
 *     });
 *   auto body = factory::create(r2,
 *     [] (const std::shared_ptr<my_runner_class_2>& r, double input) -> double
 *     {
 *       ...
 *     });
 *
 *   auto task = factory::loop(predicate, body;
 *   task.run(3.14);
 * @endcode
 *
 * From the functional perspective this would correspond to the following
 * @c if statement (assuming methods instead of tasks):
 * @code
 *   double result = 3.14;
 *   while (predicate(result))
 *     result = body(result);
 * @endcode
 */
  using loop = detail::tag::loop;

/**
 * Repeat compound task tag.
 *
 * The repeat task is a compound task that repeatedly schedules its subtask for
 * execution. The number of repetitions is specified as the parameter to the
 * @c run() call. When run, the repeat compound task will schedule its subtask for
 * execution, wait for its completion and schedule it again, repeating this cycle
 * the number of times specified to the @c run() call. The subtask will receive
 * the iteration number as its input parameter, in range <i>0&ndash;(num_repetitions-1)</i>.
 * The return value, if any, of the repeat compound task is the last return
 * value of its subtask. If the subtask was never run, the return value is a
 * default constructed instance of the return value type.
 *
 * <b>Member Types And Requirements</b>@n
 *
 * When created with a call to:
 * @code
 *   ...
 *   auto task = factory::repeat(subtask);
 *   ...
 * @endcode
 * the resulting task type of object @c task exposes the following public type
 * declarations:
 *
 *  <table><tr><th>Member type         <th>Declared as
 *    <tr><td><tt>this_type</tt>       <td><tt>decltype(@em task)</tt>
 *    <tr><td><tt>runner_type</tt>     <td><tt>detail::default_runner_type</tt>
 *    <tr><td><tt>tag</tt>             <td><tt>tag::repeat</tt>
 *    <tr><td><tt>input_type</tt>      <td><tt>std::size_t</tt>
 *    <tr><td><tt>result_type</tt>     <td><tt>decltype(@em subtask)::%result_type)</tt>
 *  </table>
 * Note that repeat task, as all compound tasks, is not associated with any
 * runner and uses @c detail::default_runner_type as a filler type.
 *
 * The following are the requirements for the @em subtask:
 *  - <tt>std::is_same<decltype(subtask)::input_type, std::size_t>::value</tt> must yield @c true
 *  - <tt>decltype(subtask)::%result_type</tt> most be default constructible or @c void
 *
 * <b>Exception Handling</b>@n
 *
 * If the subtask, when run at any iteration, throws an uncontained exception,
 * the repeat task will terminate immediately and propagate this exception as
 * its own exception. The @em subtask will not be scheduled to run again regardless
 * of the state of the internal iteration counter.
 *
 * <b>Example</b>@n
 *
 * @code
 *   #include <cool/ng/async.h>
 *   using cool::ng::async::runner;
 *   using cool::ng::async::factory;
 *   class my_runner_class : public runner { ... class content ...  };
 *     ...
 *   auto r = std::make_shared<my_runner_class>();
 *
 *   auto t1 = factory::create(r,
 *     [] (const std::shared_ptr<my_runner_class>& r, std::size_t counter) -> void
 *     {
 *       ...
 *     });
 *   auto task = factory::repeat(t1);
 *      ...
 *   task.run(100);   // execute task t1 100 times
 * @endcode
 *
 * Functionally, the repeat compound task corresponds to the <tt>for</tt>
 * loop in the synchronous programming. Thus the above example functionally
 * corresponds to the following synchronous pattern:
 * @code
 *   void t1(std::size_t counter)
 *   {
 *     ...
 *   }
 *
 *     ...
 *
 *   for (std::size_t i = 0; i < 100; ++i)
 *     t1(i);
 * @endcode
 *
 */
  using repeat = detail::tag::repeat;
/**
 * Intercept compound task.
 *
 * An intercept task is a compound task that handles the exceptions that may
 * be thrown in its subtask. The intercept tasks consists of:
 *  - the @em main subtask (a @em try task) that is expected to throw exception(s)
 *    that must be handled
 *  - one or more exception handling subtasks (the @em catch tasks) that will 
 *    only be scheduled to run if the @em try task throws an exception of the
 *    appropriate type. The try tasks mustv accept an input parameter.
 *
 * When an intercept task is run, it immediatelly schedules its @em try task
 * for execution. Should the @em try task throws an exception, the intercept
 * compound task will @em catch the thrown exception and inspect the @em catch
 * tasks in the same order as they were specified to the @em factory::intercept
 * call. The first @em catch task that accepts the input parameter of the same
 * type, or of the base type of the thrown exception object, will be scheduled
 * to run and will receive the thrown exception object as its input. If no
 * mathing @em catch task is found, the exception object is propagated out of
 * the intercept compound task.
 *
 * The @em catch task with the input parameter of type @c std::exception_ptr has
 * has a special meaning - it acts as a catch-all entity and will intercept
 * an exception object of any type.
 *
 * <b>Member Types And Requirements</b>@n
 *
 * When created with a call to:
 * @code
 *   ...
 *   auto task = factory::intercept(try, catch_1, catch_2, ... catch_n, catch_all);
 *   ...
 * @endcode
 * the resulting task type of object @c task exposes the following public type
 * declarations:
 *
 *  <table><tr><th>Member type         <th>Declared as
 *    <tr><td><tt>this_type</tt>       <td><tt>decltype(@em task)</tt>
 *    <tr><td><tt>runner_type</tt>     <td><tt>detail::default_runner_type</tt>
 *    <tr><td><tt>tag</tt>             <td><tt>tag::intercept</tt>
 *    <tr><td><tt>input_type</tt>      <td><tt>decltype(@em try)::%input_type</tt>
 *    <tr><td><tt>result_type</tt>     <td><tt>decltype(@em try)::%result_type</tt>
 *  </table>
 * Note that intercept task, as all compound tasks, is not associated with any
 * runner and uses @c detail::default_runner_type as a filler type.
 *
 * The following are the requirements for the subtasks of the intercept task:
 *  - for each @em i in range 1&ndash;<i>n</i>: <tt>std::is_same<decltype(try)::%result_type, std::is_same<decltype(catch_<i>i</i>)::%result_type>::%value</tt> must yield @c true
 *  - <tt>std::is_same<decltype(try)::%result_type, std::is_same<decltype(catch_all)::%result_type>::%value</tt> must yield @c true
 *  - for each @em i in range 1&ndash;<i>n</i>: <tt>std::is_same<decltype(catch_<i>i</i>)::%input_type, void>::%value</tt> must yield @c false
 *  - <tt>std::is_same<decltype(catch_all)::%input_type, std::exception_ptr>::%value</tt> must yield @c true
 *
 * <b>Exception Handling</b>@n
 *
 * If the @em try subtask, when run, throws an uncontained exception,
 * the intercept task will examine the @em catch_i subtasks in the creation order
 * to find one that would handle the exception. If such @em catch subtask is found,
 * the intercept will schedule it to run and will pass the exception object as an
 * input parameter. The result value of this subtask, if any, will be returned as
 * the result of the intercept compound task. If no handling @em catch subtask
 * is found, the intercept compund task will propagate the exception as its
 * own exception and terminate immediatelly. No @em catch subtasks will be run
 * in this case.
 *
 * If the handling subtask, when run, throws an uncontained exception, the intercept
 * task will propagate the exception as its own and terminate. No result value
 * is produced.
 *
 * <b>Example</b>@n
 *
 * @code
 *   #include <cool/ng/async.h>
 *   using cool::ng::async::runner;
 *   using cool::ng::async::factory;
 *   class my_runner_class_1 : public runner {  ... class content ... } };
 *   class my_runner_class_2 : public runner {  ... class content ... } };
 *     ...
 *   auto r1 = std::make_shared<my_runner_class_1>();
 *   auto r2 = std::make_shared<my_runner_class_2>();
 *
 *   auto t1 = factory::create(r1,
 *     [] (const std::shared_ptr<my_runner_class_1>& r, double input) -> int
 *     {
 *       ...
 *     });
 *   auto t2 = factory::create(r2,
 *     [] (const std::shared_ptr<my_runner_class_2>& r, const std::runtime_error& e) -> int
 *     {
 *       ...
 *     });
 *   auto t3 = factory::create(r1,
 *     [] (const std::shared_ptr<my_runner_class_1>& r, const std::exception_ptr& e) -> int
 *     {
 *       ...
 *     });
 *
 *   auto task = factory::try_catch(t1, t2, t3);
 * @endcode
 * When task @c task is run, it will immediatelly schedule the @em try task @c t1
 * for execution. If this task, during its run, throws an exception, the intercept
 * compound task @c task will catch the exception and examine @em catch tasks
 * @c t1 and @c t2, in this order. Should the exception object be of type
 * @c std::runtime_error, or should this be one of its base types, the intercept
 * compound task @c task will schedule the @em catch task @c t2 for execution on
 * its associated runner @c r1. If the exception was of any other type, it will
 * schedule the @em catch task @c t3 (since it acts as a catch-all @em catch
 * task due to its input parameter) for execution on its associated runner @c r2.
 * The @c int result returned by either @c t2 or @c t3 Em catch task would then
 * represent the result of the intercept compound task @c task.
 * <br>
 * Functionally, the intercept compound task corresponds to the <tt>try-catch</tt>
 * block in the synchronous programming. Thus the above example functionally
 * corresponds to the following synchronous pattern:
 * @code
 *   try
 *   {
 *     // code of try task t1 that may throw
 *   }
 *   catch (const std::runtime_error& e)
 *   {
 *     // code of catch task t2
 *   }
 *   catch (...)
 *   {
 *     // code of catch task t3
 *   }
 * @endcode
 * @note Except for catch-all @em catch task, the exception object is re-thrown
 *  and re-caught at each @em catch task to test whether this @em catch task
 *  should intercept it or not. This may incurr certain processing overhead
 *  should the intercept compound task have a long string of @em catch tasks.
 */
 using intercept = detail::tag::intercept;
};

struct factory;

/**
 * A class template representing the objects that can be scheduled for
 * execution by one of the @ref runner "runners".
 *
 * A task can be either a <em>simple task</em> which contains a @em Callable and 
 * is associated with a specific @ref runner or a <em>compound task</em> which
 * contains other tasks organized and exceuted in a specific way. Compound tasks
 * themselves are not associated with @ref runner "runners" as they do not contain
 * executable code.
 *
 * <b>Task Properties</b>@n
 * Each task may accept at most one input parameter. The type of the input parameter
 * can be inspected using @c task::input_type public type. If @c void the task does
 * not accept input parameter. The input parameter can be specified explicitly
 * when the task is scheduled for execution via @ref task::run() "run()" method,
 * or is specified implicitly by the execution context of the compound task, if 
 * the task is an element of a compound task and is scheduled to run as a part
 * of the compound task execution.
 *
 * The task may return a result of any data type. The type of the result of a
 * simple task is determined by the return value of its @em Callable. Teh type 
 * of the result of the compound task is determined by the rules governing each 
 * particular kind of the compound task. Since the tasks are executed asynchronously,
 * the results cannot be reported back into the user code, and if the result is
 * not picked up by the execution context of the compund task it is irrevocably
 * lost.
 *
 * Formally, each tasks exposes the following public member types:
 *   - @c this_type, the type of this task type
 *   - @c runner_type, the type of the @ref runner, associated with this task type
 *   - @c tag, the @ref tag type associated with this task type - the tag type
 *      determines the kind of this task (e.g. simple, sequential, loop, ...)
 *   - @c input_type, the type of the input for this task type. Input type
 *     @c void denotes no input.
 *   - @c result_type, the type of the result value of this task type. Result type
 *     @c void denotes that this task type has no result value.
 *   - @c impl_type, the type that will store the static information about this
 *     task type.
 *
 * <b>Simple Tasks</b>@n
 * Simple tasks are the cornerstone of asynchronous execution paradigm and the only
 * type of tasks that contain the user code. The user code is provided in a form
 * of @em Callable object, passed to one of the @ref factory::create() "create"
 * methods of the @ref factory class. When a simple task is scheduled for
 * execution, the @ref runner environment will pass a shared pointer of the
 * correct type as the first argument to the user provided @em Callable. This
 * feature can be seeen as a sort of "shared this" substitute for @c this
 * pointer, common in synchronous programming.
 *
 * <b>Composed Tasks</b>@n
 *
 * Composed tasks are tasks that combine one or more tasks into a single task. The
 * composed tasks are one of the following:
 *  - @em sequential
 *  - @em parallel
 *  - @em conditional
 *  - @em loop
 *  - @em repeat
 *  - @em intercept
 *
 * See @ref tag for more details on ech kind of tasks.
 */

template <typename TagT, typename RunnerT, typename InputT, typename ResultT, typename... TaskT>
class task
{
 public:
  using this_type   = task;
  using runner_type = RunnerT;
  using tag         = TagT;
  using input_type  = InputT;
  using result_type = ResultT;
  using impl_type   = detail::taskinfo<
      tag
    , runner_type
    , input_type
    , result_type
    , typename std::decay<TaskT>::type::impl_type...>;

 public:
  task() { /* noop */ }
 /**
  * Predicate to check whether the task is empty.
  */
  explicit operator bool () const
  {
    return !!m_impl;
  }
 /**
  * Schedule task for execution.
  */
  template <typename T = InputT>
  void run(const typename std::decay<typename std::enable_if<
      !std::is_same<T, void>::value && !std::is_rvalue_reference<T>::value
    , T>::type>::type& arg_)
  {
    m_impl->run(m_impl, arg_);
  }

  // rvalue reference
  template <typename T = InputT>
  void run(typename std::enable_if<
      !std::is_same<T, void>::value && std::is_rvalue_reference<T>::value
    , T>::type arg_)
  {
    m_impl->run(m_impl, std::move(arg_));
  }
 /**
  * Schedule task for execution.
  */
  template <typename T = InputT>
  typename std::enable_if<std::is_same<T, void>::value, void>::type run()
  {
    m_impl->run(m_impl);
  }

 private:
  friend struct factory;
  task(const std::shared_ptr<impl_type> impl_) : m_impl(impl_)
  { /* noop */ }

 private:
  std::shared_ptr<impl_type> m_impl;
};

/**
 * Helper type alias for tasks accessing member data.
 *
 * This type alias may be used to simplify the syntax of member methods of
 * @ref runner derived data classes that return tasks accessing member data when
 * using C++11 based compilers which do not support @c auto declared
 * functions, as in the following example:
 * @code
 *   class MyClass : public cool::ng::async::runner
 *   {
 *     public:
 *       cool::ng::async::GetterTask<MyClass, int> get_value()
 *       {
 *         return cool::ng::async::factor::create(
 *           myself,
 *           [](const std::shared_ptr<MyClass>& r) { return value; });
 *       }
 *     private:
 *       int value;
 *       std::weak_ptr<MyClass> myself;
 *   };
 * @endcode
 * The C++14 enabled compilers that support @c auto declared functions would
 * allow further simplification of the above @c get_value declaration into:
 * @code
 *     ...
 *   auto get_value()
 *   {
 *     ...
 * @endcode
 * but such simplifiaction is not possible with C++11 based compilers.
 */
template <typename ClassT, typename ValueT>
using GetterTask = task<tag::simple, ClassT, void, ValueT>;

/**
 * Helper type alias for tasks modifying member data.
 *
 * This type alias may be used to simplify the syntax of member methods of
 * @ref runner derived data classes that return tasks modifying member data when
 * using C++11 based compilers which do not support @c auto declared
 * functions, as in the following example:
 * @code
 *   class MyClass : public cool::ng::async::runner
 *   {
 *     public:
 *       cool::ng::async::MutatorTask<MyClass, int> set_value()
 *       {
 *         return cool::ng::async::factor::create(
 *           myself,
 *           [](const std::shared_ptr<MyClass>& r, int arg) { value = arg; });
 *       }
 *     private:
 *       int value;
 *       std::weak_ptr<MyClass> myself;
 *   };
 * @endcode
 * The C++14 enabled compilers that support @c auto declared functions would
 * allow further simplification of the above @c get_value declaration into:
 * @code
 *     ...
 *   auto set_value()
 *   {
 *     ...
 * @endcode
 * but such simplifiaction is not possible with C++11 based compilers.
 */
template <typename ClassT, typename ValueT>
using MutatorTask = task<tag::simple, ClassT, ValueT, void>;

/**
 * Task factory
 */
struct factory {
 public:
  //--- ------------------------------------------------------------------------
  //--- Simple tasks factory methods
  //--- ------------------------------------------------------------------------
  /**
   * Factory method to create @ref tag::simple "simple" tasks.
   *
   * @param r_ @ref runner to use for task execution
   * @param f_ user Callable runner should invoke during task execution
   *
   * @see @ref tag::simple "simple" task
   */
  template <typename RunnerT, typename CallableT>
  inline static task<
      tag::simple
    , RunnerT
    , typename traits::arg_type<1, CallableT>::type
    , typename traits::functional<CallableT>::result_type
  > create(const std::weak_ptr<RunnerT>& r_, const CallableT& f_)
  {
    using result_type = typename traits::functional<CallableT>::result_type;
    using input_type = typename traits::arg_type<1, CallableT>::type;
    using task_type = task<tag::simple, RunnerT, input_type, result_type>;

    // Make diagnostics a bit more user friendly - do some compile time checks
    // user callable must accept one or two parameters ...
    static_assert(
        traits::functional<CallableT>::arity::value > 0 && traits::functional<CallableT>::arity::value < 3
      , "The user supplied callable must accept one or two parameters");
    // ... the first paramer must be shared_ptr to runner type ...
    static_assert(
        std::is_same<
            std::shared_ptr<RunnerT>
          , typename traits::functional<CallableT>::template arg<0>::info::naked_type
        >::value
      , "The user Callable must accept std::shared_ptr<runner-type> as the first parameter");

    // ... but neither passed as rvalue reference ...
    static_assert(
        !traits::functional<CallableT>::template arg<0>::info::is_rref::value
      , "The first parameter to user Callable must not be rvalue reference");
    // ... nor as non-const lvalue reference
    static_assert(
        !traits::functional<CallableT>::template arg<0>::info::is_lref::value
     || (traits::functional<CallableT>::template arg<0>::info::is_lref::value
          && traits::functional<CallableT>::template arg<0>::info::is_const::value)
      , "The first parameter to user Callable must either be by value or const lvalue reference");

    return task_type(std::make_shared<typename task_type::impl_type>(r_, f_));
  }

  template <typename RunnerT, typename CallableT>
  inline static task<
      tag::simple
    , RunnerT
    , typename traits::arg_type<1, CallableT>::type
    , typename traits::functional<CallableT>::result_type
  > create(const std::shared_ptr<RunnerT>& r_, const CallableT& f_)
  {
    return factory::create(std::weak_ptr<RunnerT>(r_), f_);
  }

  // IMPLEMENTATION_NOTE:
  //   Compound tasks do not need a runner of their own hence the
  //   default_runner_type is used as a type constant for all compounds.
  //--- -----------------------------------------------------------------------
  //--- Sequential tasks factory methods
  //--- -----------------------------------------------------------------------
  /**
   * Factory method for creating @ref tag::sequential "sequential" compound tasks.
   *
   * @param t_ two or more tasks to run in sequence
   *
   * @see @ref tag::sequential "sequential" compound task
   */
  template <typename... TaskT>
  inline static task<
      tag::sequential
    , detail::default_runner_type
    , typename detail::traits::get_first<TaskT...>::type::input_type
    , typename detail::traits::get_sequence_result_type<TaskT...>::type
  > sequence(const TaskT&... t_)
  {
    static_assert(
        sizeof...(t_) > 1
      , "It takes at least two tasks to create a sequential compound task");
    static_assert(
        detail::traits::is_chain<typename std::decay<TaskT>::type...>::result::value
      , "The type of the parameter of each task in the sequence must match the return type of the preceding task.");

    using result_type = typename detail::traits::get_sequence_result_type<TaskT...>::type;
    using input_type = typename detail::traits::get_first<TaskT...>::type::input_type;
    using task_type = task<tag::sequential, detail::default_runner_type, input_type, result_type>;

    return task_type(std::make_shared<typename task_type::impl_type>(t_.m_impl...));
  }

  /**
   * Factory method for creating @ref tag::intercept "intercept" compound tasks.
   *
   * @param t_ task to execute (@em try task)
   * @param c_ one or more exeception handling (@em catch) tasks
   *
   * @see @ref tag::intercept "intercept" compound task
   */
  template <typename TryT, typename... CatchT>
  inline static task<
      tag::intercept
    , detail::default_runner_type
    , typename TryT::input_type
    , typename TryT::result_type
  > try_catch(const TryT& t_, const CatchT&... c_)
  {
    static_assert(
        sizeof...(c_) > 0
      , "It takes at least one catch task to create a try_catch (intercept) compound task");

#if defined(WINDOWS_TARGET)
#if (_MSC_VER > 1800)
    static_assert(
        detail::traits::is_same<typename TryT::result_type, typename CatchT::result_type...>::value
      , "TryT task and CatchT tasks must have the same result type");
#endif
#endif
    using result_type = typename TryT::result_type;
    using input_type = typename TryT::input_type;
    using task_type = task<tag::intercept, detail::default_runner_type, input_type, result_type>;

    return task_type(std::make_shared<typename task_type::impl_type>(t_.m_impl, c_.m_impl...));
  }

  template <typename PredicateT, typename IfT, typename ElseT>
  inline static task<
      tag::conditional
    , detail::default_runner_type
    , typename PredicateT::input_type
    , typename IfT::result_type
  > conditional(const PredicateT& p_, const IfT& if_, const ElseT& else_)
  {
    static_assert(
        std::is_same<typename PredicateT::result_type, bool>::value
      , "The predicate task must return result of type bool");
    static_assert(
        detail::traits::is_same<typename PredicateT::input_type, typename IfT::input_type, typename ElseT::input_type>::value
      , "All tasks must accept the input parameter of the same type");
    static_assert(
        std::is_same<typename IfT::result_type, typename ElseT::result_type>::value
      , "If and Else part tasks must return result of the same type");

    using result_type = typename IfT::result_type;
    using input_type = typename PredicateT::input_type;
    using task_type = task<tag::conditional, detail::default_runner_type, input_type, result_type>;

    return task_type(std::make_shared<typename task_type::impl_type>(p_.m_impl, if_.m_impl, else_.m_impl));
  }

  template <typename PredicateT, typename IfT>
  inline static task<
      tag::conditional
    , detail::default_runner_type
    , typename PredicateT::input_type
    , typename IfT::result_type
  > conditional(const PredicateT& p_, const IfT& if_)
  {
    static_assert(
        std::is_same<typename IfT::result_type, void>::value
      , "The IfT part in a conditional without ElseT part must not return value");
    static_assert(
        std::is_same<typename PredicateT::result_type, bool>::value
      , "The predicate task must return result of type bool");
    static_assert(
        detail::traits::is_same<typename PredicateT::input_type, typename IfT::input_type>::value
      , "All tasks must accept the input parameter of the same type");
    using result_type = typename IfT::result_type;
    using input_type = typename PredicateT::input_type;
    using task_type = task<tag::conditional, detail::default_runner_type, input_type, result_type>;

    return task_type(std::make_shared<typename task_type::impl_type>(p_.m_impl, if_.m_impl));
  }

  template <typename TaskT>
  inline static task<
      tag::repeat
    , detail::default_runner_type
    , std::size_t
    , typename TaskT::result_type
  > repeat( const TaskT t_)
  {
    using result_type = typename TaskT::result_type;
    using input_type = std::size_t;
    using task_type = task<tag::repeat, detail::default_runner_type, input_type, result_type>;

    static_assert(
        std::is_same<typename TaskT::input_type, std::size_t>::value
      , "The task to be repeated must have input_type std::size_t.");
    return task_type(std::make_shared<typename task_type::impl_type>(t_.m_impl));
  }

  template <typename PredicateT, typename BodyT>
  inline static task<
      tag::loop
    , detail::default_runner_type
    , typename PredicateT::input_type
    , typename BodyT::result_type
  > loop(const PredicateT& p_, const BodyT& body_)
  {
    static_assert(
        std::is_same<typename PredicateT::result_type, bool>::value
      , "The predicate task must return bool value");
    static_assert(
        std::is_same<typename PredicateT::input_type, typename BodyT::input_type>::value
      , "The predicate and body tasks must accept parameter of the same type.");
    static_assert(
        detail::traits::is_same<typename PredicateT::input_type, typename BodyT::result_type>::value
      , "The return value type of body task must match input parameter of predicate.");
    using result_type = typename BodyT::result_type;
    using input_type = typename PredicateT::input_type;
    using task_type = task<tag::loop, detail::default_runner_type, input_type, result_type>;

    return task_type(std::make_shared<typename task_type::impl_type>(p_.m_impl, body_.m_impl));
  }

  template <typename PredicateT>
  inline static task<
      tag::loop
    , detail::default_runner_type
    , void
    , void
  > loop(const PredicateT& p_)
  {
    static_assert(
        std::is_same<typename PredicateT::result_type, bool>::value
      , "The predicate task must return bool value");
    static_assert(
        std::is_same<typename PredicateT::input_type, void>::value
      , "The predicate cannot have input parameter.");

    using result_type = void;
    using input_type = void;
    using task_type = task<tag::loop, detail::default_runner_type, input_type, result_type>;

    return task_type(std::make_shared<typename task_type::impl_type>(p_.m_impl));
  }

};



#if 0
/**
 * This is task factory.
 */
class factory
{
 public:
  /**
   * Create a simple task.
   *
   * Creates and returns a simple task associated with the @ref runner. When
   * activated via @ref task::run() method, the task will submit its @em Callable for
   * execution with the associated runner. The @em Callable must satisfy the
   * following requirements:
   *
   *   - it can be a function pointer, lambda closure, @c std::function, or
   *     a functor object as long as it provides a single overload of the
   *     function call operator <tt>()</tt>.
   *   - it must accept <tt>std::shared_ptr</tt> to the @c RunnerT as its first
   *     parameter
   *   - it may accept one additional input parameter which is then the input
   *     parameter of the task
   *   - it may, but does not need to, return a value of any type.
   *
   * The @em Callable implementation should not contain any blocking code. The
   * blocking code will block the worker thread executing the callable. Since
   * the number of working threads in the pool may be limited (optimal number
   * should be close to the number of precessor cores to avoid taskinfo switches)
   * the pool may run out of the active, non-blocked threads, will will block
   * the execution of the entire program. The @em Callable implementation should
   * use I/O event sources instead of blocking read/write calls, and should
   * organize the data around runners instead of using the thread synchronization
   * primitives.
   *
   * The @em Callable may return result. The result value will be passed to the
   * next task in the sequence as an input parameter. If the next task in the
   * sequence us the parallel compound task. the result will be passed to every
   * subtask of this compund task.
   *
   * The @em Callable may throw a C++ exception. The unhandled exception will
   * travel along the sequence of tasks until it reaches the exception handling
   * task that can interept it. None of the tasks between the throwing task and
   * the interception task will be scheduled for execution.
   *
   * @param r_ the @ref runner which will be used to run the @em Callable
   * @param f_ user @em Callable to execute
   *
   * @note Unfortunately the objects returned by the @c std::bind template
   *   provide multiple function call operator overloads and cannot be passed
   *   to the simple task directly, without transforming them into @c std::function
   *   object first.
   *
   * @warning The @em Callable should not contain blocking code.
   */
  template <typename RunnerT, typename CallableT>
  inline static task<
      detail::tag::simple
    , RunnerT
    , typename traits::arg_type<1, CallableT>::type
    , typename traits::functional<CallableT>::result_type
  > create(const std::weak_ptr<RunnerT>& r_, const CallableT& f_)
  {
    using result_type = typename impl::traits::function_traits<CallableT>::result_type;
    using input_type = typename impl::traits::arg_type<1, CallableT>::type;
    using task_type = task<impl::tag::simple, RunnerT, input_type, result_type>;

    return task_type(std::make_shared<typename task_type::impl_type>(r_, f_));
  }
  template <typename RunnerT, typename CallableT>
  inline static task<
      impl::tag::simple
    , RunnerT
    , typename impl::traits::arg_type<1, CallableT>::type
    , typename impl::traits::function_traits<CallableT>::result_type
  > create(const std::shared_ptr<RunnerT>& r_, const CallableT& f_)
  {
    return factory::create(std::weak_ptr<RunnerT>(r_), f_);
  }


  template <typename... TaskT>
  inline static task<
      impl::tag::serial
    , impl::default_runner_type
    , typename impl::traits::first_task<TaskT...>::type::input_type
    , typename impl::traits::sequence_result<TaskT...>::type
    , typename std::decay<TaskT>::type ...
  > sequential(TaskT&&... t_)
  {
    using result_type = typename impl::traits::sequence_result<TaskT...>::type;
    using input_type = typename impl::traits::first_task<TaskT...>::type::input_type;
    using task_type = task<impl::tag::serial, impl::default_runner_type, input_type, result_type, typename std::decay<TaskT>::type...>;

    static_assert(
        impl::traits::all_chained<typename std::decay<TaskT>::type...>::result::value
      , "The type of the parameter of each task in the sequence must match the return type of the preceding task.");

    return task_type(std::make_shared<typename task_type::impl_type>(t_.m_impl...));
  }

  template <typename... TaskT>
  inline static task<
      impl::tag::parallel
    , typename impl::traits::first_task<TaskT...>::type::runner_type
    , typename impl::traits::first_task<TaskT...>::type::input_type
    , typename impl::traits::parallel_result<TaskT...>::type
    , typename std::decay<TaskT>::type ...
  > parallel(TaskT&&... t_)
  {
    using result_type = typename impl::traits::parallel_result<TaskT...>::type;
    using input_type = typename impl::traits::first_task<TaskT...>::type::input_type;
    using task_type = task<
        impl::tag::parallel
      , typename impl::traits::first_task<TaskT...>::type::runner_type
      , input_type
      , result_type
      , typename std::decay<TaskT>::type...>;

    static_assert(
        impl::traits::all_same<typename std::decay<typename std::decay<TaskT>::type::input_type>::type...>::value
      , "All parallel tasks must have the same parameter type or no parameter");

    return task_type(std::make_shared<typename task_type::impl_type>());
  }

  /**
   * Create a compound task which will, depending on the predicate, execute one
   * of its subtasks.
   *
   * This method template creates and returns a compound task which will, when
   * executed, evaluate the predicate @c pred_, and run task @c t1_ if the predicate
   * evaluates to @c true. Owherwise it will run task @c t2_. Such compount task
   * can be understood as an equivalent of the @c if statement in the synchronous
   * programming. The following are requirements for the user supplied predicate
   * and tasks:
   * - the predicate and both tasks must accept an input parameter of the same
   *   type. The same input value passed to the predicate to determine which
   *   subtask to execute will be passed to the chosen subtask.
   * - the predicate must return result of the @c bool type.
   * - Both tasks must return result of the same type. The result type of the
   *   compound task is the result type of the supplied subtasks.
   *
   * @param pred_ user supplied predicate used to determine which task to execute
   * @param t1_   task to run if the predicate evaluates to @c true
   * @param t2_   task to run if the predicate evaluates to @c false
   *
   * @note The thread context of evaluating the predicate @c pred_ is not
   * defined.
   *
   */
  template <typename PredicateT, typename TaskT, typename TaskY>
  inline static task<
      impl::tag::conditional
    , impl::default_runner_type
    , typename TaskT::input_type
    , typename TaskT::result_type
    , typename std::decay<TaskT>::type
    , typename std::decay<TaskY>::type
  > conditional(const PredicateT& pred_, const TaskT& t1_, const TaskY& t2_)
  {
    using pred_input_type = typename std::decay<typename impl::traits::function_traits<PredicateT>::template arg<0>::type>::type;
    using input_type = typename TaskT::input_type;
    using result_type = typename TaskT::result_type;
    using task_type = task<
        impl::tag::conditional
      , impl::default_runner_type
      , input_type
      , result_type
      , typename std::decay<TaskT>::type
      , typename std::decay<TaskY>::type>;

    static_assert(
        std::is_same<typename TaskT::input_type, typename TaskY::input_type>::value
      , "Both tasks and the predicate must accept the input parameter of the same type");
    static_assert(
        std::is_same<typename TaskT::input_type, pred_input_type>::value
      , "Both tasks and the predicate must accept the input parameter of the same type");
    static_assert(
        std::is_same<typename TaskT::result_type, typename TaskY::result_type>::value
      , "Both tasks must return value of the same type");
    static_assert(
        std::is_same<typename impl::traits::function_traits<PredicateT>::result_type, bool>::value
      , "Predicate must return value of type bool");
    static_assert(
        1 == impl::traits::function_traits<PredicateT>::arity::value
      , "Predicate must accept exactly one parameter");

    return task_type(std::make_shared<typename task_type::impl_type>(
       static_cast<typename task_type::impl_type::predicate_type>(pred_)
      , t1_.m_impl
      , t2_.m_impl
    ));
  }

  /**
   * Create a compound task which will, depending on the predicates, execute one of its subtasks.
   *
   * This method template creates and returns a compound task which will, when
   * executed, evaluate the predicates of each tuple in the same order
   * as supplied to the method call until it finds the predicate which evaluates
   * to @c true. It will then run the task of this tuple. If none of the predicates
   * evaluate to @c true it will select @c default_ task to run. Such compount task
   * can be understood as an equivalent of the @c switch statement in the synchronous
   * programming. The following are requirements for the user supplied predicates
   * and tasks:
   * - all predicates and tasks must accept an input parameter of the same
   *   type. The same input value passed to the predicate to determine which
   *   subtask to execute will be passed to the chosen subtask. The type of
   *   the input parameter will be the type of the input parameter of the returned
   *   compound task
   * - the predicates must return result of the @c bool type.
   * - all tasks must return result of the same type. The result type of the
   *   returned compound task is the result type of the supplied subtasks.
   *
   * @param default_ task to run if none of predicates yields @c true
   * @param arg_ one or more tuples containing predicate / tasks pairs
   *
   * @note The thread context of evaluating the predicates is not defined.
   */
  template <typename TaskY, typename... PredicateT, typename... TaskT>
  inline static task<
      impl::tag::oneof
    , typename TaskY::runner_type
    , typename TaskY::input_type
    , typename TaskY::result_type
    , typename std::decay<TaskY>::type
    , TaskT...
  > oneof(const TaskY& default_, const std::tuple<PredicateT, TaskT>&... arg_)
  {
    using result_type = typename TaskY::result_type;
    using input_type = typename TaskY::input_type;
    using task_type = task<
    impl::tag::oneof
    , typename TaskY::runner_type
    , input_type
    , result_type
    , typename std::decay<TaskY>::type
    , TaskT...>;

    return task_type(std::make_shared<typename task_type::impl_type>());
  }


  /**
   * Create a compound task which will, depending on the predicate, execute the
   * task zero or more times.
   *
   * This method creates and returns a compound task which will, when executed,
   * supply the input parameter to predicate and if the predicate yields @c true
   * will run the task. Upon the task completion its result will be fed to the
   * predicate and if the predicate yields @c true, it will run the task again.
   * This cycle will repeat until the predicate yields false.
   *
   * The following are requirements for the predicate and the user task:
   * - both predicate and the user task must accept the same input parameter
   *   type (which must not be @c void), This will be the input parameter type
   *   of the resulting compound task
   * - the predicate must return @c bool result
   * - the result type of the user task must be the same as its input type.
   *
   * The resulting task will have the public type declarations set to the following
   * types:
   *
   *  - @c runner_type will be set to the <tt>TaskT::runner_type</tt>
   *  - @c input_type will be set to the <tt>TaskT::input_type</tt>, which must
   *   be an integral type
   *  - @c result_type will be set the <tt>TaskT::result_type</tt>, which must
   *   be @c void type.
   *
   * @note The thread context of evaluating the predicate is not defined.
   */
  template <typename PredicateT, typename TaskT>
  inline static task<
      impl::tag::loop
    , impl::default_runner_type
    , typename TaskT::input_type
    , typename TaskT::result_type
    , typename std::decay<TaskT>::type
  > loop(const PredicateT& pred_, const TaskT& t_)
  {
    using pred_input_type = typename std::decay<typename impl::traits::function_traits<PredicateT>::template arg<0>::type>::type;
    using input_type = typename TaskT::input_type;
    using result_type = typename TaskT::result_type;
    using task_type = task<
        impl::tag::loop
      , impl::default_runner_type
      , input_type
      , result_type
      , typename std::decay<TaskT>::type>;

    static_assert(
        std::is_same<pred_input_type, input_type>::value
      , "Predicate and the task must have the same input type");
    static_assert(
        1 == impl::traits::function_traits<PredicateT>::arity::value
      , "Predicate must accept exactly one parameter");
    static_assert(
        std::is_same<typename impl::traits::function_traits<PredicateT>::result_type, bool>::value
      , "Predicate must return value of type bool");
    static_assert(
        std::is_same<input_type, result_type>::value
      , "The result_type of the task must be the same as its input_type");

    return task_type(std::make_shared<typename task_type::impl_type>(
          static_cast<typename task_type::impl_type::predicate_type>(pred_)
        , t_.m_impl));
  }
  /**
   * Create a compound task which will execute the task exactly the specified
   * number of times.
   *
   * This method creates and returns a compound task which will, when executed,
   * run its subtask exactly the number of times specified by its input
   * parameter.
   *
   * The resulting task will have the public type declarations set to the following
   * types:
   *
   *  - @c runner_type will be set to the <tt>TaskT::runner_type</tt>
   *  - @c input_type will be set to the <tt>TaskT::input_type</tt>, which must
   *   be an integral type
   *  - @c result_type will be set the <tt>TaskT::result_type</tt>, which must
   *   be @c void type.
   */
  template <typename TaskT>
  inline static task<
      impl::tag::repeat
    , impl::default_runner_type
    , typename TaskT::input_type
    , typename TaskT::result_type
    , typename std::decay<TaskT>::type
  > repeat(const TaskT& t_)
  {
    using input_type = typename TaskT::input_type;
    using result_type = typename TaskT::result_type;
    using task_type = task<
        impl::tag::repeat
      , impl::default_runner_type
      , input_type
      , result_type
      , typename std::decay<TaskT>::type>;

    static_assert(
        std::is_integral<input_type>::value
      , "The task must accept input of integral type");
    static_assert(
        std::is_void<result_type>::value
      , "The task must not return result");
    return task_type(std::make_shared<typename task_type::impl_type>(t_.m_impl));
  }
};

#endif
#if 0
/**
 * A class template representing the basic objects that can be scheduled for execution
 * by one of the @ref runner "runners".
 *
 * A task can be one of the following:
 *  - a <em>simple task</em>, which contains a @em Callable and is associated with a
 *    specific @ref runner. Invoking the run method on a simple task will schedule
 *    the task for execution with the associated @ref runner by insertion its @em Callable
 *    into the runner's task queue. Simple task objects are created via
 *    taskop::create method teplate.
 *  - a <em>compound task</em> which contains other tasks as its subtasks. Subtasks can be
 *    either compound or simple tasks. When a compound task gets activated (via
 *    a call to run(), for instance) it fetches one or more of its simple subtasks 
 *    and schedules them for execution. The compound task remains active until the 
 *    execution of all of its subtasks is complete. Since the compound tasks do not
 *    contain executable code they are not associated with any @ref runner. Componud
 *    tasks are created by any of the following @ref taskop methods templates: @ref taskop::parallel
 *    "parallel()", @ref taskop::sequential "sequential()", @ref taskop::intercept
 *    "intercept()", or @ref taskop::intercept_all "intercept_all()", or via the corresponding
 *    task class template member methods templates.
 *  - an <em>exception handling task</em> which intercepts an exception of a particular
 *    type, or any exception, that may be thrown by the preceding task. An exception
 *    handling task is any compound or simple tasks that adheres to certain limitations
 *    about its parameter or return value type. One or more exception handling tasks
 *    can be attached to any task using  @ref taskop method templates @ref taskop::intercept
 *    "intercept()" or @ref taskop::intercept_all() "intercept_all()", or by using
 *    the corresponding method templates of the task class template. Note that the
 *    exception handling tasks are normally not sheduled for execution; they get
 *    scheduled only if an eception was thrown in one of the preceding tasks
 *    and is caught by the exception handling task.
 *
 * The compound tasks are internally organized in one of two possible ways:
 *  - @em parallel organization; when such compund task is activated it will
 *    immediatelly schedule all of its subtasks for execution with their respective
 *    @ref runner "runners". Note that should two or more subtasks be associated
 *    with the the same @ref runner, they will still get scheduled immediatelly but
 *    the scheduling order in which they are passed to the @ref runner is not
 *    defined. The parallel task remains active until the
 *    last of its subtasks completes the execution. Then it collects the results
 *    of all subtasks, if any, and returns the collection as its result.
 *    Parallel compound tasks are created by using @ref taskop
 *    method template @ref taskop::parallel "parallel()" or the corresponding method
 *    template of the task class template.
 *
 *  - @em sequential organization; when such compound tasks is activated, only
 *    the first task in the sequence is scheduled for execution; when this task
 *    task is completed, its result, if any, is passed as an input parameter to
 *    the next task in the sequence, which is then scheduled for execution with
 *    its associated runner, and so on. The sequential compound tasks remains
 *    activa until the last task in the sequence completes its execution, when
 *    it collects the result of the last task, if any, and returns it as its
 *    own result.  Sequential compound tasks are created via a call to @ref taskop
 *    method template @ref taskop::sequential "sequential()" or the corresponding method
 *    template of the task class template.
 * 
 * Every task either no input parameters or exactly one input parameter and a 
 * result type, which can be @c void if the task produces no results. For simple
 * tasks the presence or absence of the input parameter and its type, as well as
 * the task's result type (or @c void if none) are deduced from its @em Callable 
 * passed to the @ref taskop::create method template.
 *
 * The presence of the <em>input parameter</em> and its type of the compound task are
 * deduced from the first task in the parameter list passed to either
 * taskop:: sequential or taskop::parallel method. Thus by definition the
 * the input paramter presence and its type of the sequential compound task
 * matches the input parameter of the first task in the sequence, and when the
 * sequential compound task is activated, it will pass its input parameter, if
 * any, to the first task in the sequence. If the compound task is a parallel task,
 * all of its other subtasks must match the input parameter of the first task
 * passed to the taskop::parallel method template, and when parallel compound task is
 * activated it will pass its input parameter to all of its subtasks.
 *
 * The <em>return value</em> type of the sequential compound task is deduced from the
 * last subtask in the sequence (last task parameter passed to taskop::sequential).
 * The return value type of the parallel compound task is defined to be
 * <tt>std::tuple<t1, t2, t3 ...></tt> where @c t1, @c t2, @c t3 ... are the
 * result types of its subtasks in the exact order as passed to taskop::parallel.
 * If a subtask of the parallel compound task does not return result
 * (result type @c void), a type <tt>void*</tt> is used in its place in the
 * <tt>std::tuple</tt> template parameter list. This element of the tuple is
 * never set, it is just used as a placeholder to enable the instatination of the
 * result tuple.
 *
 * The <em>exception handling tasks</em> that are to intercept and handle exceptions
 * of a particular type must accept the parameter of this type (or better, a
 * const reference to avoid object slicing). The exception handling tasks that
 * are to intercept and handle any exception must accept <tt>std::exception_ptr</tt> as their
 * parameter. The result type of the exception handling task must match the
 * the result type of the task to which they are attached (preceding task).
 *
 * <b>Handling of Exceptions</b>
 *
 * The way the task compositions (compount tasks) handle the exceptions has been
 * modeled to follow the C++ @c catch paradigm as close as possible, having in
 * mind that the simple tasks exectute their @em Callables asynchronously. This
 * means that an exception thrown in one @em Callable cannot be simply caught
 * in another @em Callable since this another  @em Callable may execute some time
 * in the future or even has already completed its execution. This is where the
 * <em>exception handling</em> taks enter the picture. When a currently executing 
 * @em Callable throws a C++ exception, which remains unhandled when the execution
 * of the throwing callable completes, the task enclosing the @em Callable that
 * threw will intercept the exception and pass it on to the next task in the
 * sequence. If task is not designated to handle this exception type it will
 * pass it on to the next task, and so on, until the exception reaches the
 * eception handling task that will accept this, or any, exception type. If
 * no matching exception handling task is found the exception will be silently
 * and irrevocably discarded. The following is the full set of rules that govern
 * the handling of exceptions:
 *   - an exception thrown in a task which is not part of the @em sequential 
 *     compound task, and is not either caught or intercepted by this task, is 
 *     irrevocably lost
 *   - an exception thrown inside a subtask of the @em sequential task, and not
 *     intercepted within this subtask, will travel along the sequence until it
 *     reaches an appropriate <em>exception handling</em> subtask. If no appropriate
 *     exeption handling subtask is reached before the end of the sequence the
 *     exception will exit the compound task and will be passed on to the next
 *     task in the sequence, if any. <em>None of the tasks in sequence between 
 *     the task that threw the exception and the task that intercepted it, or
 *     the end of the sequence, will be scheduled for execution</em>.
 *   - an exception thrown inside a subtask of the @em parallel task will
 *     affect this subtask only. The remaining subtasks of the parallel compound
 *     task will run to their completion. Then the compound task will pass the
 *     exception on to the next task in the sequence, if any.
 *
 */
template <typename TagT, typename ResultT, typename ParamT>
class task
{
 public:
  using this_t       = task;
  using result_t     = ResultT;
  using parameter_t  = ParamT;
  using tag          = TagT;

 public:

  /**
   * Create a compound task which will concurrently execute its subtasks.
   *
   * Using this method in the following code fragment:
   * @code
   *   auto new_task = task.parallel(task1, task2, task3);
   * @endcode
   * is exactly the same as using:
   * @code
   *   auto new_task = taskop::parallel(task, task1, task2, task3);
   * @endcode
   * This method is provided for convenience to allow chaining the
   * task operations such as:
   * @code
   *   task.parallel(task1, task2).intercept(handler).run();
   * @endcode
   * @warning This method invalidates @c this task object.
   * @see taskop::parallel
   * @see @ref sequential
   * @see taskop::sequential
   */
  template <typename... TaskT>
  task<impl::tag::parallel
     , typename impl::traits::parallel_result<this_t, TaskT...>::type
     , parameter_t>
  parallel(TaskT&&... tasks);

  /**
   * Create a compound task which will cexecute its subtasks one after another.
   *
   * Using this method in the following code fragment:
   * @code
   *   auto new_task = task.sequential(task1, task2, task3);
   * @endcode
   * is exactly the same as using:
   * @code
   *   auto new_task = taskop::sequential(task, task1, task2, task3);
   * @endcode
   * This method is provided for convenience to allow chaining the
   * task operations such as:
   * @code
   *   task.sequential(task1, task2).intercept(handler).run();
   * @endcode
   * @warning This method invalidates @c this task object.
   * @see taskop::parallel
   * @see @ref parallel
   * @see taskop::sequential
   */
  template <typename... TaskT>
  task<impl::tag::serial
     , typename impl::traits::sequence_result<this_t, TaskT...>::type
     , parameter_t>
  sequential(TaskT&&... tasks);

  /**
   * Add an exception handler(s) for specific exception(s) to the task.
   *
   * Using this method in the following code fragment:
   * @code
   *   auto new _task = task.intercept(handler1, handler2);
   * @endcode
   * is exactly the same as using:
   * @code
   *   auto new _task = taskop::intercept(task, handler1, handler2);
   * @endcode
   * This method is provided for convenience to allow chaining the
   * task operations, such as:
   * @code
   *   task.intercept(handler).parallel(task2).run();
   * @endcode
   * @warning This method invalidates @c this task object.
   * @see taskop::intercept
   * @see @ref intercept_all
   * @see taskop::intercept_all
   */
  template <typename... HandlerTaskT>
  task <impl::tag::serial, ResultT, ParamT>
  intercept(HandlerTaskT&&... handlers);

  /**
   * Add an exception handler for any exception to the task.
   *
   * Using this method in the following code fragment:
   * @code
   *   auto new _task = task.intercept_all(handler);
   * @endcode
   * is exactly the same as using:
   * @code
   *   auto new _task = taskop::intercept_all(task, handler);
   * @endcode
   * This method is provided for convenience to allow chaining the
   * task operations, such as:
   * @code
   *   task.intercept_all(handler).parallel(task2).run();
   * @endcode
   * @warning This method invalidates @c this task object.
   * @see taskop::intercept
   * @see @ref intercept
   * @see taskop::intercept_all
   */
  template <typename HandlerTaskT>
  task <impl::tag::serial, ResultT, ParamT>
  intercept_all(HandlerTaskT&& handler);

  /**
   * Schedule a task for execution by its runner.
   *
   * If the @ref task is a simple task, this method will sumbit its
   * @em Callable to its associated @ref runner for execution. If the task
   * is a compound task, it will activate it. The activation means that the
   * compound task will start submitting its subtasks to their respective
   * @ref runner "runners" for execution.
   *
   * @param param_ the parameter forwarded to the task.
   */
  void run(parameter_t&& param_)
  {
    if (m_impl == nullptr)
      throw exception::illegal_state("this task object is not valid");

    entrails::kick(impl::task_factory<ResultT, ParamT, TagT>::create_context(m_impl, param_));
  }

 private:
  friend class taskop;
  template <typename A, typename B, typename C>
  friend class impl::task_factory;
  task(const impl::taskinfo_ptr& arg_) : m_impl(arg_)
  { /* noop */ }

 private:
  impl::taskinfo_ptr m_impl;
};

// ------ task specialization for tasks with no input parameter
//
template <typename TagT, typename ResultT>
class task<TagT, ResultT, void>
{
 public:
  using this_t       = task;
  using result_t     = ResultT;
  using parameter_t  = void;

 public:

  template <typename... TaskT>
  task<impl::tag::parallel
      , typename impl::traits::parallel_result<this_t, TaskT...>::type
      , parameter_t>
  parallel(TaskT&&... tasks);

  template <typename... TaskT>
  task<impl::tag::serial
     , typename impl::traits::sequence_result<this_t, TaskT...>::type
     , parameter_t>
  sequential(TaskT&&... tasks);

  template <typename... HandlerTaskT>
  task<impl::tag::serial, ResultT, void>
  intercept(HandlerTaskT&&... handlers);

  template <typename HandlerTaskT>
  task<impl::tag::serial, ResultT, void>
  intercept_all(HandlerTaskT&& handler);

  void run()
  {
    entrails::kick(impl::task_factory<ResultT, void, TagT>::create_context(m_impl));
  }

 private:
  friend class taskop;
  template <typename A, typename B, typename C>
  friend class impl::task_factory;
  task(const impl::taskinfo_ptr& arg_) : m_impl(arg_)
  { /* noop */ }

 private:
  impl::taskinfo_ptr m_impl;
};

/**
 * Class implementing the operations on tasks.
 */
class taskop
{
 public:
  /**
   * Create a simple task.
   *
   * Creates and returns a simple task associated with the @ref runner. When
   * activated via @ref task::run() method, the task will submit its @em Callable for
   * execution with the associated runner. The @em Callable must satisfy the
   * following requirements:
   *
   *   - it can be a function pointer, lambda closure, @c std::function, or
   *     a functor object as long as it provides a single overload of the
   *     function call operator <tt>()</tt>.
   *   - it must accept <tt>const runner::weak_ptr>&</tt> as its first
   *     parameter
   *   - it may accept one additional input parameter which is then the input
   *     parameter of the task
   *   - it may, but does not need to, return a value of any type.
   *
   * The @em Callable implementation should not contain any blocking code. The
   * blocking code will block the worker thread executing the callable. Since
   * the number of working threads in the pool may be limited (optimal number
   * should be close to the number of precessor cores to avoid taskinfo switches)
   * the pool may run out of the active, non-blocked threads, will will block
   * the execution of the entire program. The @em Callable implementation should
   * use I/O event sources instead of blocking read/write calls, and should
   * organize the data around runners instead of using the thread synchronization
   * primitives.
   *
   * The @em Callable may return result. The result value will be passed to the
   * next task in the sequence as an input parameter. If the next task in the
   * sequence us the parallel compound task. the result will be passed to every
   * subtask of this compund task.
   *
   * The @em Callable may throw a C++ exception. The unhandled exception will
   * travel along the sequence of tasks until it reaches the exception handling
   * task that can interept it. None of the tasks between the throwing task and
   * the interception task will be scheduled for execution.
   *
   * @param r_ the @ref runner which will be used to run the @em Callable
   * @param f_ @em Callable to execute
   *
   * @note Unfortunately the objects returned by the @c std::bind template
   *   provide multiple function call operator overloads and cannot be passed
   *   to the simple task directly, without transforming them into @c std::function
   *   object first.
   *
   * @warning The @em Callable should not contain blocking code.
   */
  template <typename CallableT>
  static task<impl::tag::simple
            , typename impl::traits::function_traits<CallableT>::result_type
            , typename impl::traits::arg_type<1, CallableT>::type>
  create(const runner::weak_ptr& r_, CallableT&& f_)
  {
    // check number of parameters and the type of the first parameter
    static_assert(
        2 >= impl::traits::function_traits<CallableT>::arity::value && 0 < impl::traits::function_traits<CallableT>::arity::value
      , "Callable with signature RetT (const runner::ptr& [, argument]) is required to construct a task");
    static_assert(
        std::is_same<
            runner::ptr
          , typename std::decay<typename impl::traits::function_traits<CallableT>::template arg<0>::type>::type>
        ::value
      , "Callable with signature RetT (const runner::ptr& [, argument]) is required to construct a task");

    using result_t = typename impl::traits::function_traits<CallableT>::result_type;
    using param_t  = typename impl::traits::arg_type<1, CallableT>::type;

    return task<impl::tag::simple, result_t, param_t>(
        impl::task_factory<result_t, param_t, impl::tag::simple>::create(r_, f_));
  }

  /**
   * Create a compound task which will concurrently execute its subtasks.
   *
   * Creates a compound task which will, when activated, schedule all its subtasks
   * for execution with their respective @ref runner "runners" concurrently
   * and will collect the results of their executions. Whether the subtrasks will
   * actually run concurrently depends on the runners they'll use and on the
   * availability of idle threads in the worker pool. The parallel compound task
   * will defer the completion of its execution until all subtasks are
   * finished and will return the results of all subtasks as its result.
   *
   * All subtasks @c t_ must accept the input parameter of the same type, or no
   * input paramter. The input paramter of the parallel compound task is the
   * same as the input parameter of its subtasks. When activated, the parallel
   * compund task will pass its input parameter to all its subtasks.
   *
   * The result of the parallel compund task is an union of the results of its
   * subtasks, implemented using the @c std::tuple template. The position of the
   * subtasks result in the tuple is the same as the position of the subtask
   * in the parameter list. For subtasks that do not return the result the
   * tuple will contain element of the type <tt>void*</tt> which value is not
   * defined.
   *
   * @param t_ two or more subtasks to be scheduled to run concurrently.
   *
   * @note Subtasks of the parallel compound task are <em>placed into the
   *       task queues</em> of their runners concurrently. This does not
   *       guarantee concurrent execution of the subtasks.
   * @note The method consumes all tasks passed as parameters @c t_. After the
   *       completion of this call they are all invalidated.
   * @warning The parallel compound task may find one or more subtasks impossible to
   *       schedule as their runners may no longer exist. In such case the
   *       compound task will behave as if the task that could not have been
   *       run threw @ref runner_not_available exception.
   */
#if 0
  template <typename... TaskT>
  static task<impl::tag::parallel
            , typename impl::traits::parallel_result<TaskT...>::type
            , typename impl::traits::first_task<TaskT...>::type::parameter_t>
  parallel(TaskT&&... t_)
  {
    using result_t = typename impl::traits::parallel_result<TaskT...>::type;
    using param_t = typename impl::traits::first_task<TaskT...>::type::parameter_t;

    static_assert(
        impl::traits::all_same<typename std::decay<typename std::decay<TaskT>::type::parameter_t>::type...>::value
      , "All parallel tasks must have the same parameter type or no parameter");

    return task<impl::tag::parallel, result_t, param_t>(
        impl::task_factory<impl::tag::parallel, result_t, param_t>::create());
  }
#endif
  /**
   * Create a compound task which will execute its subtasks one after another.
   *
   * Creates a compound task which will, when executed, schedule its first subtask
   * for execution, wait for it completion, then schedule the next task
   * for execution, and so on, until the end of the sequence is reached. The
   * order of task execution is from the left to the right with respect to
   * the subtask positions in the parameter list.
   *
   * The inout parameter of the compound task is the same as the input parameter
   * of the first, the leftmost, task. When activated the compound task will
   * pass its parameter to the first subtask. The type of the input parameter of the
   * next subtask task must match the result type of the preeding subtask task.
   * Upon the preceding task completion its result will be passed to the next
   * task as the input parameter.
   *
   * The result of the sequential compund task is the result of its last,
   * the righmost, subtask.
   *
   * @param t_ two or more subtasks to be scheduled to run one after another.
   *
   * @note The method consumes all tasks passed as parameters @c t_. After the
   *       completion of this call they are all invalidated.
   * @warning The compound task may find the subtask to be run impossible
   *       to schedule as its runner may no longer exist. In such case the
   *       compound task will behave as if the task that could not have been
   *       run threw @ref runner_not_available exception.
   */
  template <typename... TaskT>
  static task<impl::tag::serial
            , typename impl::traits::sequence_result<TaskT...>::type
            , typename impl::traits::first_task<TaskT...>::type::parameter_t>
  sequential(const runner::weak_ptr& r_, TaskT&&... t_)
  {
    using result_t = typename impl::traits::sequence_result<TaskT...>::type;
    using param_t = typename impl::traits::first_task<TaskT...>::type::parameter_t;

    static_assert(
        impl::traits::all_chained<typename std::decay<TaskT>::type...>::result::value
      , "The type of the parameter of each task in the sequence must match the return type of the preceding task.");
    return task<impl::tag::serial, result_t, param_t>(
        impl::task_factory<result_t, param_t, impl::tag::serial>::create(r_, std::forward<TaskT>(t_)...));
  }
  /**
   * Add an exception handler(s) to the task.
   */
#if 0
  template <typename TaskT, typename... HandlerTaskT>
  static task<impl::tag::serial, typename TaskT::result_t, typename TaskT::param_t>
  intercept(TaskT&& t_, HandlerTaskT&&... handlers)
  {
    using result_t = typename TaskT::result_t;
    using param_t = typename TaskT::parameter_t;

    static_assert(
        impl::traits::all_same<typename TaskT::result_t, typename HandlerTaskT::result_t...>::value
      , "The task and its exception handlers must have the same result type");
    return task<impl::tag::serial, result_t, param_t>(
        impl::task_factory<impl::tag::serial, result_t, param_t>::create());
  }
#endif
  /**
   * Add an exception handler to the task.
   */
#if 0
  template <typename TaskT, typename HandlerTaskT>
  static task<impl::tag::serial, typename TaskT::result_t, typename TaskT::param_t>
  intercept_all(TaskT&& t_, HandlerTaskT&& handler)
  {
    using result_t = typename TaskT::result_t;
    using param_t = typename TaskT::parameter_t;

    static_assert(
        std::is_same<typename TaskT::result_t, typename HandlerTaskT::result_t>::value
      , "The task and its exception handlers must have the same result type");
    static_assert(
        std::is_same<std::exception_ptr, typename std::decay<typename HandlerTaskT::parameter_t>::type>::value
      , "The exception handler to catch all exceptions must accept std::exception_ptr as its parameter");
    return task<impl::tag::serial, result_t, param_t>(
        impl::task_factory<impl::tag::serial, result_t, param_t>::create());
  }
#endif
};

// ---------------------------------------------------------------------------
// implementation of task methods
#if 0
template <typename TagT, typename ResultT, typename ParameterT>
template <typename... HandlerTaskT>
inline task<impl::tag::serial, ResultT, ParameterT>
task<TagT, ResultT, ParameterT>::intercept(HandlerTaskT&&... handlers)
{
  return taskop::intercept(*this, std::forward<HandlerTaskT>(handlers)...);
}

template <typename TagT, typename ResultT, typename ParameterT>
template <typename HandlerTaskT>
inline task<impl::tag::serial, ResultT, ParameterT>
task<TagT, ResultT, ParameterT>::intercept_all(HandlerTaskT&& handler)
{
  return taskop::intercept_all(*this, std::forward<HandlerTaskT>(handler));
}

template <typename TagT, typename ResultT, typename ParameterT>
template <typename... TaskT>
inline task<impl::tag::parallel
          , typename impl::traits::parallel_result<task<TagT, ResultT, ParameterT>, TaskT...>::type
          , ParameterT>
task<TagT, ResultT, ParameterT>::parallel(TaskT&&... tasks)
{
  return taskop::parallel(*this, std::forward<TaskT>(tasks)...);
}
#endif
template <typename TagT, typename ResultT, typename ParameterT>
template <typename... TaskT>
inline task<impl::tag::serial
          , typename impl::traits::sequence_result<task<TagT, ResultT, ParameterT>, TaskT...>::type
          , ParameterT>
task<TagT, ResultT, ParameterT>::sequential(TaskT&&... tasks)
{
  return taskop::sequential(m_impl->get_runner(), *this, std::forward<TaskT>(tasks)...);
}
#if 0
// ---- implementation of task methods for void partial specialization

template <typename TagT, typename ResultT>
template <typename... HandlerTaskT>
inline task<impl::tag::serial, ResultT, void>
task<TagT, ResultT, void>::intercept(HandlerTaskT&&... handlers)
{
  return taskop::intercept(*this, std::forward<HandlerTaskT>(handlers)...);
}

template <typename TagT, typename ResultT>
template <typename HandlerTaskT>
inline task<impl::tag::serial, ResultT, void>
task<TagT, ResultT, void>::intercept_all(HandlerTaskT&& handler)
{
  return taskop::intercept_all(*this, std::forward<HandlerTaskT>(handler));
}

template<typename TagT, typename ResultT>
template <typename... TaskT>
inline task<impl::tag::parallel
          , typename impl::traits::parallel_result<task<TagT, ResultT, void>, TaskT...>::type
          , void>
task<TagT, ResultT, void>::parallel(TaskT&&... tasks)
{
  return taskop::parallel(*this, std::forward<TaskT>(tasks)...);
}
#endif
template<typename TagT, typename ResultT>
template <typename... TaskT>
inline task<impl::tag::serial
          , typename impl::traits::sequence_result<task<TagT, ResultT, void>, TaskT...>::type
          , void>
task<TagT, ResultT, void>::sequential(TaskT&&... tasks)
{
  return taskop::sequential(m_impl->get_runner(), *this, std::forward<TaskT>(tasks)...);
}
#endif
} } } // namespace

#endif
