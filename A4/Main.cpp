// Winter 2020

#include <iostream>
#include "scene_lua.hpp"

int main(int argc, char** argv)
{
  //std::string filename = "Assets/macho-cows.lua";
  std::string filename = "sample.lua";
  if (argc >= 2) {
    filename = argv[1];
  }

  if (!run_lua(filename)) {
    std::cerr << "Could not open " << filename << std::endl;
    return 1;
  }
}
