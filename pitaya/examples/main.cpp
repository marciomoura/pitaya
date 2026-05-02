#include <pitaya/pi_controller.hpp>
#include <iostream>

int main() {
    pitaya::pi_controller<float> controller(0.01);
    controller.configure(1.0f, 10.0f);
    
    float error = 1.0f;
    float output = controller.update(error);
    
    std::cout << "PI Controller output for error 1.0: " << output << std::endl;
    
    return 0;
}
