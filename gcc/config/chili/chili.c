/* Implementation of the Ondemand Chili v1 GNU compiler backend
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

#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "gimple.h"
#include "cfghooks.h"
#include "df.h"
#include "memmodel.h"
#include "tm_p.h"
#include "stringpool.h"
#include "attribs.h"
#include "optabs.h"
#include "regs.h"
#include "emit-rtl.h"
#include "recog.h"
#include "diagnostic-core.h"
#include "cfgrtl.h"
#include "output.h"
#include "calls.h"
#include "alias.h"
#include "explow.h"
#include "expr.h"
#include "reload.h"
#include "langhooks.h"
#include "gimplify.h"
#include "builtins.h"
#include "dumpfile.h"
#include "hw-doloop.h"
#include "rtl-iter.h"
#include "tm-constrs.h"
#include "pretty-print.h"
#include "print-rtl.h"

/* NOTE, this file has to be included after the generic ones above */
#include "target-def.h"

/* Forward declarations of hooks we are going to assign/implement below */
static bool chili_lra_p(void);
static bool chili_legitimate_address_p(machine_mode, rtx, bool);
static bool chili_must_pass_in_stack(machine_mode, const_tree);

static rtx chili_function_arg(
  cumulative_args_t, machine_mode, const_tree, bool named);

static void chili_function_arg_advance(
  cumulative_args_t, machine_mode, const_tree, bool);

static rtx chili_function_value(const_tree, const_tree, bool);
static bool chili_function_value_regno_p (const unsigned int);

static void chili_print_operand(FILE *, rtx, int);
static void chili_print_operand_address(FILE *, machine_mode, rtx);

static void chili_init_libfuncs();
static rtx chili_libcall_value(machine_mode, const_rtx);

/* Additional to definitions in 'chili.h' we implement more complex hooks
   in this file. Things that need to be globally visible (e.g. referenced
   by the machine description) are exported in 'chili-protos.h' */

#undef TARGET_LRA_P
#define TARGET_LRA_P chili_lra_p

#undef TARGET_FUNCTION_VALUE
#define TARGET_FUNCTION_VALUE chili_function_value

#undef TARGET_FUNCTION_VALUE_REGNO_P
#define TARGET_FUNCTION_VALUE_REGNO_P chili_function_value_regno_p

#undef TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG chili_function_arg

#undef TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE chili_function_arg_advance

#undef TARGET_MUST_PASS_IN_STACK
#define TARGET_MUST_PASS_IN_STACK chili_must_pass_in_stack

#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P chili_legitimate_address_p

#undef TARGET_INIT_LIBFUNCS
#define TARGET_INIT_LIBFUNCS chili_init_libfuncs

#undef TARGET_LIBCALL_VALUE
#define TARGET_LIBCALL_VALUE chili_libcall_value

#undef TARGET_PRINT_OPERAND
#define TARGET_PRINT_OPERAND chili_print_operand

#undef TARGET_PRINT_OPERAND_ADDRESS
#define TARGET_PRINT_OPERAND_ADDRESS chili_print_operand_address

struct gcc_target targetm = TARGET_INITIALIZER;

/******************************************************************************/
/* Return true if you prefer to use LRA instead of the original RA. For the
   time being, stick to the old reloader */

static bool chili_lra_p(void)
{
  return false;
}

/******************************************************************************/
/* Since a register can generally belong to more than just one class, we
   specify the smallest one */

enum reg_class chili_regno_to_class (int regno)
{
  if (regno >= 0 && regno < FIRST_PSEUDO_REGISTER)
  {
    if (regno == STACK_POINTER_REGNUM)
      return reg_class::SP_REGS;
    else if (regno == FRAME_POINTER_REGNUM)
      return reg_class::FP_REGS;
    else return reg_class::GP_REGS;
  }

  return reg_class::NO_REGS;
}

/******************************************************************************/
/* Memory can be addressed by having an arbitrary base register, thus, no
   restrictions */

