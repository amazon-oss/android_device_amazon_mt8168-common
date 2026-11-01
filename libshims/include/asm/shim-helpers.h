//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#if defined(__aarch64__)

    .text

/*
 * Defines a trampoline that branches to a function pointer in the
 * table whilst leaving the callers passed parameters untouched.
 *
 * Clobbers: x16 / x17
 */
.macro _trampoline sym, table, idx
    .globl \sym
    .type \sym, %function
    .balign 4
\sym:
    /* AAPCS64: we are free to clobber x16 / x17 */
    adrp    x16, \table + (\idx * 8)
    ldr     x16, [x16, #:lo12:\table + (\idx * 8)]
    br      x16
    .size \sym, . - \sym
.endm

/*
 * Defines a thunk that unwraps the inner class instance pointer
 * in x0 to point to the shim's allocation before branching to a
 * function pointer in the table, leaving the rest of the caller's
 * parameters untouched.
 *
 * Clobbers: x16 / x17
 */
.macro _thisthunk sym, table, idx, offset=0
    .globl \sym
    .type \sym, %function
    .balign 4
\sym:
    /*
     * Unpack inner instance pointer.
     *
     * We get passed a pointer to the caller's freshly allocated 'this'
     * as the first parameter (x0/r0) to the constructor. The shim uses
     * this memory buffer as a container to store a pointer to its own
     * 'this' allocation, which is then unpacked and forwarded to the
     * vendor blob for all non-static member method calls.
     *
     * If you're like me and still get tripped up by these weird usages
     * of double pointers, you can think of it as:
     *
     * struct fake_obj {
     *    void* real_obj;
     * };
     *
     * void* constructor(void* thiz)
     * {
     *    struct fake_obj *fake = (struct fake_obj *)thiz;
     *    fake->real_obj = malloc(REAL_OBJ_SIZE);
     *    real_constructor(fake->real_obj);
     *    return thiz;
     * }
     *
     * int some_non_static_method(void* thiz, int param)
     * {
     *    struct fake_obj *fake = (struct fake_obj *)thiz;
     *    void* real_obj = fake->real_obj; // ldr x0, [x0]
     *    return real_some_non_static_method(real_obj, param);
     * }
     *
     * void destructor(void* thiz)
     * {
     *    struct fake_obj *fake = (struct fake_obj *)thiz;
     *    real_destructor(fake->real_obj);
     *    free(fake->real_obj);
     * }
     */
    ldr     x0,  [x0, #\offset]

    /* AAPCS64: we are free to clobber x16 / x17 */
    adrp    x16, \table + (\idx * 8)
    ldr     x16, [x16, #:lo12:\table + (\idx * 8)]
    br      x16
    .size \sym, . - \sym
.endm

#elif defined(__arm__)

    .syntax unified
    .arm
    .text

.macro _trampoline sym, table, idx
    .globl \sym
    .type \sym, %function
    .balign 4
\sym:
    /* AAPCS: ip may be used as a scratch register */
    ldr     ip, 1f
0:  add     ip, pc, ip
    ldr     ip, [ip]

    bx      ip
1:  .word   \table + (\idx * 4) - (0b + 8)
    .size \sym, . - \sym
.endm

.macro _thisthunk sym, table, idx, offset=0
    .globl \sym
    .type \sym, %function
    .balign 4
\sym:
    /* Unpack inner instance pointer. */
    ldr     r0, [r0, #\offset]

    /* AAPCS: ip may be used as a scratch register */
    ldr     ip, 1f
0:  add     ip, pc, ip
    ldr     ip, [ip]

    bx      ip
1:  .word   \table + (\idx * 4) - (0b + 8)
    .size \sym, . - \sym
.endm

#endif
