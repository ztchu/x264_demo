#include <iostream>

extern "C" {
#include "x264.h"
#include "x264_config.h"
}

int main() {
    x264_param_t param;
    x264_param_default(&param);
    std::cout << "okay." << std::endl;
    system("pause");
}