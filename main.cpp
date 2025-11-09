#include "cli_parser.h"
#include <iostream>

int main(int argc, char* argv[]) {
    auto options = chronos::CLIParser::parse(argc, argv);
    chronos::CLIParser::printOptions(options);
    return options.is_valid ? 0 : 1;
}