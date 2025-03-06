#include <iostream>
#include <dlfcn.h>

int main()
{
    int prog = 1;
    int real = 1;
    void *lib = nullptr;

    typedef float (*PiFunc)(int);
    typedef float (*SquareFunc)(float, float);

    PiFunc Pi;
    SquareFunc Square;

    lib = dlopen("./libPr_2_real1.so", RTLD_LAZY);
    if (!lib)
    {
        std::cerr << "Error loading initial library: " << dlerror() << std::endl;
        return 1;
    }
    std::cout << "Library is loaded\n";

    Pi = (PiFunc)dlsym(lib, "Pi");
    Square = (SquareFunc)dlsym(lib, "Square");
    if (!Pi || !Square)
    {
        std::cerr << "Failed to load symbols: " << dlerror() << std::endl;
        dlclose(lib);
        return 1;
    }

    while (true)
    {
        std::cout << "Input program code: -1-exit, 0-change realisation, 1-calc PI, 2-calc square\n";
        std::cin >> prog;
        switch (prog)
        {
        case 0:
            dlclose(lib);
            if (real == 1)
            {
                lib = dlopen("./libPr_2_real2.so", RTLD_LAZY);
                real = 2;
            }
            else
            {
                lib = dlopen("./libPr_2_real1.so", RTLD_LAZY);
                real = 1;
            }
            if (!lib)
            {
                std::cerr << "Error loading library: " << dlerror() << std::endl;
                return 1;
            }
            std::cout << "Library is loaded\n";

            Pi = (PiFunc)dlsym(lib, "Pi");
            Square = (SquareFunc)dlsym(lib, "Square");
            if (!Pi || !Square)
            {
                std::cerr << "Failed to load symbols: " << dlerror() << std::endl;
                dlclose(lib);
                return 1;
            }
            break;
        case 1:
            int k;
            std::cin >> k;

            if (real == 1)
                std::cout << "Teck realization of Pi is Leibniz\n";
            else
                std::cout << "Teck realization of Pi is Wallis\n";
            std::cout << "Pi number: " << k <<" " << Pi(k) << "\n\n";
            break;
        case 2:
            int a, b;
            std::cin >> a >> b;

            if (real == 1)
                std::cout << "Teck realization of Square is Rectangle\n";
            else
                std::cout << "Teck realization of Pi is Triangle\n";
            std::cout << "Square is " << Square(a, b) << "\n\n";
            break;
        default:
            std::cout << "Exit\n";
            dlclose(lib);
            return 0;
        }
    }
}