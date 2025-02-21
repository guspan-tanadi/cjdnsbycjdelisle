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
#include "crypto/random/Random.h"
#include "crypto/Ca.h"
#include "memory/Allocator.h"
#include "switch/SwitchCore.h"
#include "net/NetCore.h"
#include "util/log/Log.h"
#include "util/events/EventBase.h"
#include "net/SwitchPinger.h"
#include "net/ControlHandler.h"
#include "net/InterfaceController.h"
#include "interface/Iface.h"
#include "net/EventEmitter.h"
#include "net/SessionManager.h"
#include "net/UpperDistributor.h"
#include "net/TUNAdapter.h"
#include "util/version/Version.h"

struct NetCore* NetCore_new(uint8_t* privateKey,
                            struct Allocator* allocator,
                            EventBase_t* base,
                            struct Random* rand,
                            struct Log* log,
                            bool enableNoise)
{
    struct Allocator* alloc = Allocator_child(allocator);
    struct NetCore* nc = Allocator_calloc(alloc, sizeof(struct NetCore), 1);
    nc->alloc = alloc;
    nc->base = base;
    nc->rand = rand;
    nc->log = log;

    Ca_t* ca = nc->ca = Ca_new(alloc, privateKey, base, log, rand);
    uint8_t ourPubKey[32];
    Ca_getPubKey(ca, ourPubKey);
    struct EventEmitter* ee = nc->ee = EventEmitter_new(alloc, log, ourPubKey);

    struct Address* myAddress = nc->myAddress = Allocator_calloc(alloc, sizeof(struct Address), 1);
    Bits_memcpy(myAddress->key, ourPubKey, 32);
    Address_getPrefix(myAddress);
    myAddress->protocolVersion = Version_CURRENT_PROTOCOL;
    myAddress->path = 1;

    struct SwitchCore* switchCore = nc->switchCore = SwitchCore_new(log, alloc, base);

    struct SessionManager* sm = nc->sm = SessionManager_new(alloc, base, ca, rand, log, ee);
    Iface_plumb(switchCore->routerIf, &sm->switchIf);

    struct UpperDistributor* upper = nc->upper = UpperDistributor_new(alloc, log, ee, myAddress);
    Iface_plumb(&sm->insideIf, &upper->sessionManagerIf);

    // NEVER USE THIS SWITCHPINGER
    // It is legacy and only for iface controller.
    // Replies will NOT be routed to this sp unless they are PING and have the code "IFACE_CNTRLR" in them.
    struct SwitchPinger* sp = SwitchPinger_new(base, rand, log, myAddress, alloc);

    struct InterfaceController* ifc = nc->ifController =
        InterfaceController_new(
            ca, switchCore, log, base, sp, rand, alloc, ee, enableNoise);

    struct ControlHandler* controlHandler = nc->controlHandler =
        ControlHandler_new(alloc, log, ee, ourPubKey, ifc);

    Iface_plumb(&controlHandler->coreIf, &upper->controlHandlerIf);

    Iface_plumb(&controlHandler->ifcSwitchPingerIf, &sp->controlHandlerIf);

    struct TUNAdapter* tunAdapt = nc->tunAdapt = TUNAdapter_new(alloc, log, myAddress->ip6.bytes);
    Iface_plumb(&tunAdapt->upperDistributorIf, &upper->tunAdapterIf);

    return nc;
}
