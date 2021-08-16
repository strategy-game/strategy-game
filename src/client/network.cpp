#ifdef unix
#	define _XOPEN_SOURCE_EXTENDED 1
#	include <netdb.h>
#	include <arpa/inet.h>
#endif

#include <sys/types.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <mutex>
/* Visual Studio does not know about UNISTD.H, Mingw does through */
#ifndef _MSC_VER
#	include <unistd.h>
#endif

#ifdef unix
#	include <poll.h>
#elif defined windows
/* MingW does not behave well with pollfd structures, however MSVC does */
#	ifndef _MSC_VER
typedef struct pollfd {
    SOCKET fd;
    SHORT events;
    SHORT revents;
} WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD;
WINSOCK_API_LINKAGE int WSAAPI WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout);
#	endif
#endif

#include "client/network.hpp"
#include "actions.hpp"
#include "unit.hpp"
#include "diplomacy.hpp"
#include "../world.hpp"
#include "../io_impl.hpp"

Client* g_client = nullptr;
Client::Client(std::string host, const unsigned port) {
    g_client = this;

    // Initialize WSA
#ifdef windows
    WSADATA data;
    if(WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        print_error("WSA code: %u", WSAGetLastError());
        throw std::runtime_error("Cannot start WSA");
    }
#endif
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host.c_str());
    addr.sin_port = htons(port);
    
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd == INVALID_SOCKET) {
#ifdef windows
        print_error("WSA Code: %u", WSAGetLastError());
        WSACleanup();
#endif
        throw std::runtime_error("Cannot create client socket");
    }
    
    if(connect(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
#ifdef unix
        close(fd);
#elif defined windows
        print_error("WSA Code: %u", WSAGetLastError());
        closesocket(fd);
#endif
        throw std::runtime_error("Cannot connect to server");
    }
    
    // Launch the receive and send thread
    net_thread = std::thread(&Client::net_loop, this);
    has_snapshot = false;
}

/** The server assumes all clients are able to handle all events regardless of anything
 * if the client runs out of memory it needs to disconnect and then reconnect in order
 * to establish a new connection; since the server won't hand out snapshots - wait...
 * if you need snapshots for any reason (like desyncs) you can request with ACTION_SNAPSHOT
 */
