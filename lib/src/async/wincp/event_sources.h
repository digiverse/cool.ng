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
#include "cool/ng/impl/async/event_sources.h"

#include "executor.h"

namespace cool { namespace ng { namespace async { namespace net { namespace impl {


class server : public cool::ng::async::detail::startable {
public:
  server(const std::shared_ptr<async::impl::executor>& ex_
       , const cool::ng::net::ip::address& addr_
       , int port_
       , const cb_server::weak_ptr& cb_);

  void start() override;
  void stop() override;
  void shutdown() override;

 private:
  static void CALLBACK on_accept(
      PTP_CALLBACK_INSTANCE instance_
    , PVOID context_
    , PVOID overlapped_
    , ULONG io_result_
    , ULONG_PTR num_transferred_
    , PTP_IO io_);
  void start_accept();
  void process_accept();

 private:
  std::weak_ptr<async::impl::executor> m_executor;
  async::impl::poolmgr::ptr m_pool;
  ::cool::ng::net::handle   m_handle;
  bool                      m_active;
  cb_server::weak_ptr       m_handler;

  LPFN_ACCEPTEX             m_accept_ex;      // f. pointer to AcceptEx
  LPFN_GETACCEPTEXSOCKADDRS m_get_sock_addrs; // f. pointer to GetAcceptExSockAddrs
  PTP_IO                    m_tp_io;
  uint8_t                   m_buffer[2 * sizeof(SOCKADDR_STORAGE) + 32];
  WSAOVERLAPPED             m_overlapped;
  ::cool::ng::net::handle   m_client_handle;
  DWORD                     m_filler;
};

} } } } } // namespace

#endif

