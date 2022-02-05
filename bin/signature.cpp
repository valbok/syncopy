/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "file.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << argv[0] << " DESTINATION_FILE [WINDOW]" << std::endl;
        return 0;
    }

    std::string fn = argv[1];
    syncopy::File file(fn);

    if (!file.exists()) {
        std::cerr << "Could not open file: " << fn << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "destination file : " << fn << std::endl;
    std::cout << "size             : " << file.size() << std::endl;

    auto sig = file.signature(argc < 3 ? 500 : std::stoi(argv[2]));
    auto fn_sig = fn + ".sig";
    sig.save(fn_sig);
    std::cout << "signature file   : " << fn_sig << std::endl;
    std::cout << "window           : " << sig.window << std::endl;
    std::cout << "chunks           : " << sig.chunks.size() << std::endl;

    return EXIT_SUCCESS;
}