//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

// clang-format off

#if defined _WIN32 || defined __CYGWIN__
  #define FLEXFS_API_IMPORT __declspec(dllimport)
  #define FLEXFS_API_EXPORT __declspec(dllexport)
  #define FLEXFS_API_LOCAL
#else
  #if __GNUC__ >= 4 // TODO: clang?
    #define FLEXFS_API_IMPORT __attribute__ ((visibility ("default")))
    #define FLEXFS_API_EXPORT __attribute__ ((visibility ("default")))
    #define FLEXFS_API_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define FLEXFS_API_IMPORT
    #define FLEXFS_API_EXPORT
    #define FLEXFS_API_LOCAL
  #endif
#endif

#ifdef FLEXFS_SHARED // compiled as a shared library
  #ifdef FLEXFS_SHARED_EXPORTS // defined if we are building the shared library
    #define FLEXFS_EXPORT FLEXFS_API_EXPORT
  #else
    #define FLEXFS_EXPORT FLEXFS_API_IMPORT
  #endif // FLEXFS_SHARED_EXPORTS
  #define FLEXFS_LOCAL FLEXFS_API_LOCAL
#else // compiled as a static library
  #define FLEXFS_EXPORT
  #define FLEXFS_LOCAL
#endif // FLEXFS_SHARED
