/* vim: set expandtab ts=4 sw=4: */
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "interface/tuntap/TUNMessageType.h"
#include "interface/tuntap/test/TUNTools.h"
#include "rust/cjdns_sys/RTypes.h"
#include "util/events/UDPAddrIface.h"
#include "exception/Err.h"
#include "util/events/Timeout.h"
#include "wire/Ethernet.h"
#include "wire/Headers.h"
#include "wire/Error.h"

#ifdef win32
    #include <windows.h>
    #define sleep(x) Sleep(1000*x)
#else
    #include <unistd.h>
#endif

static AddrIface_t* setupUDP(EventBase_t* base,
                                  struct Sockaddr* bindAddr,
                                  struct Allocator* allocator,
                                  struct Log* logger)
{
    // Mac OSX and BSD do not set up their TUN devices synchronously.
    // We'll just keep on trying until this works.
    struct UDPAddrIface* udp = NULL;
    RTypes_Error_t* er = NULL;
    for (int i = 0; i < 20; i++) {
        er = UDPAddrIface_new(&udp, bindAddr, allocator);
        if (udp) {
            break;
        }
        sleep(1);
    }
    if (!udp) {
        if (er) {
            Err_assert(er);
        } else {
            Assert_failure("setupUDP unable to get working UDPAddrIface");
        }
    }
    return &udp->generic;
}

struct TUNTools_pvt
{
    struct TUNTools pub;
    Identity
};

static void sendHello(void* vctx)
{
    struct TUNTools_pvt* ctx = Identity_check((struct TUNTools_pvt*) vctx);
    struct Allocator* tempAlloc = Allocator_child(ctx->pub.alloc);
    Message_t* msg = Message_new(0, 64, tempAlloc);
    Err_assert(Message_epush(msg, "Hello World", 12));
    Err_assert(Message_epush(msg, ctx->pub.tunDestAddr, ctx->pub.tunDestAddr->addrLen));
    Iface_send(&ctx->pub.udpIface, msg);
    Allocator_free(tempAlloc);
}

const uint8_t* TUNTools_testIP6AddrA = (uint8_t[])
{
    0xfd,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1
};
const uint8_t* TUNTools_testIP6AddrB = (uint8_t[])
{
    0xfd,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2
};
const uint8_t* TUNTools_testIP6AddrC = (uint8_t[])
{
    0xfd,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5
};

Iface_DEFUN TUNTools_genericIP6Echo(Message_t* msg, struct TUNTools* tt)
{
    uint16_t ethertype = -1;
    Err(TUNMessageType_pop(&ethertype, msg));
    if (ethertype != Ethernet_TYPE_IP6) {
        Log_debug(tt->log, "Spurious packet with ethertype [%04x]\n",
                  Endian_bigEndianToHost16(ethertype));
        return Error(msg, "INVALID");
    }

    struct Headers_IP6Header* header = (struct Headers_IP6Header*) Message_bytes(msg);

    if (Message_getLength(msg) != Headers_IP6Header_SIZE + Headers_UDPHeader_SIZE + 12) {
        int type = (Message_getLength(msg) >= Headers_IP6Header_SIZE) ? header->nextHeader : -1;
        Log_debug(tt->log, "Message of unexpected length [%u] ip6->nextHeader: [%d]\n",
                  Message_getLength(msg), type);
        return Error(msg, "INVALID");
    }
    uint8_t* address;
    Sockaddr_getAddress(tt->tunDestAddr, &address);
    Assert_true(!Bits_memcmp(header->destinationAddr, address, 16));
    Sockaddr_getAddress(tt->udpBindTo, &address);
    Assert_true(!Bits_memcmp(header->sourceAddr, address, 16));

    Sockaddr_getAddress(tt->udpBindTo, &address);
    Bits_memcpy(header->destinationAddr, address, 16);
    Sockaddr_getAddress(tt->tunDestAddr, &address);
    Bits_memcpy(header->sourceAddr, address, 16);

    Err(TUNMessageType_push(msg, ethertype));

    return Iface_next(&tt->tunIface, msg);
}

static Iface_DEFUN receiveMessageTUN(Message_t* msg, struct Iface* tunIface)
{
    struct TUNTools_pvt* ctx = Identity_containerOf(tunIface, struct TUNTools_pvt, pub.tunIface);
    ctx->pub.receivedMessageTUNCount++;
    return ctx->pub.cb(msg, &ctx->pub);
}

static Iface_DEFUN receiveMessageUDP(Message_t* msg, struct Iface* udpIface)
{
    struct TUNTools_pvt* ctx = Identity_containerOf(udpIface, struct TUNTools_pvt, pub.udpIface);

    if (ctx->pub.receivedMessageTUNCount) {
    Log_debug(ctx->pub.log, "test complete");
        // Got the message, test successful, tear everything down by freeing the root alloc.
        EventBase_endLoop(ctx->pub.base);
    }

    return NULL;
}

static void fail(void* ignored)
{
    Assert_true(!"timeout");
}

void TUNTools_echoTest(struct Sockaddr* udpBindTo,
                       struct Sockaddr* tunDestAddr,
                       TUNTools_Callback tunMessageHandler,
                       struct Iface* tun,
                       EventBase_t* base,
                       struct Log* logger,
                       struct Allocator* allocator)
{
    struct Allocator* alloc = Allocator_child(allocator);
    AddrIface_t* udp = setupUDP(base, udpBindTo, alloc, logger);

    struct Sockaddr* dest = Sockaddr_clone(udp->addr, alloc);
    uint8_t* tunDestAddrBytes = NULL;
    uint8_t* udpDestPointer = NULL;
    int len = Sockaddr_getAddress(dest, &udpDestPointer);
    Assert_true(len && len == Sockaddr_getAddress(tunDestAddr, &tunDestAddrBytes));
    Bits_memcpy(udpDestPointer, tunDestAddrBytes, len);

    struct TUNTools_pvt* ctx = Allocator_calloc(alloc, sizeof(struct TUNTools_pvt), 1);
    Identity_set(ctx);
    ctx->pub.udpIface.send = receiveMessageUDP;
    ctx->pub.tunIface.send = receiveMessageTUN;
    Iface_plumb(&ctx->pub.udpIface, udp->iface);
    Iface_plumb(&ctx->pub.tunIface, tun);
    ctx->pub.cb = tunMessageHandler;
    ctx->pub.tunDestAddr = Sockaddr_clone(dest, alloc);
    ctx->pub.udpBindTo = Sockaddr_clone(udpBindTo, alloc);
    ctx->pub.alloc = alloc;
    ctx->pub.log = logger;
    ctx->pub.base = base;

    Timeout_setInterval(sendHello, ctx, 1000, base, alloc);
    Timeout_setTimeout(fail, NULL, 10000, base, alloc);

    EventBase_beginLoop(base);
}
