#ifndef VOTERNN_STD_INCLUDE
#define VOTERNN_STD_INCLUDE

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <list>
#include <expected>
#include <cstring>
#include <functional>
#include <cstddef>
#include <cstdlib>
#include <array>
#include <span>
#include <string_view>
#include <variant>
#include <unordered_set>

#include <curl/curl.h> //CURL
#include "json.hpp" //Nlohmann

#include <ncurses.h>
#include <form.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netdb.h> // addrinfo
#include <unistd.h> // close()
#include <fcntl.h> // async
#include <poll.h>

#define NOOP (void)0

#define VNN_MAJOR 0
#define VNN_MINOR 1
#define VNN_BUILD 1

#define I_VNN_STRINGIFY_RAW(x) #x 
#define VNN_STRINGIFY(x) I_VNN_STRINGIFY_RAW(x)

#endif