void Client::net_loop(void) {
    // Receive the first snapshot of the world
    Packet packet = Packet(fd);
    packet.recv();
    Archive ar = Archive();
    ar.set_buffer(packet.data(), packet.size());
    ::deserialize(ar, g_world);
    
    has_snapshot = true;
    
    try {
        enum ActionType action;
        
#ifdef unix
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
#endif
        while(1) {
            // Check if we need to read packets
#ifdef unix
            int has_pending = poll(&pfd, 1, 10);
#elif defined windows
            u_long has_pending = 0;
            ioctlsocket(fd, FIONREAD, &has_pending);
#endif

            // Conditional of above statements
#ifdef unix
            if(pfd.revents & POLLIN || has_pending) {
#elif defined windows
            if(has_pending) {
#endif
                // Obtain the action from the server
                packet.recv();
                ar.set_buffer(packet.data(), packet.size());
                ar.rewind();
                
                ::deserialize(ar, &action);
                
                // Ping from server, we should answer with a pong!
                switch(action) {
                case ACTION_PONG:
                print_info("ACTION_PONG");
                    packet.send(&action);
                    print_info("Received ping, responding with pong!");
                    break;
                /** Update/Remove/Add Actions
                 * These actions all follow the same format they give a specialized ID for the index
                 * where the operated object is or should be; this allows for extreme-level fuckery
                 * like ref-name changes in the middle of a game in the case of updates.
                 *
                 * After the ID the object in question is given in a serialized form, in which the
                 * deserializer will deserialize onto the final object; after this the operation
                 * desired is done. */
                case ACTION_NATION_UPDATE:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->nations_mutex);

                        Nation* nation;
                        ::deserialize(ar, &nation);
                        if(nation == nullptr)
                            throw ClientException("Unknown nation");
                        ::deserialize(ar, nation);
                    }
                    break;
                case ACTION_NATION_ENACT_POLICY:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->nations_mutex);

                        Nation* nation;
                        ::deserialize(ar, &nation);
                        if(nation == nullptr)
                            throw ClientException("Unknown nation");
                        Policies policy;
                        ::deserialize(ar, &policy);
                        nation->current_policy = policy;
                    }
                    break;
                case ACTION_PROVINCE_UPDATE:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->provinces_mutex);

                        Province* province;
                        ::deserialize(ar, &province);
                        if(province == nullptr)
                            throw ClientException("Unknown province");
                        ::deserialize(ar, province);
                    }
                    break;
                case ACTION_PRODUCT_UPDATE:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->products_mutex);

                        Product* product;
                        ::deserialize(ar, &product);
                        if(product == nullptr)
                            throw ClientException("Unknown product");
                        ::deserialize(ar, product);
                    }
                    break;
                case ACTION_UNIT_UPDATE:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->units_mutex);

                        Unit* unit;
                        ::deserialize(ar, &unit);
                        if(unit == nullptr)
                            throw ClientException("Unknown unit");
                        ::deserialize(ar, unit);
                    }
                    break;
                case ACTION_UNIT_ADD:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->units_mutex);

                        Unit* unit = new Unit();
                        ::deserialize(ar, unit);
                        g_world->units.push_back(unit);
                        print_info("New unit of %s", unit->owner->name.c_str());
                    }
                    break;
                case ACTION_BOAT_UPDATE:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->boats_mutex);

                        Boat* boat;
                        ::deserialize(ar, &boat);
                        if(boat == nullptr)
                            throw ClientException("Unknown boat");
                        ::deserialize(ar, boat);
                    }
                    break;
                case ACTION_BOAT_ADD:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->boats_mutex);

                        Boat* boat = new Boat();
                        ::deserialize(ar, boat);
                        g_world->boats.push_back(boat);
                        print_info("New boat of %s", boat->owner->name.c_str());
                    }
                    break;
                case ACTION_OUTPOST_UPDATE:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->outposts_mutex);

                        Outpost* outpost;
                        ::deserialize(ar, &outpost);
                        if(outpost == nullptr)
                            throw ClientException("Unknown outpost");
                        ::deserialize(ar, outpost);
                    }
                    break;
                case ACTION_OUTPOST_ADD:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->outposts_mutex);

                        Outpost* outpost = new Outpost();
                        ::deserialize(ar, outpost);
                        g_world->outposts.push_back(outpost);
                        print_info("New outpost of %s", outpost->owner->name.c_str());
                    }
                    break;
                case ACTION_TREATY_ADD:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->treaties_mutex);

                        Treaty* treaty = new Treaty();
                        ::deserialize(ar, treaty);
                        g_world->treaties.push_back(treaty);
                        print_info("New treaty from %s", treaty->sender->name.c_str());
                        for(const auto& status: treaty->approval_status) {
                            print_info("- %s", status.first->name.c_str());
                        }
                    }
                    break;
                case ACTION_WORLD_TICK:
                    {
                        std::lock_guard<std::recursive_mutex> l1(g_world->time_mutex);
                        ::deserialize(ar, &g_world->time);
                    }
                    break;
                case ACTION_TILE_UPDATE:
                    {
                        std::lock_guard<std::recursive_mutex> lock(g_world->tiles_mutex);
                        std::pair<size_t, size_t> coord;
                        ::deserialize(ar, &coord.first);
                        ::deserialize(ar, &coord.second);
                        ::deserialize(ar, &g_world->get_tile(coord.first, coord.second));
                    }
                    break;
                default:
                    break;
                }
            }
            
            // Client will also flush it's queue to the server
            packet_mutex.lock();
            while(!packet_queue.empty()) {
                print_info("Network: sending packet");
                Packet elem = packet_queue.front();
                packet_queue.pop_front();

                elem.stream = SocketStream(fd);
                elem.send();
            }
            packet_queue.clear();
            packet_mutex.unlock();
        }
    } catch(ClientException& e) {
        print_error("Except: %s", e.what());
    }
}

/** Waits to receive the server initial world snapshot
 */
void Client::wait_for_snapshot(void) {
    while(!has_snapshot) {
        // Just wait...
    }
}

Client::~Client() {
#ifdef windows
    closesocket(fd);
    WSACleanup();
#else
    close(fd);
#endif
    net_thread.join();
}
