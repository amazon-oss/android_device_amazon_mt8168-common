//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#include <asm/shim-helpers.h>

#define MEMBER(sym) _thisthunk sym, func_ptr_tbl, __COUNTER__
#define STATIC(sym) _trampoline sym, func_ptr_tbl, __COUNTER__
#include "symbols.h"

    .section .note.GNU-stack, "", %progbits
