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
#include "exception/Er.h"
#include "rust/cjdns_sys/Rffi.h"
#include "memory/Allocator.h"
#include "util/events/libuv/EventBase_pvt.h"
#include "util/events/Event.h"

Er_DEFUN(void Event_socketRead(void (* const callback)(void* callbackContext),
                      void* const callbackContext,
                      int s,
                      struct EventBase* eventBase,
                      struct Allocator* userAlloc))
{
    struct EventBase_pvt* base = EventBase_privatize(eventBase);
    Rffi_FdReadableTx* out = NULL;
    const char* errout = NULL;
    Rffi_pollFdReadable(
        &out,
        &errout,
        callback,
        callbackContext,
        s,
        base->rffi_loop,
        userAlloc);
    if (errout != NULL) {
        Er_raise(userAlloc, "Event_socketRead error: %s", errout);
    }
    Er_ret();
}
