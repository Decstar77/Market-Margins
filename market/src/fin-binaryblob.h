#pragma once

#include "fin-pch.h"

namespace fin {
    template<i32 _size_>
    class FixedBinaryBlob {
    public:
        inline void Read( void * dst, i32 size ) {
            Assert( current + size < _size_ );
            std::memcpy( dst, buffer.data() + current, size );
            current += size;
        }

        inline i32 Write( const void * data, i32 size ) {
            Assert( current + size < _size_ );
            std::memcpy( buffer.data() + current, data, size );
            current += size;
            return size;
        }

        template<typename _type_>
        inline void Read( _type_ * obj ) {
            Read( (void *)obj, (i32)sizeof( _type_ ) );
        }

        template<typename _type_>
        inline i32 Write( const _type_ * obj ) {
            static_assert( std::is_pointer<_type_>::value == false, "Highly likely bug to use BinaryFile::Push on pointer type!" );
            return Write( (void *)obj, (i32)sizeof( _type_ ) );
        }

        inline void Reset() {
            current = 0;
        }

        inline i32 GetSize() {
            return current;
        }

        byte * Get() {
            return buffer.data();
        }

        const byte * Get() const {
            return buffer.data();
        }

    private:
        i32 current = 0;
        std::array<byte, _size_> buffer = {};
    };
}