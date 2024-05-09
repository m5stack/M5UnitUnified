/*!
  UnitTest for M5Utility

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.

  Each split by test_filter creates a separate binary. So they are combined by
  including cpp. Note that the /optional files are not compiled directly if not
  set to test_filter

  PlatformIO's test_filter and the hierarchy rules seem counter-intuitive.
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
