;; Predicate definitions for OnDemand Chili v1 architecture.
;;
;; Copyright (C) 2009-2018 Free Software Foundation, Inc.
;;
;; This file is part of GCC.
;;
;; GCC is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published
;; by the Free Software Foundation; either version 3, or (at your
;; option) any later version.
;;
;; GCC is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
;; License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;;  <http://www.gnu.org/licenses/>.

;; Operand that is allowed to be a register or an int constant
(define_predicate "register_imm_operand"
  (ior (match_operand 0 "register_operand")
       (match_code "const_int")))

(define_predicate "call_target_operand"
  (ior (match_operand 0 "register_operand")
       (match_code "symbol_ref")))

