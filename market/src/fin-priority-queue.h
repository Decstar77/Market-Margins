#pragma once

#include "fin-pch.h"

namespace fin {
    template<typename _type_>
    class PriorityQueue {
    public:
        PriorityQueue( std::function<const _type_ * (const _type_ *, const _type_ *)> comparer ) : comparer( comparer ) {

        }

        void Push( const _type_ & type ) {
            const u64 t = data.size();
            data.push_back( type );
            ShiftUp( t );
        }

        bool Peek( _type_ *& t ) {
            const size_t size = data.size();
            if ( size == 0 ) {
                return false;
            }

            t = &data[0];
            return true;
        }

        bool PeekCopy( _type_ & t ) const {
            const size_t size = data.size();
            if ( size == 0 ) {
                return false;
            }

            t = data[0];
            return true;
        }

        bool Pop( _type_ & t ) {
            const u64 size = data.size();
            if ( size == 0 ) {
                return false;
            }

            t = std::move( data[0] );
            data[0] = data[size - 1];
            data.pop_back();
            if ( size - 1 > 0 ) {
                ShiftDown( 0 );
            }

            return true;
        }

        bool Pop() {
            const size_t size = data.size();
            if ( size == 0 ) {
                return false;
            }

            data[0] = data[size - 1];
            data.pop_back();
            if ( data.size() > 0 ) {
                ShiftDown( 0 );
            }
            return true;
        }

        bool RemoveOnce( std::function< bool( const _type_ * ) > check ) {
            const size_t size = data.size();
            for ( size_t i = 0; i < size; i++ ) {
                const _type_ * v = &data[i];
                if ( check( v ) == true ) {
                    data[i] = data[size - 1];
                    data.pop_back();
                    if ( i < size - 1 ) {
                        ShiftDown( i );
                    }
                    return true;
                }
            }

            return false;
        }

        inline size_t Size() { return data.size(); }
        inline size_t Size() const { return data.size(); }
        inline  _type_ * Data() { return data.data(); }
        inline const _type_ * Data() const { return data.data(); }

    private:
        inline u64 GetLeft( u64 i ) { return ((i * 2) + 1); }
        inline u64 GetRight( u64 i ) { return ((i * 2) + 2); }
        inline u64 GetParent( u64 i ) { return ((i - 1) / 2); }

        void ShiftDown( u64 index ) {
            while ( true ) {
                const u64 size = data.size();
                const u64 li = GetLeft( index );
                const u64 ri = GetRight( index );

                const _type_ * pl = li < size ? &data[li] : nullptr;
                const _type_ * pr = ri < size ? &data[ri] : nullptr;
                const _type_ * picked = Compare( pl, pr );

                const _type_ * pic = &data[index];
                if ( Compare( pic, picked ) == pic ) {
                    break;
                }

                const u64 ci = picked == pl ? li : ri;
                std::swap( data[ci], data[index] );
                index = ci;
            }
        }

        void ShiftUp( u64 index ) {
            while ( index > 0 ) {
                const u64 pindex = GetParent( index );
                const _type_ * pin = &data[index];
                const _type_ * pot = &data[pindex];
                if ( Compare( pin, pot ) == pin ) {
                    std::swap( data[index], data[pindex] );
                    index = pindex;
                }
                else {
                    break;
                }
            }
        }

        inline const _type_ * Compare( const _type_ * a, const _type_ * b ) {
            if ( a == nullptr ) { return b; }
            if ( b == nullptr ) { return a; }
            return comparer( a, b );
        }

    private:
        std::vector<_type_> data;
        std::function<const _type_ * (const _type_ *, const _type_ *)> comparer;
    };

    namespace tests {
        void RunPriorityQueueTests();
    }
}
