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
#include "interface/Iface.h"
#include "interface/tuntap/SocketWrapper.h"
#include "memory/Allocator.h"
#include "util/Identity.h"
#include "wire/Message.h"
#include "wire/Error.h"

struct SocketWrapper_pvt
{
    struct SocketWrapper pub;
    struct Log* logger;
    Identity
};

static Iface_DEFUN incomingFromSocket(Message_t* msg, struct Iface* externalIf)
{
    struct SocketWrapper_pvt* ctx =
        Identity_containerOf(externalIf, struct SocketWrapper_pvt, pub.externalIf);

    if (!ctx->pub.internalIf.connectedIf) {
        Log_debug(ctx->logger, "DROP message for socket not inited");
        return Error(msg, "INVALID");
    }

    // get ess packet type
    uint8_t type = 0;
    Err(Message_epop8h(&type, msg));
    Log_debug(ctx->logger, "Packet type [%d]", type);

    if (type == SocketWrapper_TYPE_TUN_PACKET) {
        // skip tun packet length
        uint32_t discard = 0;
        Err(Message_epop32be(&discard, msg));
        return Iface_next(&ctx->pub.internalIf, msg);
    }

    // skip all other types
    return Error(msg, "INVALID");
}

static Iface_DEFUN incomingFromUs(Message_t* msg, struct Iface* internalIf)
{
    struct SocketWrapper_pvt* ctx =
        Identity_containerOf(internalIf, struct SocketWrapper_pvt, pub.internalIf);

    if (!ctx->pub.externalIf.connectedIf) {
        Log_debug(ctx->logger, "DROP message for socket not inited");
        return Error(msg, "INVALID");
    }

    // send payload length
    Err(Message_epush32be(msg, Message_getLength(msg)));
    // mark this as a normal tun packet
    Err(Message_epush8(msg, SocketWrapper_TYPE_TUN_PACKET));

    return Iface_next(&ctx->pub.externalIf, msg);
}

struct SocketWrapper* SocketWrapper_new(struct Allocator* alloc, struct Log* log)
{
    struct SocketWrapper_pvt* context =
        Allocator_calloc(alloc, sizeof(struct SocketWrapper_pvt), 1);
    Identity_set(context);
    context->pub.externalIf.send = incomingFromSocket;
    context->pub.internalIf.send = incomingFromUs;
    context->logger = log;

    return &context->pub;
}

Err_DEFUN SocketWrapper_addAddress(struct Iface* rawSocketIf,
                                uint8_t* ipv6Addr,
                                struct Log* logger,
                                struct Allocator* alloc)
{
    size_t len = 16 /* IPv6 Address length */ + 1 /* Type prefix length */;
    Message_t* out = Message_new(0, len, alloc);
    Err(Message_epush(out, ipv6Addr, 16));
    Err(Message_epush8(out, SocketWrapper_TYPE_CONF_ADD_IPV6_ADDRESS));

    return Iface_next(rawSocketIf, out);
}

Err_DEFUN SocketWrapper_setMTU(struct Iface* rawSocketIf,
                            uint32_t mtu,
                            struct Log* logger,
                            struct Allocator* alloc)
{
    size_t len = 4 /* MTU var size */ + 1 /* Type prefix length */;
    Message_t* out = Message_new(0, len, alloc);
    Err(Message_epush32be(out, mtu));
    Err(Message_epush8(out, SocketWrapper_TYPE_CONF_SET_MTU));

    return Iface_next(rawSocketIf, out);
}