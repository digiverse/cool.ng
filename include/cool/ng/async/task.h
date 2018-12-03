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
 *    <tr><td><tt>input_type</tt>      <td>type of the second arg to @em Callable, @c void if none
 *    <tr><td><tt>result_type</tt>     <td>return type of @em Callable
 *  </table>
 *
 * The following requirements are imposed on the user @em callable:
 *   - the first input parameter to @em Callable must be of type <tt>const std::shared_ptr<decltype(@em runner)>&</tt>.
 *   - the user @em Callable may accept one or two input parameters
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
 *    <tr><td><tt>input_type</tt>      <td>decltype(@em task_1)::%input_type
 *    <tr><td><tt>result_type</tt>     <td>decltype(@em task_n)::%result_type
 *  </table>
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

template <typename InputT, typename ResultT>
class task
{
 public:
  using this_type   = task;
  using input_type  = InputT;
  using result_type = ResultT;
  using impl_type   = detail::base::taskinfo<input_type, result_type>;

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
    , T>::type>::type& arg_) const
  {
    m_impl->run(m_impl, arg_);
  }

  // rvalue reference
  template <typename T = InputT>
  void run(typename std::enable_if<
      !std::is_same<T, void>::value && std::is_rvalue_reference<T>::value
    , T>::type arg_) const
  {
    m_impl->run(m_impl, std::move(arg_));
  }
 /**
  * Schedule task for execution.
  */
  template <typename T = InputT>
  typename std::enable_if<std::is_same<T, void>::value, void>::type run() const
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
      typename traits::arg_type<1, CallableT>::type
    , typename traits::functional<CallableT>::result_type
  > create(const std::weak_ptr<RunnerT>& r_, const CallableT& f_)
  {
    using result_type = typename traits::functional<CallableT>::result_type;
    using input_type = typename traits::arg_type<1, CallableT>::type;
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::simple, RunnerT, input_type, result_type>;

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

    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(r_, f_)));
  }

  template <typename RunnerT, typename CallableT>
  inline static task<
      typename traits::arg_type<1, CallableT>::type
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
      typename detail::traits::get_first<TaskT...>::type::input_type
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
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::sequential, detail::default_runner_type, input_type, result_type>;

    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(t_.m_impl...)));
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
      typename TryT::input_type
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
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::intercept, detail::default_runner_type, input_type, result_type>;

    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(t_.m_impl, c_.m_impl...)));
  }

  template <typename PredicateT, typename IfT, typename ElseT>
  inline static task<
      typename PredicateT::input_type
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
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::conditional, detail::default_runner_type, input_type, result_type>;

    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(p_.m_impl, if_.m_impl, else_.m_impl)));
  }

  template <typename PredicateT, typename IfT>
  inline static task<
      typename PredicateT::input_type
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
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::conditional, detail::default_runner_type, input_type, result_type>;

    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(p_.m_impl, if_.m_impl)));
  }

  template <typename TaskT>
  inline static task<
      std::size_t
    , typename TaskT::result_type
  > repeat( const TaskT t_)
  {
    using result_type = typename TaskT::result_type;
    using input_type = std::size_t;
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::repeat, detail::default_runner_type, input_type, result_type>;

    static_assert(
        std::is_same<typename TaskT::input_type, std::size_t>::value
      , "The task to be repeated must have input_type std::size_t.");
    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(t_.m_impl)));
  }


  template <typename PredicateT, typename BodyT>
  inline static task<
      typename PredicateT::input_type
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
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::loop, detail::default_runner_type, input_type, result_type>;

    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(p_.m_impl, body_.m_impl)));
  }

  template <typename PredicateT>
  inline static task<
      void
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
    using task_type = task<input_type, result_type>;
    using taskinfo_type = detail::taskinfo<tag::loop, detail::default_runner_type, input_type, result_type>;

    return task_type(std::shared_ptr<typename task_type::impl_type>(new taskinfo_type(p_.m_impl)));
  }

};


} } } // namespace

#endif
