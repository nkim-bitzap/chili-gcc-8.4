;; GCC machine description for Ondemand Chili v1 architecture.
;; Copyright (C) 2001-2018 Free Software Foundation, Inc.
;; Contributed by Nick Kim at (no longer existant as of now) at Ondemand.

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
;; Constraints
;;
;; Basic constraints, define a range and pick a letter (see gcc internals).
;; When implementing instructions, specify constraints by using this letter.
;;
;; For constraints defined for our chili target we use capital letters.
;; For every letter in use (e.g. J) a function will be generated
;; (e.g. satisfies_constraint_J) that subsequently can be used to build
;; target predicates.
;;
;; NOTE, be careful here, faulty constraints are often a source of obscure
;; and hard to find errors.
;;------------------------------------------------------------------------------

(define_register_constraint "D" "GP_REGS"
  "@internal
   General-purpose machine registers.")

(define_constraint "I"
  "32-bit signed integer constraint."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, -2147483648, 2147483647)")))

(define_constraint "K"
  "32-bit unsigned constraint."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 4294967295)")))

(define_constraint "J"
  "12-bit signed integer constraint."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, -2048, 2047)")))

