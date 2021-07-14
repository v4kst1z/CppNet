# CppNet
`C++11` 网络库，仅支持 `Linux`，

## Feature
基于 `Epoll` 的 `IO` 复用机制，采用边缘触发模式。通过使用 `SO_REUSEPORT`，在每个 `IO` 线程创建创建 `listening socket`，降低在存在大量请求时的响应时间。支持定时器，异步日志，线程池，`TCP`， `UDP`，及异步 `DNS` 查询。
