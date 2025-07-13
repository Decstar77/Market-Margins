#pragma once

#include "fin-pch.h"

namespace fin {
    class ReplayEngine {
    public:
        ReplayEngine( const std::string & filename ) {
            this->filename = filename;
            file.open( filename, std::ios::binary | std::ios::out );
            if ( !file.is_open() ) {
                throw std::runtime_error( "Failed to open replay file" );
            }
        }

        ~ReplayEngine() {
            file.close();
        }

        template<typename _type_>
        inline void Append( const _type_ & t ) {
            Append( reinterpret_cast<const char *>(&t), sizeof( t ) );
        }

        inline void Append( const char * data, size_t size ) {
            file.write( data, size );
        }

        inline void Reset(){
            file.seekp( 0 );
        }

        //inline bool IsAtEnd() {
        //    return file.tellp() == file.tellg();
        //}

        //inline void Read( char * data, size_t size ) {
        //       file.read( data, size );
        //}

        //template<typename _type_>
        //inline void Read( _type_ & t ) {
        //    file.read( reinterpret_cast<char *>(&t), sizeof( t ) );
        //}

    private:
        std::ofstream file;
        std::string filename;
    };
}
