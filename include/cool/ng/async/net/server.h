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

#if !defined(cool_ng_f36defb0_dda1_0ce1_b25a_943f5deed23b)
#define      cool_ng_f36defb0_dda1_0ce1_b25a_943f5deed23b

#include <string>
#include <memory>
#include <functional>
#include <cstdint>

#include "cool/ng/bases.h"
#include "cool/ng/ip_address.h"
#include "cool/ng/impl/platform.h"

#include "cool/ng/impl/async/event_sources_types.h"
#include "cool/ng/impl/async/net_server.h"

namespace cool { namespace ng {

namespace async { namespace net {

/**
 * Network server for conenction-oriented network communication.
 *
 * This class represents a network listener that listens at the specified network
 * address for an incomming connection requests from remote clients and
 * construcs a network @ref stream for each accepted connection.
 *
 * @note This class is a thin reference counting wrapper of the underlying server
 * implementation. Creating a copy of the @ref server using copy construction or
 * copy assignmen will not create a new actual server, it will just create a
 * new wrapper instace that both refer to the same server implementation instance.
 * The change of the @ref server's state will affect the state of other @ref server
 * instances (clones) that refer to the same server implementation instance.
 */
class server
{
 public:
  /**
   * Default constructor to allow @ref server "servers" to be stored in standard
   * library containers.
   *
   * This constructor constructs an empty, non-functional @ref server. The only
   * way to make it functional is to replace it with a functional server using
   * copy assignment or move assignment operator.
   *
   * @note The only permitted operations on an empty server are copy assignment
   *   and the @ref operator bool() "bool" conversion operator. Any other
   *   operation will throw @ref cool::ng::exception::empty_object "empty_object"
   *   exception.
   */
  server() { /* noop */ }
  /**
   * Constructs new instance of server with error handler.
   *
   * @tparam RunnerT <b>RunnerT</b> is the concrete type of the @ref cool::ng::async::runner "runner"
   *         to be used to schedule tasks that will call specified handlers.
   *
   * @tparam ConnectHandlerT <b>ConnectHandlerT</b> is the concrete type of the user provided
   *         @em Callable that will be invoked each time a new client connects and the @ref
   *         server has constructed new @ref stream. This type must be assignable to the
   *         following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&, const stream&)>
   * ~~~
   *         The connect handler will be called from the task submitted by @ref server
   *         to the task queue of the @ref server "server's" @ref runner.
   *         The first parameter is a shared pointer to the server's @ref runner
   *         and the second paramer is the new @ref stream itself. By passing the new
   *         @ref stream to the connect handler the @ref server yields the ownership of
   *         this stream to the user code.
   *
   * @tparam StreamFactoryT <b>StreamFactoryT</b> is the concrete type of the user provided
   *         @em Callable that will create and return a new @ref stream. This type
   *         must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<stream(const std::shared_ptr<RunnerT>&, const cool::ng::net::ip::address&, uint16_t)>
   * ~~~
   *         Server will use this factory @em Callable, from its @ref runner context,
   *         each time a new client connects to obtain a fresh @ref stream to associate
   *         with the new client connection. The factory is expected to return a new
   *         @ref stream in unconnected state, as constructed by the stream's
   *         @ref stream::stream() "ctor (2)". @ref server will close the client
   *         connection if the factory @em Callable fails to provide @ref stream
   *         in the required state or if it throws. The parameters to the factory
   *         @em Callable, in addition to the shared pointer to server's @ref runner,
   *         are remote IP address and network port of the new client.
   * @param r_  weak pointer to @ref cool::ng::async::runner "runner" to use to
   *            schedule asynchronous notifications for execution.
   * @param addr_ IP address of the network peer to bind to. This may be an
   *            @ref cool::ng::net::ipv4::host "IPv4" or an
   *            @ref cool::ng::net::ipv6::host "IPv6" address of the network interface
   *            or one of @ref cool::ng::net::ipv4::any "ipv4::any" or
   *            @ref cool::ng::net::ipv6::any "ipv6::any" wildcards.
   * @param port_ TCP port on the network to listen to.
   * @param hc_ read handler to be called from the scheduled task when a new connect
   *            request has been detected.
   * @param sf_ stream factory to use to spawn new @ref stream "streams" for
   *            connected peers
   * @param he_ error handle to be called should the server detect network errors
   *
   * @throw cool::ng::exception::socket_failure if any network socket operations failed
   * @throw cool::ng::exception::runner_not_available if the @ref cool::ng::async::runner
   *        "runner" specified via parameter @a r_ is no longer available
   * @throw std::bad_alloc if the internal memory allocation failed
   *
   * @note The server, when constructed, is in a stopped state and will not
   *       listen for the incomming connection requests. You'll need to call
   *       @ref start() method to activate the server.
   */
  template <typename RunnerT
          , typename StreamFactoryT
          , typename ConnectHandlerT
          , typename ErrorHandlerT = typename detail::types<RunnerT>::error_handler
  >
  server(const std::weak_ptr<RunnerT>& r_
       , const cool::ng::net::ip::address& addr_
       , uint16_t port_
       , const StreamFactoryT& sf_
       , const ConnectHandlerT& hc_
       , const ErrorHandlerT& he_ = ErrorHandlerT())
  {
    using stream_factory  = typename detail::types<RunnerT>::stream_factory;
    using connect_handler = typename detail::types<RunnerT>::connect_handler;
    using error_handler   = typename detail::types<RunnerT>::error_handler;

    auto impl = cool::ng::util::shared_new<detail::server<RunnerT>>(
        r_
      , static_cast<stream_factory>(sf_)
      , static_cast<connect_handler>(hc_)
      , static_cast<error_handler>(he_));

    m_impl = impl;
    impl->initialize(addr_, port_);
  }
  /**
   * Starts the @ref server.
   */
  dlldecl void start();
  dlldecl void stop();
  dlldecl const std::string& name() const;

  /**
   * Empty server predicate.
   *
   * @return true if this @ref server is properly created and functional, false if empty.
   *
   * @note Returning true does not imply that the server is in a correct state
   *       for the required operation. It just indicates that it was not
   *       default constructed as an empty shell/placeholder.
   */
  dlldecl explicit operator bool() const;

 private:
  std::shared_ptr<async::detail::itf::startable> m_impl;
};

} } } } // namespace

#endif
