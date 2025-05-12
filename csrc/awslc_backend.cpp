// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0
#include "awslc_backend.h"
#include "env.h"
#include "util.h"

namespace AmazonCorrettoCryptoProvider {

AwsLcBackend::AwsLcBackend() {
    // Constructor is empty as initialization happens in the initialize() method
}

AwsLcBackend::~AwsLcBackend() {
    // Specific cleanup will be done in cleanup() method
}

std::string AwsLcBackend::getProviderVersion() const {
    // Get OpenSSL/AWS-LC version
    return OpenSSL_version(OPENSSL_VERSION);
}

bool AwsLcBackend::initialize() {
    CRYPTO_library_init();
    ERR_load_crypto_strings();
    OpenSSL_add_all_digests();
    return true;
}

bool AwsLcBackend::isFipsMode() const {
    return FIPS_mode() == 1;
}

bool AwsLcBackend::validateLibraryVersion(bool fuzzyMatch) {
    // Extracted from loader.cpp
    unsigned long libcrypto_compiletime_version = OPENSSL_VERSION_NUMBER;
    unsigned long libcrypto_runtime_version = OpenSSL_version_num();

    if (fuzzyMatch) {
        // 0xMNNFFPPS : major minor fix patch status
        // 0x1010107f == v1.1.1g release
        // LIBCRYPTO_MAJOR_MINOR_VERSION_MASK is defined as 0xFFF00000
        const unsigned long LIBCRYPTO_MAJOR_MINOR_VERSION_MASK = 0xFFF00000;
        libcrypto_compiletime_version &= LIBCRYPTO_MAJOR_MINOR_VERSION_MASK;
        libcrypto_runtime_version &= LIBCRYPTO_MAJOR_MINOR_VERSION_MASK;
    }

    return (libcrypto_compiletime_version == libcrypto_runtime_version);
}

void AwsLcBackend::cleanup() {
    // AWS-LC cleanup 
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
}

} // namespace AmazonCorrettoCryptoProvider
