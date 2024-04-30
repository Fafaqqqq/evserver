#include <iostream>

#include "logger.h"
#include "tcp_server.h"

int main() {

  ev::default_loop loop;
  tcp::server server(loop);

  // Запускаем сервер на порту 12345
  server.start(5000, "127.0.0.1");

  // Запускаем event loop
  loop.run(0);

  return 0;
}