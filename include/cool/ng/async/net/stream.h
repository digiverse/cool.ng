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

#if !defined(cool_ng_f36defb0_dda1_1ce1_b25a_943f5deed23b)
#define      cool_ng_f36defb0_dda1_1ce1_b25a_943f5deed23b

#include <string>
#include <memory>
#include <functional>
#include <cstdint>

#include "cool/ng/bases.h"
#include "cool/ng/ip_address.h"
#include "cool/ng/impl/platform.h"

#include "cool/ng/impl/async/net_types.h"
#include "cool/ng/impl/async/net_stream.h"

namespace cool { namespace ng {

namespace async { namespace net {

namespace impl {

class server;

} // namespace impl
/**
 * Connection-based network input/output stream.
 *
 */
class stream
{
 public:
  stream() { /* noop */ }
  /**
   * <em>(1)</em> Constructs a new instance of asynchronous connection-oriented input/output
   * stream.
   *
   * When constructed using this constructor the stream is <em>not
   * connected</em> and cannot be used for data input/output until it is connected
   * to the network peer. The unconnected streams can only act as the <em>client
   * side</em> of the network connection and cannot listen for the incoming
   * connect requests. Use an instance of @ref server class to act as the
   * server side of the network point to point connection.
   *
   * @tparam RunnerT <b>RunnerT</b> is the concrete type of the @ref cool::ng::async::runner "runner"
   *         to be used to schedule tasks that will call specified handlers and factories.
   *
   * @tparam ReadHandlerT <b>ReadHandlerT</b> is he actual type of the read handler.
   *         This type must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&, void*&, std::size_t&)>
   * ~~~
   *         The first parameter is the shared pointer to the runner (which @em may
   *         be @c nullptr if the runner no longer exists). The second parameter is
   *         a reference to the buffer where data is read, and the third parameter
   *         is number of bytes, read from the network. Note that the read handler
   *         @em may modify the last two parameters. The read handler will be called
   *         from the task, scheduled to run to the spepcified runner each time
   *         data is receved from the network. Further reads will not occur until
   *         the handler's completion. The handler must either process all data
   *         during the call, or may specify the alternate buffer to use for the
   *         next read by simply modifying the second and the third input
   *         parameters.
   *
   * @tparam WriteHandlerT <b>WriteHandlerT</b> is the actual type of the write handler.
   *         This type must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&, const void*, std::size_t)>
   * ~~~
   *         The first parameter is the shared pointer to the runner (which @em may
   *         be @c nullptr if the runner no longer exists). The second parameter is
   *         an address of the buffer wit hdata to wrire, and the third parameter
   *         is number of bytes to write. The second and the third parameters are
   *         the same values as were specified to the @ref write call. The write
   *         handler is called only once, upon the write completion and signals
   *         the application that data is no longer needed and may be discared.
   *
   * @tparam EvtHandlerT <b>EvtHandlerT</b> is the actual type of the event handler.
   *         This type must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&, oob_event, uint32_t code)>
   * ~~~
   *         The first parameter is the shared pointer to the runner (which @em may
   *         be @c nullptr if the runner no longer exists). The second parameter is
   *         an enumerated value identifying the stream related event and the third
   *         is a system specific error code indicating the nature of the failure.
   *         Note that the third parameter will have meaning only if the event
   *         was caused by failure - otherwise it will be set to 0. The following
   *         is the list of possible events:
   *          Value                       | Description
   *          ----------------------------|------------
   *          oob_event::connected        | The stream successfully connected
   *          oob_event::failure_detected | The stream failed to connect to network peer
   *          oob_event::disconnected     | The network peer closed the connection
   *
   * @param r_  weak pointer to @ref cool::ng::async::runner "runner" to use to
   *            schedule asynchronous notifications for execution.
   * @param hr_ read handler to be called from the scheduled task when data has
   *            been read from the network connection
   * @param hw_ write handler to be called from the scheduled task when the @ref
   *            write operation has completed
   * @param he_ event handler to be called from the scheduled tash when an
   *            stream related event occurs
   * @param buf_ data optional data buffer to be used to read received data
   *            into - if set to @c nullptr the stream will allocate
   *            its own buffer internally
   * @param sz_ size of the user provided buffer or, if stream is to allocate
   *            buffer internally, the size of the buffer to allocate
   *
   * @throw cool::ng::exception::socket_failure if any network socket operations failed
   * @throw std::bad_alloc if the internal memory allocation failed
   * @sa connect()
   */
  template <typename RunnerT, typename ReadHandlerT, typename WriteHandlerT, typename EvtHandlerT>
  stream(const std::weak_ptr<RunnerT>& r_
       , const ReadHandlerT& hr_
       , const WriteHandlerT& hw_
       , const EvtHandlerT& he_
       , void* buf_ = nullptr
       , std::size_t sz_ = 16384)
  {
    using read_handler  = typename detail::types<RunnerT>::read_handler;
    using write_handler = typename detail::types<RunnerT>::write_handler;
    using event_handler = typename detail::types<RunnerT>::event_handler;

    auto impl = cool::ng::util::shared_new<detail::stream<RunnerT>>(
        r_
      , static_cast<read_handler>(hr_)
      , static_cast<write_handler>(hw_)
      , static_cast<event_handler>(he_));

    m_impl = impl;
    impl->initialize(buf_, sz_);
  }

