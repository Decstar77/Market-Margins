
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <string>

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

    class RpcBuilder {
    public:
        RpcBuilder( RpcCall callId ) {
            Write( static_cast<void *>(&callId), sizeof( RpcCall ) );
        }

        template<typename _type_>
        inline void WriteArg( const _type_ & arg ) {
            static_assert(std::is_same_v< std::decay_t<_type_>, std::string > == false, "Arg cannot be std::string.");
            static_assert(std::is_same_v< std::decay_t<_type_>, std::string_view > == false, "Arg cannot be std::string_view.");
            static_assert(std::is_same_v< _type_, const char * > == false, "Arg cannot be const char *.");
            Write( static_cast<const void *> (&arg), sizeof( _type_ ) );
        }

        const void * GetData() const { return static_cast<const void *>(buffer); };
        i32             GetSize() const { return length; }

    private:
        inline void Write( const void * data, i32 size ) {
            memcpy( buffer + length, data, size );
            length += size;
        }

    private:
        byte buffer[256] = {};
        i32 length = 0;
    };
}



