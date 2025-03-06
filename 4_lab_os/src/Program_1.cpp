#include <iostream>

extern "C" float Pi(int k);
extern "C" float Square(float a, float b);

int main()
{
    int prog;
    while (true)
    {
        std::cout << "Input program code: -1-exit, 1-calc PI, 2-calc square\n";
        std::cin >> prog;
        switch (prog)
        {
        case 1:
            int k;
            std::cin >> k;

            std::cout << "Pi number: " << Pi(k) << "\n\n";
            break;
        case 2:
            int a, b;
            std::cin >> a >> b;

            std::cout << Square(a, b) << "\n\n";
            break;
        default:
            std::cout << "Exit\n";
            return 0;
        }
    }
}