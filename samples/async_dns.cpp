//
// Created by v4kst1z.
//

#include <memory>

#include "../include/AsyncDns.h"
#include "../include/Ipv4Addr.h"
#include "../include/Logger.h"

int main() {
  auto dns = new AsyncDns();
  dns->AddDnsQuery("www.baidu.com");
  dns->AddDnsQuery("www.jd.com");
  dns->AddDnsQuery("www.stackoverflow.com");
  dns->StartLoop();
  while (true) {
    dns->PrintQuery();
  }

  return 0;
}