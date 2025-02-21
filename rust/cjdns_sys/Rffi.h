#ifndef rffi_H
#define rffi_H

// This file is generated from src/rffi.rs using cbindgen

#include "RffiPrefix.h"

typedef struct RTypes_CryptoAuth2_t RTypes_CryptoAuth2_t;

typedef struct Rffi_FdReadableTx Rffi_FdReadableTx;

typedef struct Rffi_Seeder Rffi_Seeder;

typedef struct Rffi_SocketIface_t Rffi_SocketIface_t;

typedef struct Rffi_SocketServer Rffi_SocketServer;

/**
 * The handle returned to C, used to talk to the timer task.
 */
typedef struct Rffi_TimerTx Rffi_TimerTx;

typedef struct Rffi_UDPIface_pvt Rffi_UDPIface_pvt;

typedef struct {
  Rffi_UDPIface_pvt *pvt;
  Iface_t *iface;
  Sockaddr_t *local_addr;
} Rffi_UDPIface;

typedef struct {
  uint8_t octets[16];
  uint8_t netmask[16];
  bool is_ipv6;
} Rffi_Address;

typedef struct {
  const char *name;
  uint8_t phys_addr[6];
  bool is_internal;
  Rffi_Address address;
} Rffi_NetworkInterface;

typedef void (*OnFreeFun)(void *ctx);

extern const uintptr_t Rffi_CURRENT_PROTOCOL;

int Rffi_CryptoAuth2_addUser_ipv6(String_t *password,
                                  String_t *login,
                                  uint8_t *ipv6,
                                  const RTypes_CryptoAuth2_t *ca);

int Rffi_CryptoAuth2_removeUsers(const RTypes_CryptoAuth2_t *context, String_t *user);

RTypes_StrList_t *Rffi_CryptoAuth2_getUsers(const RTypes_CryptoAuth2_t *ca, Allocator_t *alloc);

RTypes_CryptoAuth2_t *Rffi_CryptoAuth2_new(Allocator_t *allocator,
                                           const uint8_t *privateKey,
                                           Random_t *random);

void Rffi_CryptoAuth2_tryHandshake(const RTypes_CryptoAuth2_t *ca,
                                   Message_t *c_msg,
                                   Allocator_t *alloc,
                                   bool requireAuth,
                                   RTypes_CryptoAuth2_TryHandshake_Ret_t *ret);

RTypes_CryptoAuth2_Session_t *Rffi_CryptoAuth2_newSession(const RTypes_CryptoAuth2_t *ca,
                                                          Allocator_t *alloc,
                                                          const uint8_t *herPublicKey,
                                                          bool requireAuth,
                                                          const char *name,
                                                          bool useNoise);

Message_t *Rffi_CryptoAuth2_noiseTick(RTypes_CryptoAuth2_Session_t *sess, Allocator_t *alloc);

void Rffi_CryptoAuth2_setAuth(const String_t *password,
                              const String_t *login,
                              RTypes_CryptoAuth2_Session_t *caSession);

void Rffi_CryptoAuth2_resetIfTimeout(RTypes_CryptoAuth2_Session_t *session);

void Rffi_CryptoAuth2_reset(RTypes_CryptoAuth2_Session_t *caSession);

RTypes_CryptoAuth_State_t Rffi_CryptoAuth2_getState(RTypes_CryptoAuth2_Session_t *session);

void Rffi_CryptoAuth2_getHerPubKey(const RTypes_CryptoAuth2_Session_t *session, uint8_t *pkOut);

void Rffi_CryptoAuth2_getHerIp6(const RTypes_CryptoAuth2_Session_t *session, uint8_t *ipOut);

String_t *Rffi_CryptoAuth2_getName(const RTypes_CryptoAuth2_Session_t *session, Allocator_t *alloc);

void Rffi_CryptoAuth2_getPubKey(const RTypes_CryptoAuth2_t *ca, uint8_t *pkOut);

void Rffi_CryptoAuth2_stats(const RTypes_CryptoAuth2_Session_t *session,
                            RTypes_CryptoStats_t *statsOut);

uint32_t Rffi_CryptoAuth2_cjdnsVer(const RTypes_CryptoAuth2_Session_t *session);

int Rffi_CryptoAuth2_getSecret(const RTypes_CryptoAuth2_t *ca,
                               const String_t *name,
                               uint8_t *secretOut);

