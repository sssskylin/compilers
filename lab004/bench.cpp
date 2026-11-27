
#include <pthread.h>
#include <sched.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

constexpr std::size_t CACHE_LINE_SIZE = 64;
constexpr uint64_t ITERATIONS = 100'000'000;

struct AdjacentCounters {
  std::atomic<uint64_t> counters[2]{};
};

struct alignas(CACHE_LINE_SIZE) PaddedCounter {
  std::atomic<uint64_t> value{0};
};

struct PaddedCounters {
  PaddedCounter counters[2];
};

void pin_thread(int cpu) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);

  int result =
      pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

  if (result != 0) {
    std::cerr << "Affinity error for CPU " << cpu << ": "
              << std::strerror(result) << '\n';
  }
}

template <typename Counter, typename Accessor>
double run_benchmark(Counter& counters, Accessor get_counter) {
  std::atomic<bool> start{false};
  int actual_cpu[2] = {-1, -1};

  auto worker = [&](int id) {
    pin_thread(id);
    actual_cpu[id] = sched_getcpu();

    while (!start.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    auto& counter = get_counter(counters, id);

    for (uint64_t i = 0; i < ITERATIONS; ++i) {
      counter.fetch_add(1, std::memory_order_relaxed);
    }
  };

  std::thread t1(worker, 0);
  std::thread t2(worker, 1);

  auto begin = std::chrono::steady_clock::now();

  start.store(true, std::memory_order_release);

  t1.join();
  t2.join();

  auto end = std::chrono::steady_clock::now();

  std::cout << "Thread 1 CPU: " << actual_cpu[0] << '\n';
  std::cout << "Thread 2 CPU: " << actual_cpu[1] << '\n';

  return std::chrono::duration<double>(end - begin).count();
}

int main() {

  AdjacentCounters adjacent;

  std::cout << "Variant A\n";

  double time_a = run_benchmark(
      adjacent, [](auto& c, int id) -> auto& { return c.counters[id]; });

  std::cout << "Counter 1: " << adjacent.counters[0].load() << '\n';
  std::cout << "Counter 2: " << adjacent.counters[1].load() << '\n';
  std::cout << "Time A: " << time_a << " seconds\n\n";

  PaddedCounters padded;

  std::cout << "Variant B\n";

  double time_b = run_benchmark(
      padded, [](auto& c, int id) -> auto& { return c.counters[id].value; });

  std::cout << "Counter 1: " << padded.counters[0].value.load() << '\n';

  std::cout << "Counter 2: " << padded.counters[1].value.load() << '\n';

  std::cout << "Time B: " << time_b << " seconds\n\n";

  std::cout << "Speedup (A / B): " << time_a / time_b << "\n";

  return 0;
}
