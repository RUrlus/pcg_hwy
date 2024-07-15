/* Copyright 2024 R. Urlus [rurlus.dev@gmail.com]
 * SPDX-License-Identifier: 0BSD OR MIT-0
 */
#pragma once

/* TODO
 *
 * Plan in to create working scalar implementation that can be used as a
 * reference. Open steps:
 * - default seed
 * - figure out how to set a stream (aka different increment afaik)
 * - make it compatible with `uniform_random_bit_generator`
 * - wrap it in a nice class/struct
 *
 * The SIMD variant will use the various streams to create uncorrelated states
 */

#include <cstdint>

namespace pcg_hwy {

using pcg_uint_t = uint64_t;

#if defined(__SIZEOF_INT128__)
using pcg_ulong_t = __uint128_t;
#else
#error "uint128_t support not detected but is required."
#endif

constexpr int PCG_UINT_BITS = 64;
constexpr pcg_ulong_t PCG_DEFAULT_INC =
    (static_cast<pcg_ulong_t>(6364136223846793005ULL) << 64 |
     static_cast<pcg_ulong_t>(1442695040888963407ULL));

constexpr uint64_t PCG_DEFAULT_CHEAP_MULT = 0xda942042e4dd58b5ULL;

struct pcg64_t {
  pcg_ulong_t state, inc;
};

inline uint64_t pcg64_random(pcg64_t *rng) {
  // cheap (half-width) multiplier from reference pcg-cpp/pcg_random.hpp L-175
  const uint64_t mul = 0xda942042e4dd58b5ULL;
  /* linear congruential generator */
  pcg_ulong_t state = rng->state;
  rng->state = state * mul + rng->inc;
  /* DXSM (double xor shift multiply) permuted output */
  uint64_t hi = (uint64_t)(state >> 64);
  uint64_t lo = (uint64_t)(state | 1);
  hi ^= hi >> 32;
  hi *= mul;
  hi ^= hi >> 48;
  hi *= lo;
  return hi;
}

inline pcg64_t pcg_seed(pcg64_t rng) {
  /* must ensure rng.inc is odd */
  rng.inc = (rng.inc > 0) ? (rng.inc << 1) | 1 : PCG_DEFAULT_INC;
  rng.state += rng.inc;
  pcg64_random(&rng);
  return (rng);
}

} // namespace pcg_hwy
