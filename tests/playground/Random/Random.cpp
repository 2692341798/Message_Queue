#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <atomic>

static std::string uuid() {
    std::random_device rd;
    std::mt19937_64 gernator(rd());
    std::uniform_int_distribution<int> distribution(0, 255);
    std::stringstream ss;
    for (int i = 0; i < 8; i++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << distribution(gernator) ;
        if (i == 3 || i == 5 || i == 7) {
            ss << "-";
        }
    }
    static std::atomic<size_t> seq(1);
    size_t num = seq.fetch_add(1);
    for (int i = 7; i >= 0; i--) {
        ss << std::setw(2) << std::setfill('0') << std::hex << ((num>>(i*8)) & 0xff);
        if (i == 6) ss << "-";
    }
    return ss.str();
}

int main()
{
    for (int i = 0; i < 20; i++) {
        std::cout << uuid() << std::endl;
    }
    return 0;
}