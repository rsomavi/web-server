#!/bin/bash

# Test script for GET method
# Tests HTTP GET requests to the server

SERVER_URL="http://localhost:8080"

echo "========================================"
echo "  TEST SUITE: GET Method"
echo "========================================"
echo ""

# Test 1: GET /index.html
echo "Test 1: GET /index.html"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "$SERVER_URL/index.html")
echo "  Endpoint: $SERVER_URL/index.html"
echo "  HTTP Code: $RESPONSE"
if [ "$RESPONSE" = "200" ]; then
    echo "  Result: PASS (200)"
else
    echo "  Result: FAIL (Expected 200, got $RESPONSE)"
fi
echo ""

# Test 2: GET / (root)
echo "Test 2: GET /"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "$SERVER_URL/")
echo "  Endpoint: $SERVER_URL/"
echo "  HTTP Code: $RESPONSE"
if [ "$RESPONSE" = "200" ]; then
    echo "  Result: PASS (200)"
else
    echo "  Result: FAIL (Expected 200, got $RESPONSE)"
fi
echo ""

# Test 3: GET /no_existe.html (should return 404)
echo "Test 3: GET /no_existe.html"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "$SERVER_URL/no_existe.html")
echo "  Endpoint: $SERVER_URL/no_existe.html"
echo "  HTTP Code: $RESPONSE"
if [ "$RESPONSE" = "404" ]; then
    echo "  Result: PASS (404)"
else
    echo "  Result: FAIL (Expected 404, got $RESPONSE)"
fi
echo ""

echo "========================================"
echo "  Tests completed"
echo "========================================"
