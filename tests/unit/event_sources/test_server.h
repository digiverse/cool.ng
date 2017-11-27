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

#if !defined(cool_ng_test_f36abcb0_dda1_4ce1_b25a_943f5deed23a)
#define      cool_ng_test_f36abcb0_dda1_4ce1_b25a_943f5deed23a

#include <memory>
#include <functional>
#include <string>
#include <map>
#include "cool/ng/bases.h"
#include "cool/ng/async.h"

namespace test
{

namespace async = cool::ng::async;
namespace io = cool::ng::io;

using callback      = std::function<void()>;
using receive_ready = std::function<void(io::handle, std::size_t)>;

class test_server : public cool::ng::util::self_aware<test_server>
                  , public async::runner
{
  using client_map = std::map<io::handle, std::unique_ptr<async::reader>>;

 public:
  static ptr create(
      int port
    , const callback& on_accept
    , const receive_ready& on_receive
    , const callback on_write_complete
    , const callback& on_disconnect);

 private:
  befriend_shared_new;
  test_server(int port);
  void start();
  void on_connect(io::handle fd_, std::size_t size_);

 private:
  const int                      m_port;
  io::handle                     m_listen;
  std::unique_ptr<async::reader> m_acceptor;
  // user callbacks
  callback                       m_on_accept;
  receive_ready                  m_on_receive;
  callback                       m_on_write;
  callback                       m_on_disconnect;

  client_map                     m_clients;
};

class test_client : public cool::ng::util::self_aware<test_client>
                  , public cool::ng::async::runner
{
 public:
  static ptr create(int port, const receive_ready& on_receive, const callback on_write_complete);

  void write(int context_, const void* data_, std::size_t size_);

 private:
  befriend_shared_new;
  test_client(int port);
  void start();

 private:
  const int                      m_port;
  io::handle              m_handle;
  std::unique_ptr<async::reader> m_socket;

 private:
  // user callbacks
  receive_ready                  m_on_receive;
  callback                       m_on_write;
};

}

#endif

