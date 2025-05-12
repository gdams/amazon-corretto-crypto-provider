#ifndef AWSLC_BACKEND_H
#define AWSLC_BACKEND_H 1

#include "provider_backend.h"
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace AmazonCorrettoCryptoProvider {

/**
 * AWS-LC implementation of the ProviderBackend interface.
 * This is the default provider that ACCP has been using.
 */
class AwsLcBackend : public ProviderBackend {
public:
    AwsLcBackend();
    ~AwsLcBackend() override;

    // Provider identification
    std::string getProviderName() const override { return "AWS-LC"; }
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

#endif // AWSLC_BACKEND_H
