
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <string>
#include <random>
#include <memory>

namespace fin {
    typedef uint8_t byte;
    typedef int64_t i64;
    typedef int32_t i32;
    typedef int16_t i16;
    typedef int8_t i8;
    typedef uint64_t u64;
    typedef uint32_t u32;
    typedef uint16_t u16;

    enum OrderType {
        MarketOrder,
        LimitOrder
    };

    class Symbol {
    public:
        Symbol() {
            memset( data, 0, 4 );
        }

        inline Symbol( const char * symbol ) {
            memcpy( data, symbol, 4 );
        }

        inline Symbol( const std::string & symbol ) {
            memcpy( data, symbol.c_str(), 4 );
        }

        inline int AsInt() const { return *reinterpret_cast< const int* >( &data ); }

        inline bool operator==( const Symbol & other ) const {
            return AsInt() == other.AsInt();
        }

        inline bool operator!=( const Symbol & other ) const {
            return AsInt() != other.AsInt();
        }

        inline std::string AsString() const {
            return std::string( data, 4 );
        }

    private:
        char data[4];
    };

    struct OrderEntry {
        i64         id;
        i64         time;
        i64         price;
        i64         quantity;
        OrderType   type;
        Symbol      symbol;
    };

    enum class RpcCall {
        Invalid = 0,
        PlaceOrder_Bid = 1,
        PlaceOrder_Ask = 2,
        CancelOrder_Bid = 3,
        CancelOrder_Ask = 4,
    };

    typedef uint64_t NetId;

    class PacketBuffer {
    public:
        inline void Write( const void * data, i32 size ) {
            memcpy( this->data + this->length, data, size );
            this->length += size;
        }

        template<typename _type_>
        inline void Write( const _type_ & arg ) {
            static_assert(std::is_same_v< std::decay_t<_type_>, std::string > == false, "Arg cannot be std::string.");
            static_assert(std::is_same_v< std::decay_t<_type_>, std::string_view > == false, "Arg cannot be std::string_view.");
            static_assert(std::is_same_v< _type_, const char * > == false, "Arg cannot be const char *.");
            Write( static_cast<const void *> (&arg), sizeof( _type_ ) );
        }

        inline void Read( void * data, i32 size ) {
            memcpy( data, this->data + this->length, size );
            this->length += size;
        }

        template<typename _type_>
        inline void Read( _type_ & arg ) {
            Read( static_cast<void *> (&arg), sizeof( _type_ ) );
        }

        inline void Clear() {
            this->length = 0;
        }

    public:
        NetId   id = 0;
        i32     length = 0;
        char    data[256] = {};
    };

    class RpcCallData {
    public:
        RpcCallData() {

        }

        RpcCallData( const void * data, i32 length ) {
            Write( data, length );
        }

        template<typename... _types_>
        inline void Call( i32 funcId, _types_... args ) {
            length = 1; // Make room for the size.
            Write( &funcId, sizeof( funcId ) );
            DoSerialize( args... );
            buffer[0] = static_cast<byte>( length - 1 );
        }

        inline const byte * GetFullBuffer() const { return buffer; }
        inline i32          GetFullLength() const { return length; }

        inline const byte * GetCallBuffer() const { return buffer + 1; }
        inline i32          GetCallLength() const { return length - 1; }

    private:
        template< typename _type_ >
        inline void WriteSerializable( _type_ type ) {
            static_assert(std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers");
            Write( &type, sizeof( _type_ ) );
        }

        template< typename... _types_ >
        inline void DoSerialize( _types_... args ) {
            static_assert((... && !std::is_pointer_v<std::decay_t<_types_>>), "AddAction :: Cannot take pointers");
            static_assert((... && !std::is_same_v<std::decay_t<_types_>, std::string>), "RpcBuffer can't use string/string_view, please use StringHolder");
            (WriteSerializable( std::forward<_types_>( args ) ), ...);
        }

        inline void Write( const void * data, i32 length ) {
            memcpy( buffer + this->length, data, length );
            this->length += length;
        }

    private:
        i32 length = 0;
        byte buffer[256] = {};
    };
}



