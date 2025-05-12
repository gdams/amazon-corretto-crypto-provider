#ifndef SYMCRYPT_BACKEND_H
#define SYMCRYPT_BACKEND_H 1

#include "provider_backend.h"

namespace AmazonCorrettoCryptoProvider {

/**
 * Microsoft SymCrypt implementation of the ProviderBackend interface.
 */
class SymCryptBackend : public ProviderBackend {
public:
    SymCryptBackend();
    ~SymCryptBackend() override;

    // Provider identification
    std::string getProviderName() const override { return "SCOSSL"; }
    std::string getProviderVersion() const override;
    
    // Provider initialization
    bool initialize() override;
    bool isFipsMode() const override;
    
    // Library functionality checks
    bool validateLibraryVersion(bool fuzzyMatch) override;
    
    // Cleanup
    void cleanup() override;
};

} // namespace AmazonCorrettoCryptoProvider

#endif // SYMCRYPT_BACKEND_H
