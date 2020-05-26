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

#ifndef __CHILI_PROTOS_H__
#define __CHILI_PROTOS_H__

/* Messy here. Export target hooks we provide/implement in 'chili.h'
   and 'chili.c' that need to be globally visible.

   If you provide this file after configure and keep getting the same
   error, you may want to reconfigure/rebuild */

extern bool chili_valid_movsi_insn(machine_mode, rtx operands[2]);

extern enum reg_class chili_regno_to_class(int);
extern int chili_valid_regno_for_base_p(int);
extern int chili_valid_regno_for_index_p(int);

extern void chili_init_cumulative_args(CUMULATIVE_ARGS *ca,
                                       tree fn_type,
                                       rtx libname,
                                       tree fn_decl,
                                       int num_named);

extern HOST_WIDE_INT chili_initial_elimination_offset(int, int);

extern void chili_expand_movi(machine_mode, rtx *operands);
extern void chili_expand_cond_branch(rtx *operands);

#endif /* __CHILI_PROTOS_H__ */

