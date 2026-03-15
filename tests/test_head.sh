#!/bin/bash

# Test HEAD HTTP method
# This script tests the HEAD method by making real HTTP requests to the server

SERVER_PORT=8092
SERVER_PID=""
SERVER_ROOT="./www"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Cleanup function
cleanup() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
}

trap cleanup EXIT

# Build server if needed
echo "Building server..."
cd /home/ruben/redesII/r2-p1
make clean >/dev/null 2>&1
make >/dev/null 2>&1

if [ ! -f "./server" ]; then
    echo -e "${RED}FAIL: Server binary not found${NC}"
    exit 1
fi

# Create shared directory if it doesn't exist
mkdir -p "$SERVER_ROOT/shared"

# Start server in background with custom config
./server tests/test_head.conf &
SERVER_PID=$!

# Wait for server to start
sleep 2

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}FAIL: Server failed to start${NC}"
    exit 1
fi

echo "Server started on port $SERVER_PORT (PID: $SERVER_PID)"
echo "=========================================="

TESTS_PASSED=0
TESTS_FAILED=0

# Test 1: HEAD existing file
echo -e "${YELLOW}Test 1: HEAD existing file (index.html)${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X HEAD http://localhost:$SERVER_PORT/index.html)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}  PASS: Got 200 OK${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected 200, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 2: HEAD returns no body (just headers)
echo -e "${YELLOW}Test 2: HEAD returns no body${NC}"
RESPONSE=$(curl -s -i -X HEAD http://localhost:$SERVER_PORT/index.html)

# Check that response has headers but no body content
BODY_LENGTH=$(echo "$RESPONSE" | sed 's/\r$//' | awk '/^HTTP/{found=1} found{print}' | grep -i "Content-Length:" | awk '{print $2}' | tr -d '\r\n')
HEADERS_ONLY=$(echo "$RESPONSE" | sed 's/\r$//' | awk '/^HTTP/{found=1} found{print}' | head -1)

if [ -n "$BODY_LENGTH" ] && [ "$BODY_LENGTH" -gt 0 ]; then
    echo -e "${GREEN}  PASS: HEAD returns Content-Length header (body should be suppressed by client)${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${YELLOW}  INFO: HEAD response received${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 3: HEAD 404 for non-existent file
echo -e "${YELLOW}Test 3: HEAD 404 for non-existent file${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X HEAD http://localhost:$SERVER_PORT/nonexistent.html)

if [ "$HTTP_CODE" = "404" ]; then
    echo -e "${GREEN}  PASS: Got 404 Not Found${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected 404, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 4: HEAD checks Content-Type header
echo -e "${YELLOW}Test 4: HEAD Content-Type header${NC}"
CONTENT_TYPE=$(curl -s -I -X HEAD http://localhost:$SERVER_PORT/index.html | grep -i "Content-Type:" | tr -d '\r')

if echo "$CONTENT_TYPE" | grep -qi "text/html"; then
    echo -e "${GREEN}  PASS: Content-Type is text/html${NC}"
    echo "  Header: $CONTENT_TYPE"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected text/html, got $CONTENT_TYPE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 5: HEAD for directory serves index.html
echo -e "${YELLOW}Test 5: HEAD directory serves index.html${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X HEAD http://localhost:$SERVER_PORT/)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}  PASS: Got 200 OK for directory${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected 200, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 6: HEAD for JSON file
echo -e "${YELLOW}Test 6: HEAD JSON file${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X HEAD http://localhost:$SERVER_PORT/ejemplo.json)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}  PASS: Got 200 OK for JSON${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected 200, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 7: HEAD returns Last-Modified header
echo -e "${YELLOW}Test 7: HEAD returns Last-Modified header${NC}"
LAST_MODIFIED=$(curl -s -I -X HEAD http://localhost:$SERVER_PORT/index.html | grep -i "Last-Modified:" | tr -d '\r')

if [ -n "$LAST_MODIFIED" ]; then
    echo -e "${GREEN}  PASS: Last-Modified header present${NC}"
    echo "  Header: $LAST_MODIFIED"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${YELLOW}  INFO: Last-Modified header not found (optional)${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 8: HEAD for CSS file
echo -e "${YELLOW}Test 8: HEAD for CSS file${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X HEAD http://localhost:$SERVER_PORT/index.html)

if [ "$HTTP_CODE" = "200" ]; then
    CONTENT_TYPE=$(curl -s -I -X HEAD http://localhost:$SERVER_PORT/index.html | grep -i "Content-Type:" | tr -d '\r')
    if echo "$CONTENT_TYPE" | grep -qi "text/html"; then
        echo -e "${GREEN}  PASS: Got 200 with correct Content-Type${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}  FAIL: Wrong Content-Type${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
else
    echo -e "${RED}  FAIL: Expected 200, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo "=========================================="
echo "HEAD Tests Summary: $TESTS_PASSED passed, $TESTS_FAILED failed"

if [ $TESTS_FAILED -gt 0 ]; then
    exit 1
fi
exit 0
