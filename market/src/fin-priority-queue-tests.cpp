#pragma once

#include "fin-pch.h"
#include "fin-priority-queue.h"

namespace fin::tests {
    void PriorityQueue_Adds() {
        PriorityQueue<int> queue( []( const int * a, const int * b ) { return *a > *b ? a : b; } );
        queue.Push( 3 );
        queue.Push( 2 );
        queue.Push( 6 );

        const size_t size = queue.Size();
        const int * data = queue.Data();

        std::cout << "=====PriorityQueue_Adds=====" << std::endl;
        for ( size_t i = 0; i < size; i++ ) {
            std::cout << data[i] << std::endl;
        }
    }

    void PriorityQueue_Removes() {
        PriorityQueue<int> queue( []( const int * a, const int * b ) { return *a > *b ? a : b; } );
        queue.Push( 3 );
        queue.Push( 2 );
        queue.Push( 6 );

        queue.Pop();

        const size_t size = queue.Size();
        const int * data = queue.Data();

        std::cout << "=====PriorityQueue_Removes=====" << std::endl;
        for ( size_t i = 0; i < size; i++ ) {
            std::cout << data[i] << std::endl;
        }
    }

    void PriorityQueue_RemoveOnce() {
        PriorityQueue<int> queue( []( const int * a, const int * b ) { return *a > *b ? a : b; } );
        queue.Push( 3 );
        queue.Push( 2 );
        queue.Push( 6 );
        queue.Push( 4 );
        queue.Push( 1 );
        queue.Push( 2 );
        queue.Push( 7 );
        queue.Push( 9 );

        queue.RemoveOnce( []( const int * v ) { return *v == 3; } );

        const size_t size = queue.Size();
        const int * data = queue.Data();

        std::cout << "=====PriorityQueue_RemoveOnce=====" << std::endl;
        for ( size_t i = 0; i < size; i++ ) {
            std::cout << data[i] << std::endl;
        }
    }

    void RunPriorityQueueTests() {
        PriorityQueue_Adds();
        PriorityQueue_Removes();
        PriorityQueue_RemoveOnce();
    }
}

