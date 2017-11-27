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

#if !defined(cool_ng_f36defb0_abba_4ce1_b25a_943f5beef23a)
#define      cool_ng_f36defb0_abba_4ce1_b25a_943f5beef23a

#include <atomic>
#include <memory>
#include <functional>

#include <dispatch/dispatch.h>
#include "cool/ng/bases.h"
#include "cool/ng/ip_address.h"
#include "cool/ng/impl/async/event_sources.h"

#include "executor.h"

namespace cool { namespace ng { namespace async {


namespace net { namespace impl {

// tiny wrapper around dispatch source libdispatch functions - also
// prevents unbalanced resume/suspend calls
class dispatch_source
{
 public:
  dispatch_source(const dispatch_source&) = delete;
  dispatch_source(dispatch_source&&) = delete;
  void operator =(const dispatch_source&) = delete;
  void operator =(dispatch_source&&) = delete;

  dispatch_source() : m_active(false), m_source(nullptr)
  { /* noop */ }
  dispatch_source(::dispatch_source_t s_) : m_active(false), m_source(s_)
  { /* noop */ }
  dispatch_source& operator =(::dispatch_source_t s_)
  {
    m_source = s_;
    return *this;
  }
  void context(void* context) const
  {
    ::dispatch_set_context(m_source, context);
  }
  void cancel_handler(::dispatch_function_t handler) const
  {
    ::dispatch_source_set_cancel_handler_f(m_source, handler);
  }
  void event_handler(::dispatch_function_t handler) const
  {
    ::dispatch_source_set_event_handler_f(m_source, handler);
  }
  unsigned long get_data() const
  {
    return ::dispatch_source_get_data(m_source);
  }
  void resume()
  {
    bool ex = false;
    if (m_active.compare_exchange_strong(ex, true))
      ::dispatch_resume(m_source);
  }
  void suspend()
  {
    bool ex = true;
    if (m_active.compare_exchange_strong(ex, false))
      ::dispatch_suspend(m_source);
  }
  void cancel()
  {
    ::dispatch_source_cancel(m_source);
  }
  void release()
  {
    ::dispatch_release(m_source);
  }
 private:
  std::atomic<bool>   m_active;
  ::dispatch_source_t m_source;
};

class server : public cool::ng::async::detail::startable
             , public cool::ng::util::named
{
 public:
  server(const std::shared_ptr<async::impl::executor>& ex_
       , const cool::ng::net::ip::address& addr_
       , int port_
       , const cb::server::weak_ptr& cb_);

  void start() override;
  void stop() override;
  void shutdown() override;
  const std::string& name() const override { return named::name(); }

 private:
  void init_ipv4(const cool::ng::net::ip::address& addr_, int port_);
  void init_ipv6(const cool::ng::net::ip::address& addr_, int port_);
  static void on_cancel(void *ctx);
  static void on_event(void *ctx);

 private:
  dispatch_source         m_source;
  ::cool::ng::net::handle m_handle;
  cb::server::weak_ptr    m_handler;
};

/*
 * The stream implementation class is kept alive by three shared pointers:
 *   - shared pointer of its parent, detail::stream class template
 *   - shared pointers of contexts of dispatch queue read and write
 *     event sources
 * Note that the stream implementation does not manage the life time of event
 * source contexts - these will get deleted  through their cancel callbacks. So
 * in a sense they co-manage the life time of the stream implementation.
 */
class stream : public cool::ng::async::detail::connected_writable
             , public cool::ng::util::named
             , public cool::ng::util::self_aware<stream>
{
  enum class state { starting, connecting, connected, disconnected };

  struct context
  {
    ::cool::ng::net::handle m_handle;
    dispatch_source         m_source;
    stream::ptr             m_stream;
  };
  struct rd_context : public context
  {
    void*                   m_rd_data;
    std::size_t             m_rd_size;
    bool                    m_rd_is_mine;
  };

 public:
  stream(const std::weak_ptr<async::impl::executor>& ex_
       , const cb::stream::weak_ptr& cb_);
  ~stream();

  void initialize(const cool::ng::net::ip::address& addr_
                , uint16_t port_
                , void* buf_
                , std::size_t bufsz_);
  void initialize(cool::ng::net::handle h_, void* buf_, std::size_t bufsz_);
  void initialize(void* buf_, std::size_t bufsz_);

  void start() override;
  void stop() override;
  void shutdown() override;
  const std::string& name() const override { return named::name(); }

  void write(const void* data, std::size_t size) override;
  void connect(const cool::ng::net::ip::address& addr_, uint16_t port_) override;
  void disconnect() override;

 private:
  static void on_rd_cancel(void* ctx);
  static void on_wr_cancel(void* ctx);
  static void on_rd_event(void* ctx);
  static void on_wr_event(void* ctx);

  void create_write_source(cool::ng::net::handle h_, bool start_ = true);
  void cancel_write_source();
  void cancel_read_source();

  void create_read_source(cool::ng::net::handle h_, void* buf_, std::size_t bufsz_);
  void process_connect_event(std::size_t size);
  void process_disconnect_event();
  void process_write_event(std::size_t size);

 private:
  std::atomic<state>                   m_state;
  std::weak_ptr<async::impl::executor> m_executor; // to get the diapatch queue
  cb::stream::weak_ptr                 m_handler;  // handler for user events

  // reader part
  rd_context*       m_reader;
  void*             m_buf;       // temp store for read buffer
  std::size_t       m_size;      // temp store for read buffer size

  // writer part
  context*          m_writer;
  std::atomic<bool> m_wr_busy;
  const uint8_t*    m_wr_data;
  std::size_t       m_wr_size;
  std::size_t       m_wr_pos;

};

} } } } } // namespace

#endif