  /**
   * <em>2</em> Constructs a new instance of asynchronous connection-oriented
   * input/output stream and connects it to the specified address.
   *
   * When constructed using this constructor the stream is will initiate an
   * asyncronous connect request to the network peer. This means that when
   * the stream instance is constructed, it will not yet @em be connected to
   * the network peer, but the connection request is already pending. Upon the
   * connect request completion, the stream will invoke the specified @a he_
   * event handler with the outcome of the reqeust. Only after the call to
   * the @a he_ handler the stream instance, providing it reported success,
   * is connected and ready to transmit and receive data.
   *
   * <b>Template Parameters</b><br>
   * See @ref stream() "stream(r_, hr_, hw_, he_)" constructor for details on
   * the template parameters.
   *
   * @param r_  weak pointer to @ref cool::ng::async::runner "runner" to use to
   *            schedule asynchronous notifications for execution.
   * @param addr_ IP address of the network peer to connect to. This may be an
   *            @ref cool::ng::net::ipv4::host "IPv4" or an
   *            @ref cool::ng::net::ipv4::host "IPv6" host address.
   * @param port_ TCP port on the network peer to connect to.
   * @param hr_ read handler to be called from the scheduled task when data has
   *            been read from the network connection
   * @param hw_ write handler to be called from the scheduled task when the @ref
   *            write operation has completed
   * @param he_ event handler to be called from the scheduled tash when an
   *            stream related event occurs
   * @param buf_ data optional data buffer to be used to read received data
   *            into - if set to @c nullptr the stream will allocate
   *            its own buffer internally
   * @param sz_ size of the user provided buffer or, if stream is to allocate
   *            buffer internally, the size of the buffer to allocate
   *
   * @throw cool::ng::exception::socket_failure if any network socket operations failed
   * @throw cool::ng::exception::runner_not_available if the @ref cool::ng::async::runner
   *        "runner" specified via parameter @a r_ is no longer available
   * @throw std::bad_alloc if the internal memory allocation failed
   */
  template <typename RunnerT, typename ReadHandlerT, typename WriteHandlerT, typename OobHandlerT>
  stream(const std::weak_ptr<RunnerT>& r_
       , const cool::ng::net::ip::address& addr_
       , uint16_t port_
       , const ReadHandlerT& hr_
       , const WriteHandlerT& hw_
       , const OobHandlerT& he_
       , void* buf_ = nullptr
       , std::size_t sz_ = 16384)
  {
    using read_handler  = typename detail::types<RunnerT>::read_handler;
    using write_handler = typename detail::types<RunnerT>::write_handler;
    using event_handler = typename detail::types<RunnerT>::event_handler;

    auto impl = cool::ng::util::shared_new<detail::stream<RunnerT>>(
        r_
      , static_cast<read_handler>(hr_)
      , static_cast<write_handler>(hw_)
      , static_cast<event_handler>(he_));

    m_impl = impl;
    impl->initialize(addr_, port_, buf_, sz_);
  }

  dlldecl const std::string& name() const;

  /**
   * Send data to the connected peer.
   */
  dlldecl void write(const void* data_, std::size_t size_);

  /**
   * Connects the unconnected stream to the remote peer.
   *
   * This method initiates a connect request to the network peer at the
   * address specified by the @a addr_ and @a port parameters. Note that
   * the connect may not be completed upon the method completion and the
   * user code must wait for the event reported to the event handler specified
   * at the stream construction.
   *
   * @param addr_ IP address of the network peer to connect to. This may be an
   *            @ref cool::ng::net::ipv4::host "IPv4" or an
   *            @ref cool::ng::net::ipv4::host "IPv6" host address.
   * @param port_ TCP port on the network peer to connect to.
   *
   * @throw cool::ng::exception::invalid_state if the stream is not disconnected.
   */
  dlldecl void connect(const cool::ng::net::ip::address& addr_, uint16_t port_);

  dlldecl void disconnect();

  explicit operator bool() { return !!m_impl; }

 private:
  friend class impl::server;
  std::shared_ptr<detail::connected_writable> m_impl;
};

} } } } // namespace

#endif
