/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "file.h"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cout << argv[0] << " DELTA_FILE DESTINATION_FILE" << std::endl;
        return 0;
    }

    std::ifstream f(argv[1]);

    if (!f.good()) {
        std::cerr << "Could not open delta file: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    syncopy::Delta delta;
    if (!delta.deserialize(f)) {
        std::cerr << "Could not recognize delta file: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    syncopy::File dst(argv[2]);

    if (!dst.exists()) {
        std::cerr << "Could not open destination file: " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }

    dst.patch(delta);

    return EXIT_SUCCESS;
}