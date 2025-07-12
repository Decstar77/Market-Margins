#pragma once

#include "fin-pch.h"
#include "fin-binaryblob.h"

namespace fin {
    template<i32 size>
    consteval std::array<size_t, size> ZeroBasedPrefixSum( const std::array<size_t, size> & arr ) {
        static_assert(size > 0, "PrefixSum must have array size greater than 0");
        std::array<size_t, size> result = {};
        result[0] = 0;
        for ( int i = 1; i < size; i++ ) {
            result[i] = result[i - 1] + arr[i - 1];
        }
        return result;
    }

    struct RpcString {
        char data[32];
    };

    class BinaryBlob {
    public:
        std::vector<byte> data;

        inline void Write( const void * data, size_t size ) {
            this->data.insert( this->data.end(), (byte *)data, (byte *)data + size );
        }

        template<typename _type_>
        inline void Write( const _type_ & data ) {
            Write( &data, sizeof( data ) );
        }
    };

    template<typename _type_>
    struct is_vector {
        static const bool value = false;
    };

    template<typename _type_>
    struct is_vector< std::vector< _type_ > > {
        static const bool value = true;
    };

    class RpcHolder {
    public:
        virtual void Call( const byte * data ) = 0;
    };

    template<typename _type_>
    concept RpcValidArg =
        !std::is_pointer_v<_type_> &&
        !is_vector<std::decay_t<_type_>>::value &&
        !std::is_same_v<std::decay_t<_type_>, std::string> &&
        !std::is_same_v<std::decay_t<_type_>, std::string_view>;

    template<typename _ret_, RpcValidArg ... _args_>
    class RpcBase : public RpcHolder {
    public:
        virtual _ret_ DoCall( _args_ ... args ) = 0;

        template<typename _type_>
        inline _type_ Convert( const size_t argIndex, const std::array< size_t, sizeof...(_args_) > & offsets, const byte * data ) {
            static_assert(std::is_pointer_v<_type_> == false, "RpcBase :: Argument can't be a pointer");
            const size_t dataOffset = offsets[argIndex];
            const _type_ result = *(_type_ *)(data + dataOffset);
            return result;
        }

        template<size_t... S>
        inline _ret_ CallImpl( std::index_sequence<S...>, const std::array< size_t, sizeof...(_args_) > & offsets, const byte * data ) {
            return DoCall( Convert<_args_>( S, offsets, data )... );
        }

        virtual _ret_ Call( const byte * data ) override {
            constexpr auto seq = std::index_sequence_for< _args_... >{};
            constexpr auto sizes = std::array< size_t, sizeof...(_args_) > { sizeof( _args_ )... };
            constexpr auto offsets = ZeroBasedPrefixSum< sizeof...(_args_) >( sizes );
            return CallImpl( seq, offsets, data );
        };
    };

    template<typename _ret_, typename ... _args_>
    class RpcFunction : public RpcBase<_ret_, _args_...> {
    public:
        static_assert((... && !std::is_pointer_v<_args_>));

        RpcFunction( std::function<_ret_( _args_... )> func ) { this->func = func; }

        virtual _ret_ DoCall( _args_... args ) override {
            return func( std::forward<_args_>( args )... );
        }

    private:
        std::function<_ret_( _args_... )> func;
    };

    class RpcCallData {
    public:
        RpcCallData() {

        }

        RpcCallData( const void * data, i32 length ) {
            this->data.Write( data, length );
        }

        template<typename... _types_>
        inline void AddCall( i32 funcId, _types_... args ) {
            data.Write( &funcId );
            DoSerialize( args... );
        }

        inline const byte * Get() const {
            return data.Get();
        }

    private:
        template< typename _type_ >
        inline void WriteSerializable( _type_ type ) {
            static_assert(std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers");
            static_assert(is_vector<_type_>::value == false, "RpcBuffer cannot take Raw Lists, please cast/pass in a Span : list.GetSpan()");
            data.Write( &type );
        }

        template< typename... _types_ >
        inline void DoSerialize( _types_... args ) {
            static_assert((... && !std::is_pointer_v<std::decay_t<_types_>>), "AddAction :: Cannot take pointers");
            static_assert((... && !is_vector<std::decay_t<_types_>>::value), "RpcBuffer cannot take Raw Lists, please cast/pass in a Span : list.GetSpan()");
            static_assert((... && !std::is_same_v<std::decay_t<_types_>, std::string>), "RpcBuffer can't use string/string_view, please use StringHolder");
            (WriteSerializable( std::forward<_types_>( args ) ), ...);
        }

    private:
        FixedBinaryBlob<256> data;
    };

    class RpcTable {
    public:
        template<typename _ret_, typename ... _args_>
        void Register( i32 id, RpcFunction<_ret_, _args_...> * holder ) {
            table[id] = holder;
        }

        void Call( const RpcCallData & buffer ) {
            const byte * callData = buffer.Get();
            const i32 funcId = *(reinterpret_cast<const i32 *>(callData));
            if ( table.contains( funcId ) == true ) {
                const byte * offset = reinterpret_cast<const byte *>(callData) + sizeof( i32 );
                table[funcId]->Call( offset );
            }
            else {
                
            }
        }

        std::unordered_map< i32, RpcHolder * > table = {};
    };
}
