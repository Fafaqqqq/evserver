#include <chrono>
#include <cstdio>
#include <ev.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "logger.h"

namespace tcp_server {
  using EVLoop = struct ev_loop;
  using EVIo   = struct ev_io;

  enum {
    kClientBufSize = 0x100,
  };

  static logger::pointer logger_;

  /**
   * @brief Callback для обработки приёма данных
   */
  static void on_read(EVLoop* loop, EVIo* watcher, int events) {
    char client_data[kClientBufSize];

    // Принимаем сообщения от клиента
    auto read_cnt = recv(watcher->fd, client_data, kClientBufSize, 0);

    if (read_cnt < 0) {
      return;
    }

    if (read_cnt == 0) {
      // Останавливаем обработку события по этому сокету
      ev_io_stop(loop, watcher);

      // Закрываем сокет клиента
      close(watcher->fd);

      delete watcher;

      logger_->print("closing socet %d", watcher->fd);
      return;
    } else {
      logger_->print("read size: %d, data: %s", read_cnt, client_data);
    }

    // Отпавляем сообщение клиенту
    send(watcher->fd, client_data, read_cnt, MSG_DONTWAIT);
  }

  /**
   * @brief Callback для обработки подключения
   */
  static void on_accept(EVLoop* loop, EVIo* watcher_accept, int events) {
    struct sockaddr_in client_addr;
    socklen_t       client_len = sizeof(client_addr);
    int             sock;
    EVIo*           watcher_read = new EVIo;

    if (EV_ERROR & events) {
      logger_->print("got ivalid event" );
      return;
    }

    logger_->print("waiting for accept...");

    // Приниманием подключения с созданием сокета.
    sock = accept(watcher_accept->fd, (struct sockaddr*)&client_addr, &client_len);
    if (sock < 0) {
      logger_->print("error accepting client connection");
      return;
    }

    // Выводим информацию о подключившемся клиенте
    char addr_buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr_buffer, INET_ADDRSTRLEN);
    logger_->print("connection accepted from %s:%d", addr_buffer, ntohs(client_addr.sin_port));

    // Инициируем обработку события приёма данных от клиента
    ev_io_init(watcher_read, on_read, sock, EV_READ);
    ev_io_start(loop, watcher_read);
  }

  /**
   * @brief Функция устанавливает адрес и порт,
   * на котором будет работать сервер. Также
   * инициализирует обработку события подключения.
   */
  EVLoop* init(const char* addr, int port) {
    logger_ = logger::get();

    EVLoop* loop = ev_default_loop(EVRUN_NOWAIT);
    EVIo*   watcher = new EVIo;
    int     sock;

    sockaddr_in addr_in;
    // Настраиваем адрес сервера
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = inet_addr(addr);
    addr_in.sin_port = htons(port);


    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
      logger_->print("error creating socket");
      return nullptr;
    }

    // Связываем сокет с адресом
    logger_->print("binding socket");
    if (bind(sock, reinterpret_cast<sockaddr *>(&addr_in), sizeof(addr_in)) < 0) {
      logger_->print("error binding socket");
      close(sock);
      return nullptr;
    }

    // Начинаем слушать входящие подключения
    logger_->print("listening socket");
    if (listen(sock, SOMAXCONN) < 0) {
      logger_->print("error listening socket");
      close(sock);
      return nullptr;
    }

    // Инициализируем обработку события подключения
    ev_io_init(watcher, on_accept, sock, EV_READ);
    ev_io_start(loop, watcher);

    return loop;
  }
}

int main(int argc, char* argv[]) {
  // Запускаем сервер на порту 5000
  auto server_loop = tcp_server::init("127.0.0.1", 5000);

  // Запускаем event loop
  ev_run(server_loop, 0);

  return 0;
}