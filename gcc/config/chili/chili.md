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

;;------------------------------------------------------------------------------

;; This file contains target machine instruction templates. We are
;; forced to provide at least one condition/instruction, otherwise
;; the build will fail (zero-sized array error)

;; We will extend this file later, for now we only provide two
;; instructions, a simple add (reg = reg + reg) that uses a dummy
;; condition and a nop (interestingly, gcc refuses to build without it)

(include "constraints.md")

;;------------------------------------------------------------------------------
;; Attributes
;;
;; Define the type of the instructions we are dealing with. For the time
;; being we distinguish only the most basic instructions without paying
;; attention to fancy extras.
;;------------------------------------------------------------------------------

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
;; Moves/loads/stores
;;
;; Alright, 'movsi' is like a godmother of all transfer patterns. Depending
;; on the operands it captures simple reg-reg moves as well as loads and
;; stores (and more).
;;------------------------------------------------------------------------------

(define_insn "movsi_insn" 
  [(set (match_operand:SI 0 "nonimmediate_operand" "=r,r,r,m,m")
        (match_operand:SI 1 "general_operand" "i,r,m,r,J"))]
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

;; expand properly depending on the operand types

(define_expand "movsi"
  [(set (match_operand:SI 0 "general_operand" "")
        (match_operand:SI 1 "general_operand" ""))]
  ""
  "{
     chili_expand_movi(SImode, operands);
  }"
)

;; subword register moves/stores. Loads are handled as sign-/zero
;; extensions from memory (see below). 'port8/16' can store signed
;; immediates up to 12-bit in size (wondering how this works with
;; 'port8')

(define_insn "movhi_insn"
  [(set (match_operand:HI 0 "nonimmediate_operand" "=r,m")
        (match_operand:HI 1 "nonmemory_operand" "rI,rJ"))]
  ""
  "@
   %0 = %1;
   port16[%0] = %1;"

  [(set_attr "type" "mov,store")
   (set_attr "length" "1,8")]
)

(define_insn "movqi_insn"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=r,m")
        (match_operand:QI 1 "nonmemory_operand" "rI,rJ"))]
  ""
  "@
   %0 = %1;
   port8[%0] = %1;"

  [(set_attr "type" "mov,store")
   (set_attr "length" "1,8")]
)

(define_expand "movhi"
  [(set (match_operand:HI 0 "general_operand" "")
        (match_operand:HI 1 "general_operand" ""))]
  ""
  "{
     chili_expand_movi(HImode, operands);
  }"
)

(define_expand "movqi"
  [(set (match_operand:QI 0 "general_operand" "")
        (match_operand:QI 1 "general_operand" ""))]
  ""
  "{
     chili_expand_movi(QImode, operands);
  }"
)

;;------------------------------------------------------------------------------
;; Unconditional branches
;;
;; NOTE, one of the most important instructions to provide in the beginning
;; is a direct unconditional jump.
;;------------------------------------------------------------------------------

;; direct unconditional
(define_insn "jump"
  [(set (pc) (label_ref (match_operand 0)))]
  ""
  "jump (%0);"
  [(set_attr "type" "ubranch")]
)

;; indirect unconditional
(define_insn "indirect_jump"
  [(set (pc) (match_operand:SI 0 "register_operand" "r"))]
  ""
  "jump (%0);"
  [(set_attr "type" "ubranch")]
)

;; later for call instructions
(define_insn "return"
  [(return)]
  ""
  "jump (r63);"
  [(set_attr "type" "ubranch")]
)

;;------------------------------------------------------------------------------
;; Basic arithmetics, SI mode only, allow register and imm32 operands
;;------------------------------------------------------------------------------