int Rffi_crypto_hash_sha512(unsigned char *out,
                            const unsigned char *input,
                            unsigned long long inlen);

int Rffi_crypto_hash_sha256(unsigned char *out,
                            const unsigned char *input,
                            unsigned long long inlen);

void Rffi_stopEventLoop(RTypes_EventLoop_t *event_loop);

void Rffi_startEventLoop(RTypes_EventLoop_t *event_loop);

/**
 * Create a new EventLoop data repository.
 */
RTypes_EventLoop_t *Rffi_mkEventLoop(Allocator_t *alloc);

/**
 * Get the full filesystem path of the current running executable.
 */
int32_t Rffi_exepath(const char **out, Allocator_t *alloc);

/**
 * Spawn a new child process, and monitors its result.
 */
int32_t Rffi_spawn(const char *file,
                   const char *const *args,
                   int num_args,
                   Allocator_t *_alloc,
                   void (*cb)(int64_t, int));

/**
 * Spawn a timer task for a timeout or interval, that calls some callback whenever it triggers.
 */
void Rffi_setTimeout(Rffi_TimerTx **out_timer_tx,
                     void (*cb)(void*),
                     void *cb_context,
                     unsigned long timeout_millis,
                     bool repeat,
                     RTypes_EventLoop_t *event_loop,
                     Allocator_t *alloc);

/**
 * Reset a timer task to change its timing.
 */
int Rffi_resetTimeout(const Rffi_TimerTx *timer_tx, unsigned long timeout_millis);

/**
 * Cancel a timer task.
 */
int Rffi_clearTimeout(const Rffi_TimerTx *timer_tx);

/**
 * Return 1 if a timer task is still running, 0 otherwise.
 */
int Rffi_isTimeoutActive(const Rffi_TimerTx *timer_tx);

/**
 * Cancel all timer tasks.
 */
void Rffi_clearAllTimeouts(RTypes_EventLoop_t *event_loop);

RTypes_Error_t *Rffi_pollFdReadable(Rffi_FdReadableTx **out,
                                    void (*cb)(void*),
                                    void *cb_context,
                                    int fd,
                                    Allocator_t *alloc);

int32_t Rffi_udpIfaceGetFd(Rffi_UDPIface_pvt *iface);

int32_t Rffi_udpIfaceSetBroadcast(Rffi_UDPIface_pvt *iface, bool broadcast);

RTypes_Error_t *Rffi_udpIface_worker_states(Object_t **outP,
                                            Rffi_UDPIface_pvt *iface,
                                            Allocator_t *alloc);

int32_t Rffi_udpIfaceSetDscp(Rffi_UDPIface_pvt *iface, uint8_t dscp);

RTypes_Error_t *Rffi_udpIfaceNew(Rffi_UDPIface **outp,
                                 const Sockaddr_t *bind_addr,
                                 Allocator_t *c_alloc);

RTypes_Error_t *Rffi_fileExists(bool *existsOut, const char *path, Allocator_t *errorAlloc);

RTypes_Error_t *Rffi_socketWorkerStates(Object_t **outP,
                                        const Rffi_SocketIface_t *si,
                                        Allocator_t *alloc);

RTypes_Error_t *Rffi_socketForFd(Iface_t **ifOut,
                                 Rffi_SocketIface_t **so_out,
                                 int fd,
                                 RTypes_SocketType st,
                                 Allocator_t *alloc);

RTypes_Error_t *Rffi_unixSocketConnect(Iface_t **ifOut, const char *path, Allocator_t *alloc);

void Rffi_unixSocketServerOnConnect(Rffi_SocketServer *rss,
                                    void (*f)(void*, const Sockaddr_t*),
                                    void *ctx);

RTypes_Error_t *Rffi_unixSocketServer(Rffi_SocketServer **rssOut,
                                      Iface_t **ifaceOut,
                                      const char *path,
                                      Allocator_t *alloc);

/**
 * Convert IPv4 and IPv6 addresses from binary to text form.
 */
int32_t Rffi_inet_ntop(bool is_ip6, const void *addr, uint8_t *dst, uint32_t dst_sz);

/**
 * Convert IPv4 and IPv6 addresses from text to binary form.
 */
int32_t Rffi_inet_pton(bool is_ip6, const char *src, uint8_t *addr);

/**
 * Get a list of available network interfaces for the current machine.
 */