int chili_valid_regno_for_base_p(int regno)
{
  /* physical registers only, virtual registers make
     actually no sense here */
  if (regno >= 0 && regno < FIRST_PSEUDO_REGISTER)
  {
    return 1;
  }

  return 0;
}

/******************************************************************************/
/* The same as for address base registers applies to index registers, no
   restrictions */

int chili_valid_regno_for_index_p(int regno)
{
  return chili_valid_regno_for_base_p(regno);
}

/******************************************************************************/
/* Specify whether 'regno' corresponds to a function argument register */

static bool chili_valid_arg_regno(int regno)
{
  return regno >= FIRST_ARG_REGNUM && regno <= LAST_ARG_REGNUM;
}

/******************************************************************************/
/* Check whether the specified register is a function value return register */

static bool chili_function_value_regno_p(const unsigned int regno)
{
  return (RET_VALUE_REGNUM == regno);
}

/******************************************************************************/
/* See if we are dealing with integral types with length matching the return
   register width (i.e. 32 bit). If not, just create a dummy rtx for the
   return value */

static rtx chili_function_value(const_tree ret_type,
                                const_tree fn_type ATTRIBUTE_UNUSED,
                                bool outgoing ATTRIBUTE_UNUSED)
{
  if (INTEGRAL_TYPE_P(ret_type)
  && TYPE_PRECISION(ret_type) < BITS_PER_WORD)
    return gen_rtx_REG(SImode, RET_VALUE_REGNUM);

  /* not yet sure how a result is split into two registers */
  return gen_rtx_REG(TYPE_MODE(ret_type), RET_VALUE_REGNUM);
}

/******************************************************************************/
/* Advance in the function argument list past argument given by mode/type,
   i.e. extend the cumulative list by the argument */

static void chili_function_arg_advance(cumulative_args_t cargs,
                                       machine_mode mode ATTRIBUTE_UNUSED,
                                       const_tree type ATTRIBUTE_UNUSED,
                                       bool named ATTRIBUTE_UNUSED)
{
  return;
}

/******************************************************************************/
/* Specify what kind of arguments can't be passed in registers */

static bool chili_must_pass_in_stack(machine_mode mode,
                                     const_tree type)
{
  return false;
}

/******************************************************************************/
/* Need to tell what is considered to be a 'legitimate address'. Allow just
   anything for the first build */

static bool chili_legitimate_address_p(machine_mode mode,
                                       rtx x,
                                       bool strict)
{
  /* NOTE, allow integral modes up to a single integer */
  if (SImode == mode || HImode == mode || QImode == mode)
  {
    /* single operand addressing, register or immediate */
    if (REG == GET_CODE(x) || CONST_INT == GET_CODE(x))
    {
      return true;
    }
    else if (PLUS == GET_CODE(x))
    {
      /* base + offset addressing */
      int base_code = GET_CODE(XEXP(x, 0));
      int off_code = GET_CODE(XEXP(x, 1));

      if ((REG == off_code || CONST_INT == off_code)
      && (REG == base_code))
      {
        return true;
      }
    }
  }

  return false;
}

/******************************************************************************/
/* If frame register elimination is supported/desired, compute the initial
   elimination offset */

HOST_WIDE_INT chili_initial_elimination_offset (int from,
                                                int to ATTRIBUTE_UNUSED)
{
  return 0;
}

/******************************************************************************/
/* Init cumulative arguments for a function call */

void chili_init_cumulative_args(CUMULATIVE_ARGS *cum,
                                tree fntype ATTRIBUTE_UNUSED,
                                rtx libname ATTRIBUTE_UNUSED,
                                tree fndecl ATTRIBUTE_UNUSED,
                                int num_named ATTRIBUTE_UNUSED)
{
  return;
}

/******************************************************************************/
/* Determine where to put an argument to a function. Value is zero to push
   argument on the stack, or a register in which to store the argument. */

static rtx chili_function_arg(cumulative_args_t cargs,
                              machine_mode mode,
                              const_tree type,
                              bool named)
{
  return NULL_RTX;
}

/******************************************************************************/
/* When building in the beginning, we are forced to provide at least one
   condition (zero-sized array error). Thus, provide a dummy to overcome
   this.

   NOTE, it is erroneous to use this to reject particular operand combos,
   i.e. having one and the same instruction alternative being ok for
   for some operand combinations and not ok for other (if you want to
   do this, use additional instruction alternatives). Use conditions to
   allow/reject entire patterns depending on a given ISA (e.g. see Arm,
   some patterns are only allowed in A32 resp. T32 mode */

bool chili_valid_movsi_insn(machine_mode mode, rtx operands[2])
{
  return true;
}

/******************************************************************************/
/* When expanding movsi/movhi/movqi in our simple case, we only need to check
   for memory to memory patterns which are not supported. In case we detect
   one, we simply force the 'source' operand to be put into a register prior
   to expansion/selection */

void chili_expand_movi(machine_mode mode, rtx *operands)
{
  if (MEM == GET_CODE(operands[0]) 
  && MEM == GET_CODE(operands[1]))
  {
    /* allow for all integral modes up to int32 */
    operands[1] = force_reg(mode, operands[1]);
  }

  /* for the time being the default expander works fine
  emit_insn(gen_movsi_insn(operands[0], operands[1])); */
}

/******************************************************************************/
/* The expansion code for conditional branches is actually similar to the
   default expansion code in 'insn-emit.c' with the major difference of
   the mode for the condition operator, which we rewrite to be 'SImode'
   instead of 'VOIDmode' by default */

void chili_expand_cond_branch(rtx *operands)
{
  rtx cc = operands[0];
  rtx cmp0 = operands[1];
  rtx cmp1 = operands[2];
  rtx dst = operands[3];

  machine_mode mode = GET_MODE(cmp0);
  rtx_code cc_code = GET_CODE(cc);

  gcc_assert(dst != NULL);

  /* in some cases both compare operands can be memory */
  if (!register_operand(cmp0, mode))
    cmp0 = force_reg(mode, cmp0);

  if (REG != GET_CODE(cmp1) && CONST_INT != GET_CODE(cmp1))
    cmp1 = force_reg(mode, cmp1);

  /* create a new CC rtx for SImode with 'cmp0' and 'cmp1'
     as operands */
  rtx code = gen_rtx_fmt_ee(cc_code, mode, cmp0, cmp1);

  /* combine with the 'dst' label and assemble a jump */
  rtx label = gen_rtx_LABEL_REF(VOIDmode, dst);

  rtx insn = gen_rtx_SET(pc_rtx,
    gen_rtx_IF_THEN_ELSE(VOIDmode, code, label, pc_rtx));

  /* pattern 'jump_insn(if_then_else(cc, then, else))' */
  emit_jump_insn(insn);
}

/******************************************************************************/
/* Chili does not provide a flag/status register, instead the result of
   a condition test is allowed to be put in arbitrary registers */

void chili_expand_store_cond(rtx *operands)
{
  rtx dst = operands[0];
  rtx cc = operands[1];
  rtx cmp0 = operands[2];
  rtx cmp1 = operands[3];

  gcc_assert(dst != NULL);

  machine_mode mode = GET_MODE(cmp0);

  if (!register_operand(cmp0, mode))
    cmp0 = force_reg(mode, cmp0);

  if (!register_imm_operand(cmp1, mode))
    cmp1 = force_reg(mode, cmp1);

  enum rtx_code cc_code = GET_CODE(cc);

  /* create a node for the comparison itself first */
  rtx code = gen_rtx_fmt_ee(cc_code, mode, cmp0, cmp1);
  rtx new_dst = dst;

  if (REG == GET_CODE(dst))
  {
    if (can_create_pseudo_p())
    {
      /* Chili does not provide a direct way to store the result of
         a comparison, thus, we implement this by introducing a new
         pseudo ('vr4') as an intermediate destination and write the
         value at the end into the original destination ('vr3'):

         vr4 = 0;
         if (vr1 == vr2) vr4 = 1;
         vr3 = vr4;
     */

      rtx new_dst = gen_reg_rtx(GET_MODE(dst));

      emit_move_insn(new_dst, GEN_INT(!STORE_FLAG_VALUE));
      emit_move_insn(new_dst, code);
      code = new_dst;
    }
    else {
      /* a way to deal with cstore without additional pseudos is to
         issue the same test instruction but with the condition and
         the flag value being reversed, i.e.:

         if (r1 == r2) r4 = 1;
         if (r1 != r2) r4 = 0;

         Can't find much about hardware restrictions in my docs, so,
         let's not bother with that just yet */
      gcc_unreachable();
    }
  }

  /* now write to a register upon comparison result */
  emit_move_insn(dst, code);
}

/******************************************************************************/

void chili_expand_call(rtx *operands)
{
  rtx body = operands[0];
  rtx dst = XEXP(body, 0);
  rtx something = XEXP(body, 1);

  /* everything that is not a symbol references goes into a register,
     resulting in an indirect call */
  if (SYMBOL_REF != GET_CODE(dst))
  {
    dst = force_reg(FUNCTION_MODE, dst);
  }

//  emit_call_insn(dst);
}

/******************************************************************************/
/* Register/rename additional target libfuncs here */

static void chili_init_libfuncs(void)
{
  return;
}

/******************************************************************************/
/* Generate a return value rtx for libcalls in the given mode */

static rtx chili_libcall_value(machine_mode mode,
                               const_rtx fun)
{
  return NULL_RTX;
}

/******************************************************************************/
/* Print instruction operands properly. Memory operands should trigger the
   the 'print_operand_address' hook (i.e.'chili_print_operand_address') */

static void chili_print_operand(FILE *file,
                                rtx op,
                                int letter)
{
  if (NULL == op)
  {
    /* use '@' (see PRINT_OPERAND_PUNCT_VALID_P in 'chili.h')
       to substitute macros (e.g. 'STORE_FLAG_VALUE') in the
       insn template output string without operands. I.e.
       can't reference 'op' here */
    if (letter == '@')
    {
      fprintf(file, "%d", STORE_FLAG_VALUE);
    }
  }
  else
  {
    if (REG == GET_CODE(op))
    {
      fprintf(file, "%s", reg_names[REGNO(op)]);
    }
    else if (MEM == GET_CODE(op))
    {
      /* use a separate hook for memory address operands */
      output_address(GET_MODE(op), XEXP(op, 0));
    }
    else
    {
      /* this includes constants and all other constant
         expressions including labels, symbols and floating
         points as integers */
      output_addr_const(file, op);
    }
  }
}

/******************************************************************************/
/* Print memory address operands properly (e.g. 'port[r62 + -4]') */

static void chili_print_operand_address(FILE *file,
                                        machine_mode mode,
                                        rtx addr)
{
  if (NULL != addr)
  {
    if (REG == GET_CODE(addr))
    {
      fprintf(file, "%s", reg_names[REGNO(addr)]);
      return;
    }
    else if (CONST_INT == GET_CODE(addr))
    {
      fprintf(file, "%x", INTVAL(addr));
      return;
    }
    else if (PLUS == GET_CODE(addr))
    {
      rtx base = XEXP(addr, 0);
      rtx offset = XEXP(addr, 1);

      int base_code = GET_CODE(base);
      int off_code= GET_CODE(offset);

      if (REG == base_code)
      {
        if (REG == off_code)
        {
          fprintf(file, "%s + %s",
            reg_names[REGNO(base)], reg_names[REGNO(offset)]);
          return;
        }
        else if (CONST_INT == off_code)
        {
          fprintf(file, "%s + %d",
            reg_names[REGNO(base)], INTVAL(offset));
          return;
        }
      }
    }
  }
}

