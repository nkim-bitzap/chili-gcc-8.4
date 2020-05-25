;; GCC machine description for Ondemand Chili v1 architecture.
;; Copyright (C) 2001-2018 Free Software Foundation, Inc.

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; GCC is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
;; License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.

;; Attributes.

;; Define the type of the instructions we are dealing with, for the time being
;; we only distinguish between the most basic instruction types without paying
;; attention to fancy extras
(define_attr "type"
  "unknown,nop,mov,arith,logic,sex,test,mult,load,store,cbranch,ubranch,
   call, libcall"
  (const_string "unknown"))

(define_attr "mode"
  "unknown,none,SI"
  (const_string "unknown"))

(define_attr "length"
  "" (const_int 1))

;;------------------------------------------------------------------------------
;; Moves/loads/stores                                                         ;;
;;                                                                            ;;
;; Alright, "movsi" is like a godmother of all instruction patterns. Opposite ;;
;; to the assumption that it is just a reg-reg/imm move, it is actually       ;;
;; supposed to handles all kinds of data moves, including the ones from/to    ;;
;; memory, since there are no explicit "load" or "store" patterns in the RTL, ;;
;; except for the operands marked MEM. Thus, in order to avoid complications  ;;
;; during the reload when things get spilled and thus turn into MEM refs, we  ;;
;; need to make sure each movsi is expanded properly in order to able to      ;;
;; handle all cases                                                           ;;
;;------------------------------------------------------------------------------

(define_insn "movsi_insn"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=r,r,r,m,m")
        (match_operand:SI 1 "general_operand" "i,r,m,r,i"))]
  "chili_valid_movsi_insn(SImode, operands)"
  "@
   %0 = %1;
   %0 = %1;
   %0 = port32[%1];
   port32[%0] = %1;
   port32[%0] = %1;"

  [(set_attr "type" "mov,mov,load,store,store")
   (set_attr "length" "1,1,8,8,8")]
)

(define_expand "movsi"
  [(set (match_operand:SI 0 "general_operand" "")
        (match_operand:SI 1 "general_operand" ""))]
  ""
  "{
     chili_expand_movsi(operands);
     DONE;
  }"
)

;; -----------------------------------------------------------------------------
;; nop                                                                        ;;
;; -----------------------------------------------------------------------------

(define_insn "nop"
  [(const_int 0)]
  ""
  "nop"
  [(set_attr "type" "arith")]
)

