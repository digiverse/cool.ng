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

#include <iostream>
#include <typeinfo>
#include <memory>
#include <stack>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

#define BOOST_TEST_MODULE Error

#include "unit_test_common.h"

#include "cool/ng/error.h"


namespace error = cool::ng::error;
using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(error_)

COOL_AUTO_TEST_CASE(T001,
  *utf::description("is error"))
{
  {
    auto erc = error::make_error_code(error::errc::not_an_error);
    BOOST_CHECK(!erc);
  }
  {
    auto erc = error::make_error_code(error::errc::not_set);
    BOOST_CHECK(erc);
  }
}

COOL_AUTO_TEST_CASE(T002,
  *utf::description("enumeration test for error code/condition"))
{
  BOOST_CHECK(std::is_error_code_enum<error::errc>());
  BOOST_CHECK(std::is_error_condition_enum<error::errc>());
}

COOL_AUTO_TEST_CASE(T003,
  *utf::description("error condition"))
{
  {
    auto erc = error::make_error_code(error::errc::not_set);
    auto ec = erc.default_error_condition();

    BOOST_CHECK_EQUAL(ec.message(), erc.message());
  }
  {
    auto erc = error::make_error_code(error::errc::not_set);
    auto ec = erc.default_error_condition();

    BOOST_CHECK(erc.category().equivalent(erc.value(), ec));
    BOOST_CHECK(erc.category().equivalent(erc, ec.value()));
  }
}


BOOST_AUTO_TEST_SUITE_END()
