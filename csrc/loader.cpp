// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0
#include "env.h"
#include "generated-headers.h"
#include "util.h"
#include "provider_backend.h"
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define OPENSSL_THREAD_DEFINES
#include <openssl/opensslconf.h>
#if defined(OPENSSL_THREADS)
// thread support enabled
#else
#error Openssl must be compiled with thread support
#endif

// Right now we only support PTHREAD
#include <pthread.h>

// https://www.openssl.org/docs/man1.1.1/man3/OPENSSL_VERSION_NUMBER.html
// 0xMNNFFPPS : major minor fix patch status
// 0x1010107f == v1.1.1g release
#define LIBCRYPTO_MAJOR_MINOR_VERSION_MASK 0xFFF00000

using namespace AmazonCorrettoCryptoProvider;

// Global instance of the currently active provider backend
std::shared_ptr<ProviderBackend> g_providerBackend;

namespace {
void initialize()
{
    // Get provider backend from system property or environment variable if set,
    // otherwise default to AWS-LC
    const char* backend_name = getenv("ACCP_CRYPTO_PROVIDER");
    
    // Check Java system property which takes precedence over environment variable
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_OK) {
        jclass system_class = env->FindClass("java/lang/System");
        if (system_class != NULL) {
            jmethodID get_property_method = env->GetStaticMethodID(system_class, "getProperty", 
                "(Ljava/lang/String;)Ljava/lang/String;");
            if (get_property_method != NULL) {
                jstring property_name = env->NewStringUTF("com.amazon.corretto.crypto.provider.cryptoBackend");
                jstring property_value = (jstring)env->CallStaticObjectMethod(system_class, 
                    get_property_method, property_name);
                
                if (property_value != NULL) {
                    const char* value = env->GetStringUTFChars(property_value, NULL);
                    if (value != NULL && strlen(value) > 0) {
                        backend_name = value;
                        // Note: We intentionally "leak" this string as it will be used 
                        // throughout the application's lifetime
                    }
                    env->ReleaseStringUTFChars(property_value, value);
                }
                env->DeleteLocalRef(property_name);
                if (property_value != NULL) {
                    env->DeleteLocalRef(property_value);
                }
            }
            env->DeleteLocalRef(system_class);
        }
    }
    
    if (!backend_name) {
        backend_name = "AWS-LC"; // Default
    }
    
    g_providerBackend = ProviderBackend::createBackend(backend_name);
    if (g_providerBackend) {
        g_providerBackend->initialize();
    }
}
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    initialize();
    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    if (g_providerBackend) {
        g_providerBackend->cleanup();
        g_providerBackend.reset();
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_corretto_crypto_provider_Loader_isFipsMode(JNIEnv*, jclass)
{
    if (g_providerBackend) {
        return g_providerBackend->isFipsMode() ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_amazon_corretto_crypto_provider_Loader_isExperimentalFipsMode(JNIEnv*, jclass)
{
#ifdef EXPERIMENTAL_FIPS_BUILD
    return JNI_TRUE;
#else
    return JNI_FALSE;
#endif
}

JNIEXPORT jboolean JNICALL Java_com_amazon_corretto_crypto_provider_Loader_isFipsSelfTestFailureSkipAbort(
    JNIEnv*, jclass)
{
#ifdef FIPS_SELF_TEST_SKIP_ABORT
    return JNI_TRUE;
#else
    return JNI_FALSE;
#endif
}

JNIEXPORT jstring JNICALL Java_com_amazon_corretto_crypto_provider_Loader_getNativeLibraryVersion(JNIEnv* pEnv, jclass)
{
    try {
        raii_env env(pEnv);

        return env->NewStringUTF(STRINGIFY(PROVIDER_VERSION_STRING));
    } catch (java_ex& ex) {
        ex.throw_to_java(pEnv);
        return NULL;
    }
}

JNIEXPORT jstring JNICALL Java_com_amazon_corretto_crypto_provider_Loader_getCryptoProviderBackend(JNIEnv* pEnv, jclass)
{
    try {
        raii_env env(pEnv);
        
        if (g_providerBackend) {
            return env->NewStringUTF(g_providerBackend->getProviderName().c_str());
        } else {
            return env->NewStringUTF("Unknown");
        }
    } catch (java_ex& ex) {
        ex.throw_to_java(pEnv);
        return NULL;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_corretto_crypto_provider_Loader_validateLibcryptoVersion(
    JNIEnv* pEnv, jclass, jboolean jFuzzyMatch)
{
    bool fuzzyMatch = (jFuzzyMatch == JNI_TRUE);

    try {
        unsigned long libcrypto_compiletime_version = OPENSSL_VERSION_NUMBER;
        unsigned long libcrypto_runtime_version = OpenSSL_version_num();

        if (fuzzyMatch) {
            libcrypto_compiletime_version &= LIBCRYPTO_MAJOR_MINOR_VERSION_MASK;
            libcrypto_runtime_version &= LIBCRYPTO_MAJOR_MINOR_VERSION_MASK;
        }

        if (libcrypto_compiletime_version != libcrypto_runtime_version) {
            char accp_loader_exception_msg[256] = { 0 };
            snprintf(accp_loader_exception_msg, sizeof(accp_loader_exception_msg),
                "Runtime libcrypto version does not match compile-time version. Expected: 0x%08lX , Actual: 0x%08lX",
                libcrypto_compiletime_version, libcrypto_runtime_version);
            throw java_ex(EX_RUNTIME_CRYPTO, accp_loader_exception_msg);
        }
        return JNI_TRUE;
    } catch (java_ex& ex) {
        ex.throw_to_java(pEnv);
    }

    return JNI_FALSE;
}
