# A9N : Coding Gudelines

This document outlines the coding standards and practices for the project. The primary goal is to maintain high code readability and consistency throughout the codebase. Please adhere to the following guidelines when contributing.

## General Guidelines

- Prioritize small, modular functions, classes, and methods.
- Code readability takes precedence over other concerns such as performance optimization.
- Code should be self-explanatory, with meaningful naming conventions.

## C/C++ Guidelines

- Follow the `.clang-format` configuration provided in the repository.
- For C files, use the extensions `.c` for source files and `.h` for header files.
- For C++ files, use the extensions `.cpp` for source files and `.hpp` for header files.
- When working in the kernel, **do not** use fixed-width integer types (e.g., `uint32_t`, `uint64_t`). Instead, use the `a9n::word` type for word-sized data.
- Use `inline constexpr` for constant values in headers.

### Constant Grouping

- For **grouping constants**, follow these guidelines:
  - Use lowercase `enum` or `namespace` names.
    - member names must be captalized. (e.g., `some_enum::SOME_CONSTANT`)
  - If a **specific value** is required, define constants inside a `namespace` using `inline constexpr`.
  - For **tags or enumerations**, use `enum class` (C++ only). **Primitive `enum`** types should be avoided.
  
### Class/Module Boundaries

- Use `liba9n::option` or `liba9n::result` types to represent function returns at class/module boundaries. Avoid using `unwrap()`; prefer monadic operations for error handling.

### Header Files

- Include guards in header files should follow the naming pattern: `NAMESPACE_FILENAME_HPP`. The entire guard should be uppercase, and underscores should separate the namespace and filename.

### Object-Oriented Design

- Use `class` for stateful objects and free functions for stateless functionality.

## Assembly (ASM) Guidelines

- All ASM functions should have a prefix `_` to avoid naming conflicts and indicate low-level code.

## Testing Guidelines

- Test files should be named with the prefix `test_`, followed by the file name being tested. For example, `test_example.cpp` or `test_example.c`.

