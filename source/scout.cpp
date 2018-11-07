#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../include/scout/control_flow_graph.h"
#include "../include/scout/debugger.h"

int main(int const argc, char const* const argv[])
{
    std::vector<std::string> const args(argv + 1, argv + argc);

    if (args.empty())
    {
        std::cerr << "Missing arguments" << std::endl;
        return 1;
    }

    auto const file_name = args.front();
    std::cout << file_name << std::endl;

    std::ifstream file_stream(file_name, std::ios::binary);

    if (!file_stream.good())
    {
        std::cerr << "File could not be opened." << std::endl;
        return 1;
    }

    debugger d;
    file_stream >> d;

    if (file_stream.fail())
    {
        std::cerr << "File has invalid format." << std::endl;
        return 1;
    }

    auto const cfg = control_flow_graph<debugger>(d);

    /* TODO */

    return 0;
}
