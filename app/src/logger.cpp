#include "logger.h"

#include <cassert>
#include <cstring>
#include <memory>

#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <mutex>

static logger::pointer __instance = nullptr;

logger::logger() {
  assert(file_.is_open());
}

logger::pointer logger::get() {

  return __instance ? __instance
                    // Не очень хорошо использовать new для создания shared_ptr.
                    // Лучше бы сделать static-объект и возвращать на него raw-указатель,
                    // Но сейчас просто хочу сделать так - черещ new.
                    : __instance = std::shared_ptr<logger>(new logger());
}

void logger::print(const char* format, ...) {
  if (!format)
    return;

  std::lock_guard lock(mtx_);

  va_list vars;
  va_start(vars, format);

  vsprintf((char*)str_buf_.data(), format, vars);

  time_t date = time(0);
  char*  date_str = ctime(&date);
  date_str[strlen(date_str) - 1] = 0;

  file_ << date_str << " | " << str_buf_.data() << std::endl;
}