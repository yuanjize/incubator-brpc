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

// bthread - A M:N threading library to make applications more concurrent.

// Date: 2017/07/27 23:07:06

#ifndef BTHREAD_PARKING_LOT_H
#define BTHREAD_PARKING_LOT_H

#include "butil/atomicops.h"
#include "bthread/sys_futex.h"

namespace bthread {
/*
 * 首先TaskControl有一个ParkingLot数组，长度是固定的。
 * 每次创建一个TaskGroup的时候会随机从这个数组中给他们一个
 *TC在从TG steal_task的时候就将当时的（_PL）ParkingLot保存到_last_pl_state中。这两个成员属性都是TG的
 * TG在wait_task的时候不断执行下面两个操作，直到返回：
 *   1._pl->wait(_last_pl_state);
 *   2.steal_task 该操作中更新_last_pl_state
 *只要_last_pl_state和_pl相等，那么线程就会wait住。
 * 那么什么时候两者不相等呢？ 发生singalTask的时候，这个时候一般是在wait_Task期间新建立了TaskGROUP
 *
 * */
// Park idle workers.
class BAIDU_CACHELINE_ALIGNMENT ParkingLot {
public:
    class State {
    public:
        State(): val(0) {}
        bool stopped() const { return val & 1; }
    private:
    friend class ParkingLot;
        State(int val) : val(val) {}
        int val;
    };

    ParkingLot() : _pending_signal(0) {}

    // Wake up at most `num_task' workers.
    // Returns #workers woken up.

    //函数首先+n，然后唤醒N个的代的不知道什么东西
    int signal(int num_task) {// 这里左移动一位是因为最低位作为state使用
        _pending_signal.fetch_add((num_task << 1), butil::memory_order_release);
        return futex_wake_private(&_pending_signal, num_task);
    }

    // Get a state for later wait().
    State get_state() {
        return _pending_signal.load(butil::memory_order_acquire);
    }

    // Wait for tasks.
    // If the `expected_state' does not match, wait() may finish directly.
    void wait(const State& expected_state) {
        futex_wait_private(&_pending_signal, expected_state.val, NULL);
    }

    // Wakeup suspended wait() and make them unwaitable ever. 
    void stop() {
        _pending_signal.fetch_or(1);//最后一位置为1
        futex_wake_private(&_pending_signal, 10000);//唤醒所有，并且别的也不允许睡了
    }
private:
    // higher 31 bits for signalling, LSB for stopping.
    butil::atomic<int> _pending_signal;
};

}  // namespace bthread

#endif  // BTHREAD_PARKING_LOT_H
