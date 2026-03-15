#!/bin/bash

# Test script for POST method
# Tests HTTP POST requests to the server

SERVER_URL="http://localhost:8080"

echo "========================================"
echo "  TEST SUITE: POST Method"
echo "========================================"
echo ""

# Test: POST to /scripts/test.py
echo "Test: POST /scripts/test.py"
echo "  Endpoint: $SERVER_URL/scripts/test.py"
echo "  Data: mensaje=test"

# Send POST request and capture response
RESPONSE_CODE=$(curl -s -o /tmp/post_response.txt -w "%{http_code}" \
    -X POST \
    -d "mensaje=test" \
    "$SERVER_URL/scripts/test.py")

echo "  HTTP Code: $RESPONSE_CODE"

# Show server response
echo "  Server Response:"
echo "  ----------------------------------------"
cat /tmp/post_response.txt
echo "  ----------------------------------------"

# Verify result
if [ "$RESPONSE_CODE" = "200" ]; then
    echo "  Result: PASS (200)"
else
    echo "  Result: FAIL (Expected 200, got $RESPONSE_CODE)"
fi

# Cleanup
rm -f /tmp/post_response.txt

echo ""
echo "========================================"
echo "  Tests completed"
echo "========================================"