(define_insn "addsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (plus:SI (match_operand:SI 1 "register_operand" "r")
                 (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 + %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

(define_insn "subsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (minus:SI (match_operand:SI 1 "register_operand" "r")
                  (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 - %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

(define_insn "mulsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (mult:SI (match_operand:SI 1 "register_operand" "r")
                 (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 * %2;"
  [(set_attr "type" "mult")
   (set_attr "length" "4")]
)

;; shift left

(define_insn "ashlsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (ashift:SI (match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 << %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

;; arithmetic shift right

(define_insn "ashrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (ashiftrt:SI (match_operand:SI 1 "register_operand" "r")
                     (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 >>> %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

;; logical shift right

(define_insn "lshrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (lshiftrt:SI (match_operand:SI 1 "register_operand" "r")
                     (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 >> %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

;; single operand arithmetics

(define_insn "abssi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (abs:SI (match_operand:SI 1 "nonmemory_operand" "rI")))]
  ""
  "%0 = abs(%1);"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

(define_insn "negsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (neg:SI (match_operand:SI 1 "nonmemory_operand" "rI")))]
  ""
  "%0 = -%1;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

;; NOTE, bitwise operations are handled as arithmetics, i.e.
;; there are no conditions involved

(define_insn "andsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (and:SI (match_operand:SI 1 "register_operand" "r")
                (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 & %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (ior:SI (match_operand:SI 1 "register_operand" "r")
                (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 | %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (xor:SI (match_operand:SI 1 "register_operand" "r")
                (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "%0 = %1 ^ %2;"
  [(set_attr "type" "arith")
   (set_attr "length" "1")]
)

;;------------------------------------------------------------------------------
;; Sign/zero extension
;;
;; 'port8/16' load zero-extended values (bytes/half-words) into registers
;; 'sport8/16' load sign-extended values (bytes/half-words) into registers
;;
;; There is no dedicated zero-extension instruction for registers on our
;; target, thus, leave it to gcc to find alternatives if/when required
;;------------------------------------------------------------------------------

(define_insn "extendhisi2"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
        (sign_extend:SI (match_operand:HI 1 "nonimmediate_operand" "r,m")))]
  ""
  "@
   %0 = sext(%1);
   %0 = sport16[%1];"

  [(set_attr "type" "arith,load")
   (set_attr "length" "1,8")]
)

(define_insn "extendqisi2"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
        (sign_extend:SI (match_operand:QI 1 "nonimmediate_operand" "r,m")))]
  ""
  "@
   %0 = sext(%1);
   %0 = sport8[%1];"

  [(set_attr "type" "arith,load")
   (set_attr "length" "1,8")]
)

(define_insn "zero_extendhisi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (zero_extend:SI (match_operand:HI 1 "memory_operand" "m")))]
  ""
  "%0 = port16[%1];"

  [(set_attr "type" "load")
   (set_attr "length" "8")]
)

(define_insn "zero_extendqisi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (zero_extend:SI (match_operand:QI 1 "memory_operand" "m")))]
  ""
  "%0 = port8[%1];"

  [(set_attr "type" "load")
   (set_attr "length" "8")]
)

;;------------------------------------------------------------------------------
;; Conditional branches
;;
;; NOTE, comparison operands can be memory, like in this simple case:
;;
;; int x, y;
;; if (x > y) return 1;
;;
;; Thus, we allow general operands, otherwise the instruction will not be
;; matched ('emit_cmp_and_jump_insns', 'prepare_cmp_insn', etc. in
;; 'optabs.c') and restrict them to registers during expansion.
;;
;; Also, having the machine mode for the condition code being different
;; from 'VOIDmode' leads to 'insn_operand_matches(...)' being false,
;; since 'prepare_cmp_insn' tests the cbranch-pattern against a dummy
;; 'rtx test = gen_rtx_fmt_ee(cmp, VOIDmode, x, y)'
;;------------------------------------------------------------------------------

(define_expand "cbranchsi4"
  [(set (pc)
    (if_then_else (match_operator:VOID 0 "comparison_operator"
                    [(match_operand:SI 1 "general_operand")
                     (match_operand:SI 2 "general_operand")])
                  (label_ref (match_operand 3 "" ""))
                  (pc)))]
  ""
  "{
     chili_expand_cond_branch(operands);

     /* see auto-generated 'insn-emit.c' in build/gcc on when and
        when not to use 'DONE' */
     DONE;
  }"
)

;; signed comparison branches

(define_insn "cond_branch_eq"
  [(set (pc)
    (if_then_else (
        eq:SI (match_operand:SI 0 "register_operand" "r")
              (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 == %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_ne"
  [(set (pc)
    (if_then_else (
        ne:SI (match_operand:SI 0 "register_operand" "r")
              (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 != %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_gt"
  [(set (pc)
    (if_then_else (
        gt:SI (match_operand:SI 0 "register_operand" "r")
              (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 > %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_ge"
  [(set (pc)
    (if_then_else (
        ge:SI (match_operand:SI 0 "register_operand" "r")
              (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 >= %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_lt"
  [(set (pc)
    (if_then_else (
        lt:SI (match_operand:SI 0 "register_operand" "r")
              (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 < %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_le"
  [(set (pc)
    (if_then_else (
        le:SI (match_operand:SI 0 "register_operand" "r")
              (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 <= %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

;; unsigned condition branches, if you fail to see them in your code,
;; you most probably messed up machine modes (and/or more)

(define_insn "cond_branch_gtu"
  [(set (pc)
    (if_then_else (
        gtu:SI (match_operand:SI 0 "register_operand" "r")
               (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 (us) > %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_geu"
  [(set (pc)
    (if_then_else (
        geu:SI (match_operand:SI 0 "register_operand" "r")
               (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 (us) >= %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_ltu"
  [(set (pc)
    (if_then_else (
        ltu:SI (match_operand:SI 0 "register_operand" "r")
               (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 (us) < %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

(define_insn "cond_branch_leu"
  [(set (pc)
    (if_then_else (
        leu:SI (match_operand:SI 0 "register_operand" "r")
               (match_operand:SI 1 "nonmemory_operand" "rI"))
        (label_ref (match_operand 2))
        (pc)))]
  ""
  "if (%0 (us) <= %1) jump (%2);"
  [(set_attr "type" "cbranch")]
)

;;------------------------------------------------------------------------------

(define_insn "nop"
  [(const_int 0)]
  ""
  "nop"
)

