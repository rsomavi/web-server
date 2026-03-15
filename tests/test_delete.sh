#!/bin/bash

# Test DELETE HTTP method
# This script tests the DELETE method by making real HTTP requests to the server

SERVER_PORT=8091
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
    # Remove test files
    rm -f "$SERVER_ROOT/shared/test_delete.txt"
    rm -f "$SERVER_ROOT/shared/test_delete2.txt"
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

# Create shared directory and test files
mkdir -p "$SERVER_ROOT/shared"
echo "Test content for delete" > "$SERVER_ROOT/shared/test_delete.txt"
echo "Another test file" > "$SERVER_ROOT/shared/test_delete2.txt"

# Start server in background with custom config
./server tests/test_delete.conf &
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

# Test 1: DELETE existing file in shared folder
echo -e "${YELLOW}Test 1: DELETE existing file in shared folder${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$SERVER_PORT/shared/test_delete.txt)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}  PASS: Got 200 OK${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    
    # Verify file was deleted
    if [ ! -f "$SERVER_ROOT/shared/test_delete.txt" ]; then
        echo -e "${GREEN}  PASS: File was deleted${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}  FAIL: File still exists${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
else
    echo -e "${RED}  FAIL: Expected 200, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 2: DELETE non-existent file should return 404
echo -e "${YELLOW}Test 2: DELETE non-existent file (should return 404)${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$SERVER_PORT/shared/nonexistent.txt)

if [ "$HTTP_CODE" = "404" ]; then
    echo -e "${GREEN}  PASS: Got 404 Not Found${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected 404, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 3: DELETE outside shared folder should fail
echo -e "${YELLOW}Test 3: DELETE outside shared folder should fail${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$SERVER_PORT/index.html)

if [ "$HTTP_CODE" = "400" ]; then
    echo -e "${GREEN}  PASS: Got 400 Bad Request${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected 400, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 4: DELETE file with no extension (should fail)
echo -e "${YELLOW}Test 4: DELETE file with no extension (should fail)${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$SERVER_PORT/shared/noextension)

if [ "$HTTP_CODE" = "400" ]; then
    echo -e "${GREEN}  PASS: Got 400 Bad Request for no extension${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}  FAIL: Expected 400, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 5: DELETE another existing file
echo -e "${YELLOW}Test 5: DELETE another existing file${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$SERVER_PORT/shared/test_delete2.txt)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}  PASS: Got 200 OK${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    
    if [ ! -f "$SERVER_ROOT/shared/test_delete2.txt" ]; then
        echo -e "${GREEN}  PASS: File was deleted${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}  FAIL: File still exists${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
else
    echo -e "${RED}  FAIL: Expected 200, got $HTTP_CODE${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo "=========================================="
echo "DELETE Tests Summary: $TESTS_PASSED passed, $TESTS_FAILED failed"

if [ $TESTS_FAILED -gt 0 ]; then
    exit 1
fi
exit 0
