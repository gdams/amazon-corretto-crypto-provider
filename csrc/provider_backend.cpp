// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0
#include "provider_backend.h"
#include "awslc_backend.h"
#include "symcrypt_backend.h"
#include "env.h"

namespace AmazonCorrettoCryptoProvider {

std::shared_ptr<ProviderBackend> ProviderBackend::createBackend(const std::string& name) {
    if (name == "scossl" || name == "SCOSSL") {
        return std::make_shared<SymCryptBackend>();
    }
    
    // Default to AWS-LC
    return std::make_shared<AwsLcBackend>();
}

} // namespace AmazonCorrettoCryptoProvider
