; test02.asm  –  Error handling test (from spec)
; Every statement below should trigger a diagnostic.

label:
label:              ; duplicate label definition
        br  nonesuch    ; no such label
        ldc 08ge        ; not a valid number (bad octal digit)
        ldc             ; missing operand
        add 5           ; unexpected operand for no-operand instruction
        ldc 5, 6        ; extra content after operand
0def:                   ; bogus label name (starts with digit)
fibble;                 ; bogus mnemonic
0def                    ; bogus mnemonic (starts with digit)
