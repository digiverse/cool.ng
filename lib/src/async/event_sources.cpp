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

#include "cool/ng/error.h"
#include "cool/ng/exception.h"

#include "cool/ng/impl/async/net_types.h"
#include "cool/ng/async/event_sources.h"

#if defined(COOL_ASYNC_PLATFORM_GCD)
# include "gcd/event_sources.h"
#elif defined(COOL_ASYNC_PLATFORM_WINCP)
# include "wincp/event_sources.h"
#else
# error "unknown asynchronous platform - only supported are GCD and Windows completion ports"
#endif

// ==========================================================================
// ======
// ======
// ====== Event sources code common to both GCD and WINCP platforms.
// ======
// ======
// ==========================================================================

namespace cool { namespace ng { namespace async {

using cool::ng::error::no_error;

namespace exc = cool::ng::exception;
namespace ip = cool::ng::net::ip;
namespace ipv4 = cool::ng::net::ipv4;
namespace ipv6 = cool::ng::net::ipv6;

void timer::start()
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  m_impl->start();
}

void timer::stop()
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  m_impl->stop();
}

void  timer::period(uint64_t p_, uint64_t l_)
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  m_impl->period(p_, l_);
}

const std::string& timer::name() const
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  return m_impl->name();
}

timer::operator bool() const
{
  return !!m_impl;
}

namespace impl {
// --------------------------------------------------------------------------
// -----
// ----- Factory methods
// ------

dlldecl std::shared_ptr<detail::itf::timer> create_timer(
    const std::shared_ptr<runner>& r_
  , const std::weak_ptr<cb::timer>& t_
  , uint64_t p_
  , uint64_t l_)
{
  auto ret = cool::ng::util::shared_new<timer>(t_, p_, l_);
  ret->initialize(r_->impl());
  return ret;
}

} // namespace impl

namespace net {

void server::start()
{
  m_impl->start();
}

void server::stop()
{
  m_impl->stop();
}

const std::string& server::name() const
{
  return m_impl->name();
}

const std::string& stream::name() const
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  return m_impl->name();
}

void stream::write(const void* data_, std::size_t size_)
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  m_impl->write(data_, size_);
}

void stream::connect(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  m_impl->connect(addr_, port_);
}

void stream::disconnect()
{
  if (!*this)
    throw cool::ng::exception::empty_object();
  m_impl->disconnect();
}

stream::operator bool() const
{
  return !!m_impl;
}

namespace impl {

// --------------------------------------------------------------------------
// -----
// ----- Factory methods
// ------

std::shared_ptr<cool::ng::async::net::detail::startable> create_server(
    const std::shared_ptr<runner>& r_
  , const ip::address& addr_
  , uint16_t port_
  , const cb::server::weak_ptr& cb_)
{
  auto ret = cool::ng::util::shared_new<server>(r_->impl(), cb_);
  ret->initialize(addr_, port_);
  return ret;
}

std::shared_ptr<detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& r_
  , const cool::ng::net::ip::address& addr_
  , uint16_t port_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_)
{
  auto ret = cool::ng::util::shared_new<stream>(r_->impl(), cb_);
  ret->initialize(addr_, port_, buf_, bufsz_);
  return ret;
}

std::shared_ptr<detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& r_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_)
{
  auto ret = cool::ng::util::shared_new<stream>(r_->impl(), cb_);
  ret->initialize(buf_, bufsz_);
  return ret;
}


} } } } }


