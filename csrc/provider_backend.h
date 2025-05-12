#ifndef PROVIDER_BACKEND_H
#define PROVIDER_BACKEND_H 1

#include "compiler.h"
#include "config.h"
#include <stdint.h>
#include <memory>
#include <string>

namespace AmazonCorrettoCryptoProvider {

/**
 * Base interface for cryptographic provider backends.
 * This abstraction allows ACCP to use different backend implementations
 * such as AWS-LC or Microsoft SymCrypt.
 */
class ProviderBackend {
public:
    virtual ~ProviderBackend() = default;

    // Provider identification
    virtual std::string getProviderName() const = 0;
    virtual std::string getProviderVersion() const = 0;
    
    // Provider initialization
    virtual bool initialize() = 0;
    virtual bool isFipsMode() const = 0;
    
    // Library functionality checks
    virtual bool validateLibraryVersion(bool fuzzyMatch) = 0;
    
    // Cleanup
    virtual void cleanup() = 0;
    
    // Factory method to create appropriate backend
    static std::shared_ptr<ProviderBackend> createBackend(const std::string& name);
};

} // namespace AmazonCorrettoCryptoProvider

#endif // PROVIDER_BACKEND_H
