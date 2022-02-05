/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "file.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cout << argv[0] << " SIGNATURE_FILE SOURCE_FILE" << std::endl;
        return 0;
    }

    syncopy::Signature sig;
    if (!sig.load(argv[1])) {
        std::cerr << "Could not recognize signature file: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "signature file : " << argv[1] << std::endl;
    std::cout << "window         : " << sig.window << std::endl;
    std::cout << "chunks         : " << sig.chunks.size() << std::endl;

    std::string fn = argv[2];
    syncopy::File src(fn);

    if (!src.exists()) {
        std::cerr << "Could not open source file: " << fn << std::endl;
        return EXIT_FAILURE;
    }

    auto delta = src.delta(sig);
    auto fn_delta = fn + ".delta";
    delta.save(fn_delta);

    std::cout << std::endl;
    std::cout << "delta file     : " << fn_delta << std::endl;
    std::cout << "md5            : " << delta.md5 << std::endl;
    std::cout << "chunks         : " << delta.chunks.size() << std::endl;

    return EXIT_SUCCESS;
}