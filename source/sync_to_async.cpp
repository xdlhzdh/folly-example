#include <chrono>
#include <folly/Executor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/init/Init.h>
#include <iostream>

class Syscom {
public:
  folly::coro::Task<void> sendData() {
    // folly::coro::sleep can be cancelled. If the task is cancelled, it will
    // throw an exception. General sync blocking function can not be cancelled
    co_await folly::coro::sleep(std::chrono::seconds(5));
    std::cout << "Sending data...\n";
    co_return;
  }

  folly::coro::Task<std::string> receiveData() {
    co_await folly::coro::sleep(std::chrono::seconds(5));
    std::cout << "Receiving data...\n";
    co_return "Received Data";
  }
};

folly::coro::Task<std::string>
sendAndReceiveDataAsync(Syscom &syscom, folly::Executor *executor,
                        folly::CancellationToken cancelToken) {
  co_await folly::coro::co_viaIfAsync(
      executor,
      folly::coro::co_withCancellation(cancelToken, syscom.sendData()));
  co_return co_await folly::coro::co_viaIfAsync(
      executor,
      folly::coro::co_withCancellation(cancelToken, syscom.receiveData()));
}

folly::coro::Task<void> runOperation(Syscom &syscom, folly::Executor *executor,
                                     folly::CancellationToken cancelToken) {
  auto data = co_await sendAndReceiveDataAsync(syscom, executor, cancelToken);
  std::cout << "Data: " << data << std::endl;
  co_return;
}

void runInThread(Syscom &syscom, folly::Executor *executor,
                 folly::CancellationToken cancelToken) {
  // try - catch should be used here
  try {
    folly::coro::blockingWait(runOperation(syscom, executor, cancelToken));
  } catch (const folly::OperationCancelled &e) {
    std::cout << "Operation cancelled in thread\n";
  }
}

int main(int argc, char *argv[]) {
  folly::Init init(&argc, &argv);

  Syscom syscom;
  folly::CPUThreadPoolExecutor executor(1);
  folly::CancellationSource cancelSource;
  auto cancelToken = cancelSource.getToken();

  std::thread t(runInThread, std::ref(syscom), &executor, cancelToken);

  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "Request cancellation..." << std::endl;
  cancelSource.requestCancellation();

  t.join();
  return 0;
}