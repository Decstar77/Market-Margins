#pragma once

#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace fin {
    constexpr std::size_t cacheline = std::hardware_destructive_interference_size;

    template<typename _type_>
    class SPSCQueue {
    public:
        SPSCQueue( int size ) : readPos( 0 ), writePos( 0 ), data( size + 1 ) {
        }

        bool Push( const _type_ & d ) {
            const int rp = readPos.load( std::memory_order_acquire );
            const int wp = writePos.load( std::memory_order_relaxed );
            const int p = Inc( wp );
            if ( p == rp ) {
                return false;
            }

            data[wp] = d;
            writePos.store( p, std::memory_order_release );
            return true;
        }

        bool Pop( _type_ & d ) {
            const int wp = writePos.load( std::memory_order_acquire );
            const int rp = readPos.load( std::memory_order_relaxed );
            if ( rp == wp ) {
                return false;
            }

            d = data[rp];
            readPos.store( Inc( rp ), std::memory_order_release );
            return true;
        }

    private:
        inline i32 Inc( int i ) { return (i + 1) % data.size(); }

        alignas(cacheline) std::atomic<i32> readPos;
        alignas(cacheline) std::atomic<i32> writePos;
        alignas(cacheline) std::vector<_type_> data;
    };

    template<typename _type_>
    class WaitQueue {
    public:
        void Push( const _type_ & t ) {
            std::lock_guard<std::mutex> lock( mutex );
            q.push( t );
            cv.notify_one();
        }

        void Pop( _type_ & t ) {
            std::unique_lock<std::mutex> lock( mutex );
            cv.wait( lock, [this]() { return q.empty() == false; } );
            t = q.front();
            q.pop();
        }

    private:
        std::queue<_type_> q;
        std::mutex mutex;
        std::condition_variable cv;
    };
}
