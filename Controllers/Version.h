#pragma once

// The version string is injected at build time by CMake from the top-level
// VERSION file (see add_compile_definitions in CMakeLists.txt). The fallback
// below only applies to tooling that compiles without that definition.
#ifndef MEDIAFLYOUTS_VERSION
#define MEDIAFLYOUTS_VERSION "0.0.0-dev"
#endif
