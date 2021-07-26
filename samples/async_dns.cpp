//
// Created by v4kst1z.
//

#include "../include/AsyncDns.h"

int main() {
  AsyncDns dns = AsyncDns();
  dns.AddDnsQuery("www.baidu.com", [](std::string ip) {
    std::cout << "www.baidu.com ip is " << ip << std::endl;
  });
  dns.AddDnsQuery("www.jd.com", [](std::string ip) {
    std::cout << "www.jd.com ip is " << ip << std::endl;
  });
  dns.AddDnsQuery("www.zhihu.com", [](std::string ip) {
    std::cout << "www.zhihu.com ip is " << ip << std::endl;
  });
  dns.Loop();
  return 0;
}