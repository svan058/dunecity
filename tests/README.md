# Dune Legacy Unit Tests

This directory contains unit tests for Dune Legacy using the Catch2 testing framework.

## Test Suites

### PathBudgetTestCase
Tests for the PathBudget synchronization protocol used in multiplayer.
See: `.analysis/features/pathbudget-sync/design.md` for the protocol specification.

Tests cover:
- **DESYNC detection** - Verifies correct detection of budget mismatches
- **Budget-for-decision-time** - Verifies clients report correct budget values
- **Timing** - Verifies stats/decision cycle calculations

### NetworkManagerTestCase  
Tests for NetworkManager functionality:
- Packet type constants
- Address utilities (Address2String)
- ENet packet stream serialization
- Keep-alive timing constants
- NAT traversal constants

## Building Tests

Tests are optional and require Catch2. To enable:

```bash
# Install Catch2 via Homebrew (macOS)
brew install catch2

# Or via system package manager (Linux)
sudo apt-get install catch2  # Debian/Ubuntu

# Configure with tests enabled
cmake -B build -DDUNELEGACY_BUILD_TESTS=ON

# Build
cmake --build build --target dunelegacy_tests

# Run tests
./build/dunelegacy_tests
```

### Running Specific Tests

```bash
# Run only PathBudget tests
./build/dunelegacy_tests "[pathbudget]"

# Run only DESYNC tests
./build/dunelegacy_tests "[desync]"

# Run only network tests
./build/dunelegacy_tests "[network]"

# List all tests
./build/dunelegacy_tests --list-tests
```

## Test Structure

Tests use Catch2's TEST_CASE macro:

```cpp
TEST_CASE("Description", "[tags]") {
    REQUIRE(condition);
    REQUIRE_FALSE(not_condition);
}
```

Tests requiring ENet initialization use the `ENetFixture`:

```cpp
TEST_CASE_METHOD(ENetFixture, "Description", "[network]") {
    // ENet is initialized here
}
```

## Adding New Tests

1. Create a new directory: `tests/YourTestCase/`
2. Create header and cpp files
3. Add source to `tests/CMakeLists.txt`
4. Use Catch2 macros for assertions

