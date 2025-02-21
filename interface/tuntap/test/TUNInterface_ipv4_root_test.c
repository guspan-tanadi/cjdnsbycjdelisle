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
#include "interface/tuntap/TUNInterface.h"
#include "interface/tuntap/TUNMessageType.h"
#include "memory/Allocator.h"
#include "util/Assert.h"
#include "util/log/Log.h"
#include "util/log/FileWriterLog.h"
#include "wire/Error.h"
#include "wire/Ethernet.h"
#include "wire/Headers.h"
#include "util/platform/netdev/NetDev.h"
#include "test/RootTest.h"
#include "interface/tuntap/test/TUNTools.h"

// On loan from the DoD, thanks guys.
static const uint8_t testAddrA[4] = {11, 0, 0, 1};
static const uint8_t testAddrB[4] = {11, 0, 0, 2};

static Iface_DEFUN receiveMessageTUN(Message_t* msg, struct TUNTools* tt)
{
    uint16_t ethertype = -1;
    Err(TUNMessageType_pop(&ethertype, msg));
    if (ethertype != Ethernet_TYPE_IP4) {
        Log_debug(tt->log, "Spurious packet with ethertype [%u]\n",
                  Endian_bigEndianToHost16(ethertype));
        return Error(msg, "INVALID");
    }

    struct Headers_IP4Header* header = (struct Headers_IP4Header*) Message_bytes(msg);

    Assert_true(Message_getLength(msg) == Headers_IP4Header_SIZE + Headers_UDPHeader_SIZE + 12);

    Assert_true(!Bits_memcmp(header->destAddr, testAddrB, 4));
    Assert_true(!Bits_memcmp(header->sourceAddr, testAddrA, 4));

    Bits_memcpy(header->destAddr, testAddrA, 4);
    Bits_memcpy(header->sourceAddr, testAddrB, 4);

    Err(TUNMessageType_push(msg, ethertype));

    return Iface_next(&tt->tunIface, msg);
}

int main(int argc, char** argv)
{
    // TODO(cjd): fix TUNConfigurator_addIp4Address() for Illumos, BSD.
    #if defined(sunos) || defined(freebsd)
        return 0;
    #endif

    struct Allocator* alloc = Allocator_new(1<<20);
    EventBase_t* base = EventBase_new(alloc);
    struct Log* logger = FileWriterLog_new(stdout, alloc);

    struct Sockaddr* addrA = Sockaddr_fromBytes(testAddrA, Sockaddr_AF_INET, alloc);
    struct Sockaddr* addrB = Sockaddr_fromBytes(testAddrB, Sockaddr_AF_INET, alloc);

    char assignedIfName[TUNInterface_IFNAMSIZ];
    TUNInterface_t* tt = NULL;
    Err_assert(TUNInterface_new(&tt, NULL, assignedIfName, logger, alloc));
    addrA->flags |= Sockaddr_flags_PREFIX;
    addrA->prefix = 30;
    Err_assert(NetDev_addAddress(assignedIfName, addrA, logger, alloc));

    TUNTools_echoTest(addrA, addrB, receiveMessageTUN, tt->iface, base, logger, alloc);
    Allocator_free(alloc);
    return 0;
}
