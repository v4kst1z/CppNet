//
// Created by v4kst1z.
//

#ifndef CPPNET_NET_LOGGER_H_
#define CPPNET_NET_LOGGER_H_

#include <sstream>
#include <thread>

#include <Common.h>
#include <SafeQueue.h>

enum class Level {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL
};

struct Message {
 public:
  explicit Message(const std::string &data, bool terminal = false);

  const std::string &GetData();

  bool GetTerminal();
  DISALLOW_COPY_AND_ASSIGN(Message);
 private:
  std::string data_;
  bool terminal_;
};

class Logger {
 public:
  static Logger &GetInstance();

  struct LogStream {
   public:
    LogStream(Level level, Logger *log);

    LogStream(LogStream &&rlog) noexcept;

    void AppendLog(const std::string &log, bool terminal = false);

    ~LogStream();

    std::ostringstream stream_;
    Level level_;
    Logger *log_;
  };

  LogStream operator()(const char *file_name, int line, const char *func_name, Level level);

  void Start();

  void Loop();

  void SetQuit(bool quit);

  ~Logger() = default;

  DISALLOW_COPY_AND_ASSIGN(Logger);
 private:
  Logger();

  const std::string GetCurrentLogLevel(Level level) const {
    switch (level) {
      case Level::DEBUG : return "[DEBUG]\t";
      case Level::ERROR: return "[ERROR]\t";
      case Level::INFO: return "[INFO]\t";
      case Level::FATAL: return "[FATAL]\t";
      case Level::WARN: return "[WARN]\t";
    }
  }

  const std::string GetCurrentDateTime();

  bool quit_;
  std::unique_ptr<SafeQueue<Message>> queue_data_;
  std::thread log_thread_;
  bool terminal_;

  friend class LogStream;
};

static Logger &log = Logger::GetInstance();

#define DEBUG log(__FILE__, __LINE__,  __func__, Level::DEBUG).stream_
#define ERROR log(__FILE__, __LINE__,  __func__, Level::ERROR).stream_
#define INFO log(__FILE__, __LINE__,  __func__, Level::INFO).stream_
#define FATAL log(__FILE__, __LINE__,  __func__, Level::FATAL).stream_
#define WARN log(__FILE__, __LINE__,  __func__, Level::WARN).stream_

#endif //CPPNET_NET_LOGGER_H_
