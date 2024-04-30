#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <ev++.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "logger.h"

// Класс, представляющий TCP-сервер
namespace tcp {
  class server {
    public:
      server(ev::loop_ref loop) : loop_(loop), watcher_(loop) {
        logger_ = logger::get();
      }

      // Метод для запуска сервера
      void
      start(int port, const char* addr_str) {
        logger_->print("starting server...");

        sockaddr_in addr;
        // Создаем сокет
        logger_->print("creating socket");
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) {
          logger_->print("error creating socket");
          return;
        }

        // Настраиваем адрес сервера
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(addr_str);
        addr.sin_port = htons(port);

        // Связываем сокет с адресом
        logger_->print("binding socket");
        if (bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
          logger_->print("error binding socket");
          close(sock);
          return;
        }

        // Начинаем слушать входящие подключения
        logger_->print("listening socket");
        if (listen(sock, SOMAXCONN) < 0) {
          logger_->print("error listening socket");
          close(sock);
          return;
        }

        // Инициализируем watcher для прослушивания новых подключений
        watcher_.set<server, &server::on_accept>(this);
        watcher_.set(sock, ev::READ);
        watcher_.start();

        logger_->print("server successfully started on %s:%d", addr_str, port);
      }

    private:
      // Callback для обработки новых подключений
      void
      on_accept(ev::io &watcher, int events) {
        if (EV_ERROR & events) {
          logger_->print("got ivalid event");
          return;
        }

        sockaddr_in client_addr;
        socklen_t   client_len = sizeof(client_addr);

        // Принимаем входящее подключение
        logger_->print("waiting for accept...");
        int sock = accept(watcher.fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        if (sock < 0) {
          logger_->print("error accepting client connection");
          return;
        }

        // Выводим информацию о подключившемся клиенте
        char addr_buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, addr_buffer, INET_ADDRSTRLEN);
        logger_->print("connection accepted from %s:%d", addr_buffer, ntohs(client_addr.sin_port));

        char client_data[0x100];
        auto read = recv(sock, client_data, 0x100, 0);

        logger_->print("read size: %d, data: %s", read, client_data);

        send(sock, client_data, read, MSG_DONTWAIT);

        // Закрываем сокет клиента
        close(sock);
      }

      ev::io          watcher_; // watcher для прослушивания новых подключений
      ev::loop_ref    loop_;    // ссылка на event loop
      logger::pointer logger_;  // для логирования событий
  };
}

#endif