#include <folly/coro/BlockingWait.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/init/Init.h>
#include <iostream>

using namespace folly;

coro::Task<void> runTasks() {
  auto task1 =
      folly::makeFuture().delayed(std::chrono::seconds(1)).thenValue([](auto) {
        return "Task 1 completed";
      });
  auto task2 =
      folly::makeFuture().delayed(std::chrono::seconds(2)).thenValue([](auto) {
        return "Task 2 completed";
      });

  auto [index, result] =
      co_await coro::collectAny(std::move(task1), std::move(task2));

  std::cout << result.value() << " at index " << index << std::endl;
}

// 模拟一个阻塞函数在协程中执行，并且个这个阻塞函数设置超时时间

int MAIN_2(int argc, char *argv[]) {

  folly::Init init(&argc, &argv);
  folly::coro::blockingWait(runTasks());

  return 0;
}