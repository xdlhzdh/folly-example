#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using namespace boost::asio;
using namespace std::chrono_literals;

void handle_error(const std::string &operation,
                  const boost::system::error_code &ec) {
  if (ec != boost::asio::error::operation_aborted) {
    std::cerr << "Error during " << operation << ": " << ec.message()
              << std::endl;
  } else {
    std::cerr << "Operation " << operation << " was cancelled" << std::endl;
  }
}

template <typename AsyncOperation>
awaitable<bool> with_timeout(io_context &io_context, AsyncOperation &&operation,
                             std::chrono::steady_clock::duration timeout) {
  boost::system::error_code ec;
  steady_timer timer(io_context);
  timer.expires_after(timeout);

  bool completed = false;

  auto operation_wrapper = [&]() -> awaitable<void> {
    completed = co_await std::move(operation);
    timer.cancel();
  };

  co_spawn(io_context, operation_wrapper(), detached);

  co_await timer.async_wait(redirect_error(use_awaitable, ec));
  if (!completed) {
    std::cerr << "Operation timed out or failed" << std::endl;
    co_return false;
  }

  co_return true;
}

awaitable<bool> async_resolve(tcp::resolver &resolver, const std::string &host,
                              const std::string &port,
                              tcp::resolver::results_type &endpoints) {
  boost::system::error_code ec;
  endpoints = co_await resolver.async_resolve(
      host, port, redirect_error(use_awaitable, ec));
  if (ec) {
    handle_error("async_resolve", ec);
    co_return false;
  }
  std::cout << "Resolved to: " << endpoints->endpoint() << std::endl;
  co_return true;
}

awaitable<bool> async_connect(tcp::socket &socket,
                              tcp::resolver::results_type endpoints) {
  boost::system::error_code ec;
  co_await socket.async_connect(*endpoints, redirect_error(use_awaitable, ec));
  if (ec) {
    handle_error("async_connect", ec);
    co_return false;
  }
  std::cout << "Connected to: " << endpoints->endpoint() << std::endl;
  co_return true;
}

awaitable<bool> async_write_date(tcp::socket &socket,
                                 const std::string &message) {
  boost::system::error_code ec;
  co_await async_write(socket, buffer(message),
                       redirect_error(use_awaitable, ec));
  if (ec) {
    handle_error("async_write", ec);
    co_return false;
  }
  std::cout << "Sent: " << message << std::endl;
  co_return true;
}

awaitable<bool> async_read_data(tcp::socket &socket) {
  char data[1024];
  boost::system::error_code ec;
  std::size_t n = co_await socket.async_read_some(
      buffer(data), redirect_error(use_awaitable, ec));
  if (ec) {
    handle_error("async_read", ec);
    co_return false;
  }
  std::cout << "Received: " << std::string(data, n) << std::endl;
  co_return true;
}

awaitable<void> run_client(io_context &io_context, const std::string &host,
                           const std::string &port) {
  tcp::resolver resolver(io_context);
  // need wait for the result
  tcp::resolver::results_type endpoints;
  auto resolve_op = async_resolve(resolver, host, port, endpoints);
  if (!co_await with_timeout(io_context, resolve_op, 5s)) {
    co_return;
  }

  tcp::socket socket(io_context);
  auto connect_op = async_connect(socket, endpoints);

  // It should be use local variable to keep the lifetime of the string
  // There will be some problem if the string is a temporary constructed
  std::string message = "Hello";
  auto write_op = async_write_date(socket, message);

  auto read_op = async_read_data(socket);

  if (!co_await with_timeout(io_context, connect_op, 5s) ||
      !co_await with_timeout(io_context, write_op, 5s) ||
      !co_await with_timeout(io_context, read_op, 5s)) {
    co_return;
  }
}

int MAIN_1(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: client <host> <port>" << std::endl;
    return 1;
  }
  std::cout << "Starting client host: " << argv[1] << " port: " << argv[2]
            << std::endl;

  io_context io_context;
  co_spawn(io_context, run_client(io_context, argv[1], argv[2]), detached);
  io_context.run();
  return 0;
}