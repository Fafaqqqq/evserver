#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <array>
#include <fstream>
#include <memory>
#include <mutex>

/**
 * @brief Singlton-класс для логирования
 */
class logger {
  public:

    using pointer = std::shared_ptr<logger>;

    // Форматированая функция печати
    void
    print(const char* format, ...);

    // Получене объекта логера
    static pointer
    get();

  private:
    enum {
      kBufSize = 1000,
    };

    logger();
    logger(const logger&) = delete;
    logger(logger&& ) = delete;

    logger& operator=(const logger&) = delete;
    logger& operator=(logger&&) = delete;


    // Файл, в который выводятся логи
    std::ofstream file_{"log.txt"};

    // Мьютекс для защиты от гонок
    std::mutex    mtx_;

    // Сюда складывается строка, построеная по формату
    std::array<uint8_t, kBufSize> str_buf_;
};

#endif