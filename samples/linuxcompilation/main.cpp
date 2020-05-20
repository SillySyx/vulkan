#include <iostream>

#include "App.h"

int main()
{
  auto app = new App();

  std::cout << app->Hello("me") << std::endl;
}
