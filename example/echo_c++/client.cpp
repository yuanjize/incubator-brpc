// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// A client sending requests to server every 1 second.

#include <bthread/work_stealing_queue.h>
#include "iostream"

using namespace bthread;

int main(int argc, char *argv[]) {
    WorkStealingQueue<int> wsq;
    wsq.init(8);
    int i = 0;
    while (wsq.push(i)) {
        i++;
        std::cout << i << std::endl;
    }
    int s = 0;
    while (wsq.steal(&s)) {
        std::cout << s << std::endl;
    }

    std::cout << "===========" << std::endl;
    butil::atomic<size_t> BAIDU_CACHELINE_ALIGNMENT _top;
    _top = 10;
    size_t t = 100;
    bool r = _top.compare_exchange_strong(t, t + 200, butil::memory_order_relaxed) ;
    std::cout << _top << std::endl;
    std::cout << t << std::endl;
    std::cout << r << std::endl;

    return 0;
}
