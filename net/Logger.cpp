//
// Created by v4kst1z.
//

#include <Logger.h>

Logger &Logger::GetInstance() {
  static Logger log;
  return log;
}

Logger::LogStream Logger::operator()(const char *file_name, int line, const char *func_name, Level level) {
  Logger::LogStream stream(level, this);
  stream.stream_ << GetCurrentLogLevel(level) << GetCurrentDateTime() << file_name << " " << line << " " << func_name
                 << " ";
  return std::move(stream);
}

void Logger::Start() {
  log_thread_ = std::thread(&Logger::Loop, this);
}

void Logger::Loop() {
  while (true) {
    if (quit_ && queue_data_->Empty()) break;
    std::unique_ptr<Message> data = queue_data_->WaitPop();
    if (!data->GetTerminal())
      std::cout << data->GetData() << std::endl;
  }
}

void Logger::SetQuit(bool quit) {
  quit_ = quit;
}

Logger::Logger() :
    quit_(false),
    queue_data_(new SafeQueue<Message>()),
    terminal_(false) {}

const std::string Logger::GetCurrentDateTime() {
  time_t now = time(NULL);
  struct tm *tm_s = localtime(&now);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X\t", tm_s);
  return buf;
}

Message::Message(const std::string &data, bool terminal) :
    data_(std::move(data)),
    terminal_(terminal) {}

const std::string &Message::GetData() {
  return data_;
}

bool Message::GetTerminal() {
  return terminal_;
}

Logger::LogStream::LogStream(Level level, Logger *log)
    : level_(level),
      log_(log) {}

Logger::LogStream::LogStream(Logger::LogStream &&rlog) noexcept
    : stream_(std::move(rlog.stream_)),
      level_(rlog.level_),
      log_(rlog.log_) {
  rlog.log_ = nullptr;
}

void Logger::LogStream::AppendLog(const std::string &log, bool terminal) {
  log_->queue_data_->Push(make_unique<Message>(log, terminal));
}

Logger::LogStream::~LogStream() {
  if (!stream_.str().empty()) {
    AppendLog(stream_.str(), log_->terminal_);
  }
}