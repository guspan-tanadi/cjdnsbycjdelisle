#ifndef rffi_H
#define rffi_H

// This file is generated from src/rffi.rs using cbindgen

#include "RffiPrefix.h"

typedef struct RTypes_CryptoAuth2_t RTypes_CryptoAuth2_t;

RTypes_IfWrapper_t Rffi_testwrapper_create(Allocator_t *a);

RTypes_IfWrapper_t Rffi_android_create(Allocator_t *a);

int Rffi_CryptoAuth2_addUser_ipv6(String_t *password,
                                  String_t *login,
                                  uint8_t *ipv6,
                                  RTypes_CryptoAuth2_t *ca);

int Rffi_CryptoAuth2_removeUsers(RTypes_CryptoAuth2_t *context, String_t *user);

RTypes_StrList_t *Rffi_CryptoAuth2_getUsers(const RTypes_CryptoAuth2_t *ca, Allocator_t *alloc);

RTypes_CryptoAuth2_t *Rffi_CryptoAuth2_new(Allocator_t *allocator,
                                           const uint8_t *privateKey,
                                           Random_t *random);

RTypes_CryptoAuth2_Session_t *Rffi_CryptoAuth2_newSession(RTypes_CryptoAuth2_t *ca,
                                                          Allocator_t *alloc,
                                                          const uint8_t *herPublicKey,
                                                          bool requireAuth,
                                                          char *name,
                                                          bool useNoise);

void Rffi_CryptoAuth2_setAuth(const String_t *password,
                              const String_t *login,
                              RTypes_CryptoAuth2_Session_t *caSession);

void Rffi_CryptoAuth2_resetIfTimeout(RTypes_CryptoAuth2_Session_t *session);

void Rffi_CryptoAuth2_reset(RTypes_CryptoAuth2_Session_t *caSession);

RTypes_CryptoAuth_State_t Rffi_CryptoAuth2_getState(RTypes_CryptoAuth2_Session_t *session);

void Rffi_CryptoAuth2_getHerPubKey(const RTypes_CryptoAuth2_Session_t *session, uint8_t *pkOut);

void Rffi_CryptoAuth2_getHerIp6(const RTypes_CryptoAuth2_Session_t *session, uint8_t *ipOut);

String_t *Rffi_CryptoAuth2_getName(const RTypes_CryptoAuth2_Session_t *session, Allocator_t *alloc);

void Rffi_CryptoAuth2_getPubKey(RTypes_CryptoAuth2_t *ca, uint8_t *pkOut);

void Rffi_CryptoAuth2_stats(const RTypes_CryptoAuth2_Session_t *session,
                            RTypes_CryptoStats_t *statsOut);

void Rffi_panic(const char *msg);

#endif /* rffi_H */
