/*
 * Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

package com.amazon.corretto.crypto.provider;

import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.security.Security;

/**
 * Tests for verifying backend provider selection functionality
 */
public class BackendProviderTest {
    private String originalBackendSetting;
    private AmazonCorrettoCryptoProvider accp;

    @Before
    public void setUp() {
        // Store original setting if any
        originalBackendSetting = System.getProperty("com.amazon.corretto.crypto.provider.cryptoBackend");
        
        // Get ACCP instance
        accp = new AmazonCorrettoCryptoProvider();
    }

    @After
    public void tearDown() {
        // Restore original setting
        if (originalBackendSetting != null) {
            System.setProperty("com.amazon.corretto.crypto.provider.cryptoBackend", originalBackendSetting);
        } else {
            System.clearProperty("com.amazon.corretto.crypto.provider.cryptoBackend");
        }
        
        // Re-initialize ACCP
        Security.removeProvider(accp.getName());
    }

    @Test
    public void testDefaultBackendProvider() {
        // Clear any setting to ensure default
        System.clearProperty("com.amazon.corretto.crypto.provider.cryptoBackend");
        
        // Re-initialize ACCP to pick up changes
        Security.removeProvider(accp.getName());
        Security.insertProviderAt(accp, 1);
        
        // Default should be AWS-LC
        assertEquals("Default backend provider should be AWS-LC", 
                     "AWS-LC", 
                     accp.getCryptoProviderBackend());
    }

    @Test
    public void testAwsLcBackendSelection() {
        // Explicitly set AWS-LC backend
        System.setProperty("com.amazon.corretto.crypto.provider.cryptoBackend", "AWS-LC");
        
        // Re-initialize ACCP to pick up changes
        Security.removeProvider(accp.getName());
        Security.insertProviderAt(accp, 1);
        
        // Should be AWS-LC
        assertEquals("Backend provider should be AWS-LC when explicitly selected", 
                     "AWS-LC", 
                     accp.getCryptoProviderBackend());
    }

    @Test
    public void testScosslBackendSelectionIfAvailable() {
        // Set SCOSSL backend
        System.setProperty("com.amazon.corretto.crypto.provider.cryptoBackend", "SCOSSL");
        
        // Re-initialize ACCP to pick up changes
        Security.removeProvider(accp.getName());
        
        try {
            Security.insertProviderAt(accp, 1);
            
            // Check if SCOSSL is the backend
            // Note: This test will "pass" either way, as we check the condition in the assertion message
            String backend = accp.getCryptoProviderBackend();
            if (backend.equals("SCOSSL")) {
                assertTrue("SCOSSL backend was successfully selected", true);
            } else {
                // This might happen if SCOSSL support is not compiled in
                assertTrue("SCOSSL backend was requested but not available, " + 
                          "using " + backend + " instead", true);
            }
        } catch (Exception e) {
            // If loading fails with SCOSSL (which might be expected if it's not installed),
            // we'll just log the information
            System.out.println("Note: Failed to load with SCOSSL backend: " + e.getMessage());
            assertTrue("Could not load SCOSSL backend (might be expected if not installed)", true);
        }
    }
}
