#!/bin/bash

# Master test runner for HTTP methods
# Runs all HTTP method tests: GET, PUT, DELETE, HEAD

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  HTTP Methods Test Suite${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

TOTAL_PASSED=0
TOTAL_FAILED=0

# Test GET
echo -e "${YELLOW}[1/4] Running GET tests...${NC}"
if bash test_get.sh; then
    echo -e "${GREEN}GET tests PASSED${NC}"
    TOTAL_PASSED=$((TOTAL_PASSED + 1))
else
    echo -e "${RED}GET tests FAILED${NC}"
    TOTAL_FAILED=$((TOTAL_FAILED + 1))
fi
echo ""

# Test PUT
echo -e "${YELLOW}[2/4] Running PUT tests...${NC}"
if bash test_put.sh; then
    echo -e "${GREEN}PUT tests PASSED${NC}"
    TOTAL_PASSED=$((TOTAL_PASSED + 1))
else
    echo -e "${RED}PUT tests FAILED${NC}"
    TOTAL_FAILED=$((TOTAL_FAILED + 1))
fi
echo ""

# Test DELETE
echo -e "${YELLOW}[3/4] Running DELETE tests...${NC}"
if bash test_delete.sh; then
    echo -e "${GREEN}DELETE tests PASSED${NC}"
    TOTAL_PASSED=$((TOTAL_PASSED + 1))
else
    echo -e "${RED}DELETE tests FAILED${NC}"
    TOTAL_FAILED=$((TOTAL_FAILED + 1))
fi
echo ""

# Test HEAD
echo -e "${YELLOW}[4/4] Running HEAD tests...${NC}"
if bash test_head.sh; then
    echo -e "${GREEN}HEAD tests PASSED${NC}"
    TOTAL_PASSED=$((TOTAL_PASSED + 1))
else
    echo -e "${RED}HEAD tests FAILED${NC}"
    TOTAL_FAILED=$((TOTAL_FAILED + 1))
fi
echo ""

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Final Summary${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "Total: $((TOTAL_PASSED + TOTAL_FAILED)) test suites"
echo -e "${GREEN}Passed: $TOTAL_PASSED${NC}"
echo -e "${RED}Failed: $TOTAL_FAILED${NC}"
echo ""

if [ $TOTAL_FAILED -gt 0 ]; then
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi
