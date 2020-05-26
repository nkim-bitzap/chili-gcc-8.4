/* Definitions of Ondemand's Chili v1 target machine for GNU compiler.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.

   This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/******************************************************************************/
/* General target configuration                                               */
/******************************************************************************/

/* 32 bit little endian ISA */
#define UNITS_PER_WORD 4
#define MAX_BITS_PER_WORD 32
#define BITS_BIG_ENDIAN 0
#define BYTES_BIG_ENDIAN 0
#define WORDS_BIG_ENDIAN 0
#define STRICT_ALIGNMENT 0
#define POINTER_BOUNDARY 32
#define PARM_BOUNDARY 32
#define FUNCTION_BOUNDARY 32
#define STRUCTURE_SIZE_BOUNDARY 8
#define BIGGEST_ALIGNMENT 128

/* treat pointers and function addresses as integers */
#define FUNCTION_MODE SImode
#define Pmode SImode

/* how many int's are required to hold X number of bytes */
#define CHILI_NUM_INTS(X) (((X) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

/* how many registers are required to hold a value in the given mode */
#define CHILI_NUM_REGS(MODE) CHILI_NUM_INTS(GET_MODE_SIZE(MODE))

/******************************************************************************/
/* Target register file                                                       */
/*                                                                            */
/* r0       function return value                                             */
/* r1-r6    function call arguments                                           */
/* r7-r14   callee-saved temporaries                                          */
/* r15-r54  caller-saved temporaries                                          */
/* r55-r60  reserved inline asm registers                                     */
/* r61      frame pointer                                                     */
/* r62      stack pointer                                                     */
/* r63      return address                                                    */
/******************************************************************************/

/* available register classes */
enum reg_class {
  NO_REGS,              /* no registers, expected first by GCC */              \
  SP_REGS,              /* class containing the stack pointer only */          \
  FP_REGS,              /* class containing the frame pointer only */          \
  ASM_REGS,             /* reserved inline asm registers */                    \
  GP_REGS,              /* general purpose integer registers */                \
  ALL_REGS,             /* all registers, expected as last class */            \
  LIM_REG_CLASSES       /* expected as finalizer by GCC */                     \
};

/* names of register classes */
#define REG_CLASS_NAMES                                                        \
{                                                                              \
  "NO_REGS",                                                                   \
  "SP_REG",                                                                    \
  "FP_REG",                                                                    \
  "ASM_REGS",                                                                  \
  "GP_REGS",                                                                   \
  "ALL_REGS"                                                                   \
}

/* register names, lots to type */
#define REGISTER_NAMES                                                         \
{                                                                              \
  "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",                       \
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",                      \
  "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",                      \
  "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",                      \
  "r32", "r33", "r34", "r35", "r36", "r37", "r38", "r39",                      \
  "r40", "r41", "r42", "r43", "r44", "r45", "r46", "r47",                      \
  "r48", "r49", "r50", "r51", "r52", "r53", "r54", "r55",                      \
  "r56", "r57", "r58", "r59", "r60", "r61", "r62", "r63"                       \
}

/* register class content */
#define REG_CLASS_CONTENTS                                                     \
{                                                                              \
  { 0x00000000, 0x00000000 }, /* no registers */                               \
  { 0x00000000, 0x40000000 }, /* stack pointer (r62) */                        \
  { 0x00000000, 0x20000000 }, /* frame pointer (r61) */                        \
  { 0x00000000, 0x1f800000 }, /* inline asm regs (r55-r60) */                  \
  { 0xffffffff, 0xe07fffff }, /* all but inline asm registers */               \
  { 0xffffffff, 0xffffffff }  /* all registers */                              \
}

/* list of registers not available for allocation */
#define FIXED_REGISTERS                                                        \
{                                                                              \
  /* r0-r15 */                                                                 \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                              \
  /* r16-r31 */                                                                \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                              \
  /* r32-r47 */                                                                \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                              \
  /* r48-r54 */                                                                \
  0, 0, 0, 0, 0, 0, 0,                                                         \
  /* r55-r63 */                                                                \
  1, 1, 1, 1, 1, 1, 0, 1, 0                                                    \
}

/* list of registers potentially clobbered by callee's */
#define CALL_USED_REGISTERS                                                    \
{                                                                              \
  /* r0-r6*/                                                                   \
  1, 1, 1, 1, 1, 1, 1,                                                         \
  /* r7-r14 */                                                                 \
  0, 0, 0, 0, 0, 0, 0, 0,                                                      \
  /* r15-r31 */                                                                \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                           \
  /* r32-r47 */                                                                \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                              \
  /* r48-r54 */                                                                \
  1, 1, 1, 1, 1, 1, 1,                                                         \
  /* r55-r63 */                                                                \
  1, 1, 1, 1, 1, 1, 1, 1, 0                                                    \
}

/* suggest a register allocation order */
#define REG_ALLOC_ORDER {                                                      \
  15, 16, 17, 18, 19, 20, 21, 22,      /* caller saved registers */            \
  23, 24, 25, 26, 27, 28, 29, 30,                                              \
  31, 32, 33, 34, 35, 36, 37, 38,                                              \
  39, 40, 41, 42, 43, 44, 45, 46,                                              \
  47, 48, 49, 50, 51, 52, 53, 54,                                              \
  0,                                   /* return value register */             \
  1, 2, 3, 4, 5, 6,                    /* argument registers */                \
  7, 8, 9, 10, 11, 12, 13, 14,         /* callee saved registers */            \
  61, 63, 62,                          /* fp, ra, sp */                        \
  55, 56, 57, 58, 59, 60               /* fixed asm registers */               \
}

#define N_REG_CLASSES (int)reg_class::LIM_REG_CLASSES
#define GENERAL_REGS reg_class::GP_REGS

#define REGNO_REG_CLASS(REGNO) chili_regno_to_class(REGNO)
#define REGNO_OK_FOR_BASE_P(REGNO) chili_valid_regno_for_base_p(REGNO)
#define REGNO_OK_FOR_INDEX_P(REGNO) chili_valid_regno_for_index_p(REGNO)

#define BASE_REG_CLASS reg_class::GP_REGS
#define INDEX_REG_CLASS reg_class::GP_REGS

#define FIRST_ARG_REGNUM 1
#define LAST_ARG_REGNUM 6
#define FIRST_ASM_REGNUM 55
#define LAST_ASM_REGNUM 60
#define FIRST_CALLEE_SAVED_REGNUM 7
#define LAST_CALLEE_SAVED_REGNUM 14
#define STACK_POINTER_REGNUM 62
#define FRAME_POINTER_REGNUM 61
#define RET_VALUE_REGNUM 0
#define RET_ADDRESS_REGNUM 63
#define FIRST_PSEUDO_REGISTER 64
#define MAX_REGS_PER_ADDRESS 1
#define ARG_POINTER_REGNUM FRAME_POINTER_REGNUM
#define NUM_ARG_REGISTERS LAST_ARG_REGNUM

#define ELIMINABLE_REGS {{ FRAME_POINTER_REGNUM }}
#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET) \
  (OFFSET) = chili_initial_elimination_offset((FROM), (TO))

/******************************************************************************/
/* Memory, stack, function args                                               */
/******************************************************************************/

#define MOVE_MAX 4
#define SLOW_BYTE_ACCESS 0
#define PUSH_ARGS 0
#define ACCUMULATE_OUTGOING_ARGS 1
#define STACK_BOUNDARY 32
#define STACK_GROWS_DOWNWARD 1
#define FRAME_GROWS_DOWNWARD 1
#define STACK_POINTER_OFFSET -8
#define EXIT_IGNORE_STACK 1
#define MAX_ARGS_IN_REGISTERS 6

/* cumulative argument info */
typedef struct
{
  int num_reg_args;
  int num_args;
} chili_cumulative_arg_info;

#define CUMULATIVE_ARGS chili_cumulative_arg_info

/* init cumulative args */
#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, INDIRECT, N_NAMED_ARGS)     \
  chili_init_cumulative_args(&CUM, FNTYPE, LIBNAME, INDIRECT, N_NAMED_ARGS);

/* misc. function stuff */
#define FUNCTION_ARG_REGNO_P(N)                                                \
  ((N >= FIRST_ARG_REGNUM) && (N <= LAST_ARG_REGNUM))

#define FIRST_PARM_OFFSET(FNDECL) 0

/******************************************************************************/
/* Misc.                                                                      */
/******************************************************************************/

#define DEFAULT_SIGNED_CHAR 1

/* the value that is used to represent a comparison that evaluates to
   'true', usually 1 or -1 */
#define STORE_FLAG_VALUE 1

/* for nested functions only */
#define TRAMPOLINE_SIZE 64
#define TRAMPOLINE_ALIGNMENT 32

/* treat 'case' labels as integers */
#define CASE_VECTOR_MODE SImode

/* no profiler support yet */
#define FUNCTION_PROFILER(FILE, LABELNO)                                       \
  do {                                                                         \
  } while(0)

/* target CPU builtins */
#define TARGET_CPU_CPP_BUILTINS()                                              \
  do {                                                                         \
    builtin_assert ("cpu=chili");                                              \
    builtin_assert ("machine=chili");                                          \
    builtin_define ("__chili__");                                              \
    builtin_define ("__CHILI__");                                              \
    builtin_define ("__CHILI_SOFT_FLOAT__");                                   \
  } while (0)

/******************************************************************************/
/* Assembler                                                                  */ 
/******************************************************************************/

/* how to output alignment directives */
#define ASM_OUTPUT_ALIGN(STREAM, LOG)                                          \
  do {                                                                         \
    if (LOG != 0)                                                              \
      fprintf (STREAM, "\t.align\t%d\n", 1 << (LOG));                          \
  } while (0)

/* how to output labels */
#define ASM_OUTPUT_LABEL(FILE, NAME)                                           \
  do {                                                                         \
    assemble_name (FILE, NAME);                                                \
    fputs (":\n", FILE);                                                       \
  } while (0)

/* not entirely sure about the following directives since missing in my
   docs. Thus, guessing here */
#define ASM_APP_ON "#APP\n"
#define ASM_APP_OFF "#NO_APP\n"

#define TEXT_SECTION_ASM_OP "\t.code"
#define DATA_SECTION_ASM_OP "\t.data"

// #define READONLY_DATA_SECTION_ASM_OP "\t.rodata"
#define BSS_SECTION_ASM_OP "\t.bss"

#undef GLOBAL_ASM_OP
#define GLOBAL_ASM_OP "\t.global\t"

#undef TARGET_ASM_ALIGNED_SI_OP
#define TARGET_ASM_ALIGNED_SI_OP "\t.word\t"

