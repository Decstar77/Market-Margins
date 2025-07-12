#pragma once

// This is a precomiled header

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <deque>
#include <stack>
#include <list>
#include <array>
#include <span>
#include <thread>
#include <atomic>
#include <tuple>
#include <type_traits>
#include <functional>
#include <json/json.hpp>

typedef uint64_t u64;
typedef int64_t i64;
typedef int32_t i32;
typedef uint8_t byte;
typedef double f64;

#define Assert( condition ) if( !(condition) ) { throw std::runtime_error( "Assertion failed: " + std::string( #condition ) ); }
#define AssertMsg( condition, message ) if( !(condition) ) { throw std::runtime_error( "Assertion failed: " + std::string( #condition ) + " " + message ); }
#define AssertMsgf( condition, message, ... ) if( !(condition) ) { throw std::runtime_error( "Assertion failed: " + std::string( #condition ) + " " + message ); }
#define AssertMsgf( condition, message, ... ) if( !(condition) ) { throw std::runtime_error( "Assertion failed: " + std::string( #condition ) + " " + message ); }