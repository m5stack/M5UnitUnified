/*
  UnitTest for M5Utility

  Each split by test_filter creates a separate binary. So they are combined by
  including cpp. Note that the /optional files are not compiled directly if not
  set to test_filter

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

#include "optional/assignment.cpp"
#include "optional/bases.cpp"
#include "optional/constexpr.cpp"
#include "optional/constructors.cpp"
#include "optional/emplace.cpp"
#include "optional/extensions.cpp"
#include "optional/hash.cpp"
#include "optional/in_place.cpp"
#include "optional/issues.cpp"
#include "optional/make_optional.cpp"
#include "optional/noexcept.cpp"
#include "optional/nullopt.cpp"
#include "optional/observers.cpp"
#include "optional/relops.cpp"
#include "optional/swap.cpp"
