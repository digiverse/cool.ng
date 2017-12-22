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

#if !defined(cool_ng_f36defb0_aaa1_4ce1_b25a_943f5beef23a)
#define      cool_ng_f36defb0_aaa1_4ce1_b25a_943f5beef23a

#include <atomic>
#include <memory>
#include <functional>
#include <cstdint>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#include "cool/ng/bases.h"
#include "cool/ng/ip_address.h"
#include "cool/ng/impl/async/event_sources_types.h"
#include "cool/ng/impl/async/event_sources.h"

#include "executor.h"
#include "critical_section.h"

namespace cool { namespace ng { namespace async { namespace impl {


// ==========================================================================
// ======
// ======
// ====== Timer event source
// ======
// ======
// ==========================================================================

class timer : public cool::ng::util::named
            , public detail::itf::timer
            , public cool::ng::util::self_aware<timer>
{
  enum class state { running, destroying };

  struct context
  {
    context(const timer::ptr& s_);
    ~context();

    static void on_event(PTP_CALLBACK_INSTANCE i_, PVOID ctx_, PTP_TIMER t_);

    timer::ptr                m_timer;
    async::impl::poolmgr::ptr m_pool;
    PTP_TIMER                 m_source;
    std::atomic<bool>         m_active;
  };

 public:
  timer(const std::weak_ptr<cb::timer>& t_
      , uint64_t p_
      , uint64_t l_);
  ~timer();

  void initialize(const std::shared_ptr<async::impl::executor>& ex_);
  // detail::itf::timer
  void start() override;
  void stop() override;
  void period(uint64_t p_, uint64_t l_) override;
  void shutdown() override;
  const std::string& name() const override
  {
    return named::name();
  }

 private:
  void expired();
  
 private:
  std::atomic<state>                   m_state;
  const std::weak_ptr<cb::timer>       m_callback;
  std::weak_ptr<async::impl::executor> m_executor;
  context*                             m_context;

  uint64_t m_period;
  uint64_t m_leeway;
};

} // namespace impl



// ==========================================================================
// ======
// ======
// ====== Network event sources
// ======
// ======
// ==========================================================================

namespace net { namespace impl {

class server : public async::detail::itf::startable
             , public cool::ng::util::named
             , public cool::ng::util::self_aware<server>
{
  enum class state { init, stopped, starting, accepting, stopping, destroying, error };

  struct context
  {
    context(const server::ptr& s_);
    ~context();

    void shutdown();

   void process_accept();

    server::ptr m_server;
  };

 public:
  server(const std::shared_ptr<async::impl::executor>& ex_
       , const cb::server::weak_ptr& cb_);
  ~server();

  void initialize(const cool::ng::net::ip::address& addr_, uint16_t port_);
  const std::string& name() const { return named::name(); }
  void start() override;
  void stop() override;
  void shutdown() override;

  static void install_handle(cool::ng::async::net::stream& s_, cool::ng::net::handle h_);

 private:
  void start_accept();
  void stop_accept();
  void process_accept();
  static void CALLBACK on_accept(
      PTP_CALLBACK_INSTANCE instance_
    , PVOID context_
    , PVOID overlapped_
    , ULONG io_result_
    , ULONG_PTR num_transferred_
    , PTP_IO io_);

 private:
  std::atomic<state>          m_state;
  std::weak_ptr<async::impl::executor> m_executor; // runner for this server
  cb::server::weak_ptr        m_handler;           // towards user level event handler
  async::impl::poolmgr::ptr   m_pool;              // to keep threadpool alive

  ::cool::ng::net::handle   m_handle;              // listen socket
  ::cool::ng::net::handle   m_client_handle;       // client soocket for accept
  int                       m_sock_type;           // socket type flag to create client socket

  // function pointers of inaccessbile Winsock2 symbols
  LPFN_ACCEPTEX             m_accept_ex;      // f. pointer to AcceptEx
  LPFN_GETACCEPTEXSOCKADDRS m_get_sock_addrs; // f. pointer to GetAcceptExSockAddrs

  // for async accept operation
  PTP_IO                    m_tpio;
  uint8_t                   m_buffer[2 * sizeof(SOCKADDR_STORAGE) + 32];
  WSAOVERLAPPED             m_overlapped;

  async::impl::critical_section m_cs;
  ptr*                      m_context;
};

/*
 * The stream implementation class is kept alive by two shared pointers:
 *   - shared pointer of its parent, detail::stream class template
 *   - shared pointers of event source context, known to ThreadpoolIo
 * Note that the stream implementation does not manage the life time of event
 * source context - it will get deleted  when ThreadpoolIo reports connection
 * error.
 */
class stream : public detail::itf::connected_writable
             , public cool::ng::util::named
             , public cool::ng::util::self_aware<stream>
{
  enum class state { disconnected, connecting, connected, disconnecting  };

  struct context
  {
    context(const stream::ptr& s_, cool::ng::net::handle h_, void* buf, std::size_t sz_);
    ~context();

    // threadpool callback from completion port
    static void CALLBACK on_event(
        PTP_CALLBACK_INSTANCE instance_
      , PVOID context_
      , PVOID overlapped_
      , ULONG io_result_
      , ULONG_PTR num_transferred_
      , PTP_IO io_);

    stream::ptr          m_stream;
    cool::ng::net::handle m_handle;

    // overlapped & threadpool support
    WSAOVERLAPPED m_rd_overlapped;
    WSAOVERLAPPED m_wr_overlapped;
    PTP_IO        m_tpio;

    // reader part
    std::size_t m_rd_size;
    bool        m_rd_is_mine;
    void*       m_rd_data;
    DWORD       m_read_bytes;

    // writer part
    std::atomic<bool> m_wr_busy;
    const uint8_t*    m_wr_data;
    std::size_t       m_wr_size;
    std::size_t       m_wr_pos;
    DWORD             m_written_bytes;
  };

 public:
  stream(const std::weak_ptr<async::impl::executor>& ex_
       , const cb::stream::weak_ptr& cb_);
  ~stream();

  void initialize(const cool::ng::net::ip::address& addr_
                , uint16_t port_
                , void* buf_
                , std::size_t bufsz_);
  void initialize(void* buf_, std::size_t bufsz_);

  // event_source interface
  void shutdown() override;
  const std::string& name() const override { return named::name(); }

  // connected writable interface
  void write(const void* data, std::size_t size) override;
  void connect(const cool::ng::net::ip::address& addr_, uint16_t port_) override;
  void disconnect() override;
  void set_handle(cool::ng::net::handle h_) override;

 private:
  friend class exec_for_io;

  void start_read_source();
  void start_write_source();

  // entry point to process i/o events from executor
  void on_event(PVOID overlapped_, ULONG io_result_, ULONG_PTR num_transferred_);
  void process_connect_event(ULONG io_result_);
  void process_disconnect_event();

  void process_read_event(ULONG_PTR count_);
  void process_write_event(ULONG_PTR count_);
 
 private:

 private:
  std::atomic<state>                   m_state;
  std::weak_ptr<async::impl::executor> m_executor; // to get the diapatch queue
  async::impl::poolmgr::ptr            m_pool;
  cb::stream::weak_ptr                 m_handler;  // handler for user events
  context*                             m_context;  // threadpool I/O context

  // Win32 weirdness support
  LPFN_CONNECTEX                       m_connect_ex;

  // reader part - original parameters
  std::size_t                          m_rd_size;
  void*                                m_rd_data;

};

} } } } } // namespace

#endif