int32_t Rffi_interface_addresses(const Rffi_NetworkInterface **out, Allocator_t *alloc);

/**
 * Non-monotonic nanosecond time, which has no relationship to any wall clock.
 */
uint64_t Rffi_hrtime(void);

/**
 * Monotonic millisecond time.
 */
uint64_t Rffi_now_ms(void);

void Rffi_sleep_ms_sync(uint64_t ms);

RTypes_IfWrapper_t Rffi_testwrapper_create(Allocator_t *a);

RTypes_IfWrapper_t Rffi_android_create(Allocator_t *a);

void Rffi_panic(const char *msg);

void Rffi_setLogger(Log_t *l);

RTypes_Error_t *Rffi_error(const char *msg, Allocator_t *alloc);

RTypes_Error_t *Rffi_error_fl(const char *msg, const char *file, int line, Allocator_t *alloc);

char *Rffi_printError(RTypes_Error_t *e, Allocator_t *alloc);

void Rffi_glock(void);

void Rffi_gunlock(void);

int Rffi_parseBase10(const uint8_t *buf, uint32_t max_len, int64_t *num_out, uint32_t *bytes);

/**
 * Create a root level allocator.
 */
Allocator_t *Rffi_allocator_newRoot(const char *file, uintptr_t line);

void Rffi_allocator_free(Allocator_t *a, const char *file, uintptr_t line);

int Rffi_allocator_isFreeing(Allocator_t *a);

Allocator_t *Rffi_allocator_child(Allocator_t *a, const char *file, uintptr_t line);

uint8_t *Rffi_allocator_malloc(Allocator_t *a, uintptr_t size);

uint8_t *Rffi_allocator_calloc(Allocator_t *a, uintptr_t size);

uint8_t *Rffi_allocator_realloc(Allocator_t *a, uint8_t *ptr, uintptr_t new_size);

void Rffi_allocator_onFree(Allocator_t *a,
                           OnFreeFun fun,
                           void *ctx,
                           const char *file,
                           uintptr_t line);

void Rffi_allocator_adopt(Allocator_t *a, Allocator_t *to_adopt);

RTypes_Error_t *Rffi_Seeder_addDnsSeed(Rffi_Seeder *seeder,
                                       const String_t *dns_seed,
                                       bool trust_snode,
                                       Allocator_t *alloc);

RTypes_Error_t *Rffi_Seeder_rmDnsSeed(bool *found,
                                      Rffi_Seeder *seeder,
                                      const String_t *dns_seed,
                                      Allocator_t *alloc);

RTypes_Error_t *Rffi_Seeder_listDnsSeeds(RTypes_Seeder_DnsSeeds_t **seeds_out,
                                         Rffi_Seeder *seeder,
                                         Allocator_t *alloc);

void Rffi_Seeder_new(Rffi_Seeder **seeder_out,
                     Iface_t **iface_out,
                     const uint8_t *my_pubkey,
                     Allocator_t *alloc);

bool Rffi_Seeder_got_lladdr(Rffi_Seeder *seeder, const Control_LlAddr_t *lladdr);

RTypes_Error_t *Rffi_Seeder_mk_creds(Rffi_Seeder *seeder, String_t **creds, Allocator_t *alloc);

RTypes_Error_t *Rffi_Seeder_public_peer(Rffi_Seeder *seeder,
                                        String_t **user_out,
                                        String_t **pass_out,
                                        uint16_t user_num,
                                        uint64_t passwd,
                                        const String_t *code,
                                        const String_t *addr4,
                                        const String_t *addr6,
                                        Allocator_t *alloc);

RTypes_Error_t *Rffi_Seeder_public_status(Rffi_Seeder *seeder,
                                          String_t **ipv4_out,
                                          String_t **ipv6_out,
                                          String_t **peer_id_out,
                                          Allocator_t *alloc);

RTypes_Error_t *Rffi_Seeder_got_peers(Rffi_Seeder *seeder,
                                      const String_t *peers,
                                      Allocator_t *alloc);

bool Rffi_Seeder_has_lladdr(Rffi_Seeder *seeder);

RTypes_Error_t *Rffi_Benc_decodeJson(Dict_t **out,
                                     Message_t *msg,
                                     bool lax_mode,
                                     Allocator_t *alloc);

RTypes_Error_t *Rffi_Benc_encodeJson(Dict_t *input, Message_t *msg, Allocator_t *alloc);

#endif /* rffi_H */
