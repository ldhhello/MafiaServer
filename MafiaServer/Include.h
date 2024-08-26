#pragma once

// STD Library
#include <iostream>

#include <string>
#include <thread>
#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <map>
#include <random>

#include <climits>

using namespace std;

// Boost Library
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/optional.hpp>

#include "Logger.h"

// Defines
using Port = unsigned short;
namespace asio = boost::asio;

//using SSLStream = asio::ssl::stream<asio::ip::tcp::socket>;
using SSLSocket = asio::ssl::stream<asio::ip::tcp::socket>;
using Socket = SSLSocket;//asio::ip::tcp::socket;
using socket_type = asio::detail::socket_type;

const int THREAD_COUNT = 50;
const int MAX_ROOM_NUMBER = 1000;

extern asio::io_context context[THREAD_COUNT];
//extern asio::io_context room_context[THREAD_COUNT];

#define IGNORE_EXCEPTION(c) try { c; } catch(...) {}

#define CHECK_PACKET_SIZE(s)     \
if (recv_packet.size() != s)     \
{                                \
	close();                     \
	return;                      \
}

#define HANDLE_METHOD(method, function, size) \
case method:                                  \
	CHECK_PACKET_SIZE(size);                  \
	function();                               \
	break;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define GCC_VERSION STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)

#if defined DEBUG
#define BUILD "Debug"
#else
#define BUILD "Release"
#endif

#define SERVER_NAME "Mafia_Server"
#define MAFIA_PROTOCOL_VERSION "3.0"
