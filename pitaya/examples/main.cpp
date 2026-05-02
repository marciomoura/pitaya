#include <pitaya/pitaya.hpp>
#include <iostream>

int main() {
    auto result = pitaya::sum(10, 20);
    std::cout << "The sum of 10 and 20 is: " << result << std::endl;
    return 0;
}
