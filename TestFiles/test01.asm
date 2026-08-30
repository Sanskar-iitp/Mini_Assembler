; test01.asm  –  Valid assembly test (from spec)
; No errors should be produced.
; One warning expected: 'label' is defined but never used.

label:              ; an unused label  (→ warning, not error)

        ldc 0
        ldc -5
        ldc +5

loop:   br  loop    ; infinite loop — offset must be -1
        br  next    ; comment says offset should be zero
next:
        ldc loop    ; load code address of 'loop'
        ldc var1    ; forward reference to data label

var1:   data 0      ; a variable holding zero
