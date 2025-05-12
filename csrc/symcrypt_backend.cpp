// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0
#include "symcrypt_backend.h"
#include "env.h"

// Include SymCrypt headers
// TODO: Update these includes with the actual SymCrypt headers once integrated
#ifdef USE_SYMCRYPT
#include <symcrypt.h>
#else
// Mock declarations for when building without SymCrypt
// These allow compilation but will throw errors if actually used
#define SYMCRYPT_ERROR      int
#define SYMCRYPT_NO_ERROR   0
#endif

namespace AmazonCorrettoCryptoProvider {

SymCryptBackend::SymCryptBackend() {
    // Constructor is empty as initialization happens in the initialize() method
}

SymCryptBackend::~SymCryptBackend() {
    // Specific cleanup will be done in cleanup() method
}

std::string SymCryptBackend::getProviderVersion() const {
#ifdef USE_SYMCRYPT
    // Get SymCrypt version from the library
    return SymCryptGetVersionString();
#else
    return "SCOSSL (not available)";
#endif
}

bool SymCryptBackend::initialize() {
#ifdef USE_SYMCRYPT
    // Initialize SymCrypt library
    SYMCRYPT_ERROR status = SymCryptInit();
    if (status != SYMCRYPT_NO_ERROR) {
        // Log error or throw exception
        fprintf(stderr, "Failed to initialize SymCrypt library: error code %d\n", status);
        return false;
    }
    return true;
#else
    fprintf(stderr, "SymCrypt backend selected but library is not available\n");
    return false;
#endif
}

bool SymCryptBackend::isFipsMode() const {
#ifdef USE_SYMCRYPT
    // Check if SymCrypt is running in FIPS mode
    return SymCryptIsFipsMode();
#else
    return false;
#endif
}

bool SymCryptBackend::validateLibraryVersion(bool fuzzyMatch) {
#ifdef USE_SYMCRYPT
    // Get SymCrypt version
    const char* expected_version = SYMCRYPT_VERSION_STRING;
    const char* actual_version = SymCryptGetVersionString();
    
    if (fuzzyMatch) {
        // For fuzzy matching, we'll compare just the major and minor versions
        // Parse major.minor from version strings (assuming format like "X.Y.Z")
        int expected_major = 0, expected_minor = 0;
        int actual_major = 0, actual_minor = 0;
        
        sscanf(expected_version, "%d.%d", &expected_major, &expected_minor);
        sscanf(actual_version, "%d.%d", &actual_major, &actual_minor);
        
        return (expected_major == actual_major && expected_minor == actual_minor);
    } else {
        // Exact match required
        return (strcmp(expected_version, actual_version) == 0);
    }
#else
    return false;
#endif
}

void SymCryptBackend::cleanup() {
#ifdef USE_SYMCRYPT
    // Clean up SymCrypt resources
    SymCryptCleanup();
#endif
}

} // namespace AmazonCorrettoCryptoProvider
