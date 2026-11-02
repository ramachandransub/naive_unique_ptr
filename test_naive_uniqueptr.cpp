#include "unique_ptr.hpp"
#include <cstdio>
#include <utility>

using naive_smart_pointers::unique_ptr;

struct W {
  int id;
  ~W() { std::printf("~W %d\n", id);}
};

void delA(W* w) {
  std::printf("delA:%d\n", w ? w->id : -1);
  delete w;
}

void delB(W* w) {
  std::printf("delB:%d\n", w ? w->id : -1);
  delete w;
}

using un_ptr_w = unique_ptr<W, void (*)(W*)>;
auto main() -> int {
  {
    // un_ptr_w a{new W{1}, delA};
    // auto b{std::move(a)};
  }


  {
    std::printf("Part 2\n");
    un_ptr_w x{new W{20}, delB};
    {
      un_ptr_w y{new W{10}, delB};
      x = std::move(y);
      std::printf("~scope y\n");
    }
    std::printf("~scope x\n");
  }
}