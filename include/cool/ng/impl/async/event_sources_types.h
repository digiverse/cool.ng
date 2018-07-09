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

#if !defined(cool_ng_f36defb0_bb34_4ce1_b25a_943f5deed23a)
#define      cool_ng_f36defb0_bb34_4ce1_b25a_943f5deed23a

#include <memory>
#include <cstdint>
#include <functional>
#include <system_error>

#include "cool/ng/ip_address.h"
#include "cool/ng/async/runner.h"

namespace cool { namespace ng { namespace async {


namespace detail {

// --- ============================================
// --- Common interfaces for implementation classes
//
namespace itf {

// event source generic interface
class event_source
{
 public:
  virtual ~event_source() { /* noop */ }
  virtual const std::string& name() const = 0;
  virtual void shutdown() = 0;
};

//--- startable event source interface
class startable : public event_source
{
 public:
  virtual void start() = 0;
  virtual void stop() = 0;
};

//--- timer event source interface
class timer : public startable
{
 public:
  virtual void period(uint64_t, uint64_t) = 0;
};

//--- writable event source interface
class writable : public event_source
{
 public:
  virtual void write(const void* data, std::size_t sz) = 0;
};

} }  // namespace detail::itf


namespace impl {
namespace cb {

class timer
{
 public:
  virtual ~timer() { /* noop */}
  virtual void expired() = 0;
};


} // namespace cb

// --- ---------------------------------------------
// ---
// --- Factories
// ---

dlldecl std::shared_ptr<detail::itf::timer> create_timer(
    const std::shared_ptr<runner>& r_
  , const std::weak_ptr<cb::timer>& t_
  , uint64_t p_
  , uint64_t l_
);

} // namespace impl

// --- ============================================
// --- Network event sources
//
namespace net  {

class stream;

namespace detail {

enum class oob_event { connect, disconnect, failure, internal };

namespace ip = cool::ng::net::ip;


template <typename T>
class types
{
 public:
  using ptr = std::shared_ptr<T>;
  // types required by server
  using stream_factory = std::function<cool::ng::async::net::stream(const ptr&, const ip::address&, uint16_t)>;
  using connect_handler = std::function<void(const ptr&, const cool::ng::async::net::stream&)>;
  using error_handler = std::function<void(const ptr&, const std::error_code&)>;

  // types required by stream
  using write_handler = std::function<void(const ptr&, const void*, std::size_t)>;
  using read_handler  = std::function<void(const ptr&, void*&, std::size_t&)>;
  using event_handler = std::function<void(const ptr&, oob_event, const std::error_code&)>;

};

namespace itf {

//--- connected writable event source interface
class connected_writable : public async::detail::itf::writable
{
 public:
  virtual void connect(const ip::address&, uint16_t) = 0;
  virtual void disconnect() = 0;
  virtual void set_handle(cool::ng::net::handle h_) = 0;

};

} // namespace itf

} // namespace detail


namespace impl {

using cool::ng::net::handle;
namespace ip = cool::ng::net::ip;

// callback implementation interfaces
namespace cb {

// callback interface required by the implementation of the TCP server
class server
{
 public:
  using weak_ptr = std::weak_ptr<server>;
  using ptr = std::shared_ptr<server>;

 public:
  virtual ~server() { /* noop */ }
  virtual cool::ng::async::net::stream manufacture(const ip::address&, uint16_t) = 0;
  virtual void on_connect(const cool::ng::async::net::stream&) = 0;
  virtual void on_event(const std::error_code&) = 0;
};

// --- callback interface required by the implementation of the TCP stream
class stream
{
 public:
  using weak_ptr = std::weak_ptr<stream>;
  using ptr = std::shared_ptr<stream>;


 public:
  virtual ~stream() { /* noop */ }
  virtual void on_read(void*&, std::size_t&) = 0;
  virtual void on_write(const void*, std::size_t) = 0;
  virtual void on_event(detail::oob_event, const std::error_code&) = 0;
};

} // namespace cb

// factories for implementation classes

dlldecl std::shared_ptr<async::detail::itf::startable> create_server(
    const std::shared_ptr<runner>& r_
  , const cool::ng::net::ip::address& addr_
  , uint16_t port_
  , const cb::server::weak_ptr& cb_);

dlldecl std::shared_ptr<detail::itf::connected_writable> create_stream(
    const std::shared_ptr<runner>& runner_
  , const cool::ng::net::ip::address& addr_
  , uint16_t port_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_);
dlldecl std::shared_ptr<detail::itf::connected_writable> create_stream(
    const std::shared_ptr<runner>& runner_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_);


} // namespace impl

} } } } // namespace

#endif
