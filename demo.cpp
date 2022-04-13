#include <iostream>

#include "InstrumentorTimer.h"
#include "InstrumentorMacro.h"

namespace Benchmark {
    int Fibonacci(int x) {
        std::string name = std::string("Fibonacci ") + std::to_string(x);
        PROFILE_SCOPE(name.c_str());

        std::this_thread::sleep_for(std::chrono::microseconds(1)); // Compensate statistical temporal precision problem

        if (x < 3) return 1;
        int part1 = Fibonacci(x - 1);
        int part2 = Fibonacci(x - 2);
        return part1 + part2;
    }

    void RunBenchmarks() {
        PROFILE_FUNCTION();

        std::cout << "Running Benchmarks..." << std::endl;
        std::thread t1([]() { Fibonacci(9); });
        std::thread t2([]() { Fibonacci(10); });

        t1.join();
        t2.join();
    }
}

int main() {
    Instrumentor::BeginSession("Benchmark");
    Benchmark::RunBenchmarks();
    return 0;
}
