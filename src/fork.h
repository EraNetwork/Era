// Copyright (c) 2016-2018 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ERA_FORK_H
#define ERA_FORK_H

#include "bignum.h"

/** System Upgrade 01 */
// 2 * 60 * 24 * 7 * 2 = 40320 blocks (Two-week fork buffer)
// 431858 + 40320 (Two-weeks from current reference block)
static const int64_t nUpgrade_01 = 472178; // Halving system implemented

#endif // ERA_FORK_H
