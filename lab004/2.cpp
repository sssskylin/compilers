#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

struct Particle {
  float x, y, z;
  float vx, vy, vz;
  uint32_t color;
};

int main() {
  const size_t N = 10'000'000;

  std::mt19937 rng(12345);
  std::uniform_real_distribution<float> ud(-1.f, 1.f);

  std::vector<Particle> aos(N);
  for (auto& p : aos) {
    p.x = ud(rng);
    p.y = ud(rng);
    p.z = ud(rng);
    p.vx = ud(rng);
    p.vy = ud(rng);
    p.vz = ud(rng);
    p.color = rng();
  }

  std::vector<float> x(N);
  std::vector<float> y(N);
  std::vector<float> z(N);
  std::vector<float> vx(N);
  std::vector<float> vy(N);
  std::vector<float> vz(N);
  std::vector<uint32_t> color(N);

  for (size_t i = 0; i < N; ++i) {
    x[i] = aos[i].x;
    y[i] = aos[i].y;
    z[i] = aos[i].z;
    vx[i] = aos[i].vx;
    vy[i] = aos[i].vy;
    vz[i] = aos[i].vz;
    color[i] = aos[i].color;
  }

  auto now = [] { return std::chrono::high_resolution_clock::now(); };
  auto ms = [](auto a, auto b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
  };

  auto t0 = now();
  for (size_t i = 0; i < N; ++i) {
    aos[i].x += aos[i].vx;
    aos[i].y += aos[i].vy;
    aos[i].z += aos[i].vz;
  }
  auto t1 = now();

  auto t2 = now();
  for (size_t i = 0; i < N; ++i) {
    x[i] += vx[i];
    y[i] += vy[i];
    z[i] += vz[i];
  }
  auto t3 = now();

  volatile float sink = aos[N - 1].x + x[N - 1];
  (void)sink;

  printf("AoS: %.1f ms\n", ms(t0, t1));
  printf("SoA: %.1f ms\n", ms(t2, t3));
  printf("SoA быстрее в %.2f раза\n", ms(t0, t1) / ms(t2, t3));
}
