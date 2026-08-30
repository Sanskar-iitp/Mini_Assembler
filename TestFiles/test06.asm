; test06.asm  –  Compute sum of integers 1..10 = 55
; Custom success test to demonstrate additional assembler capability.
;
; Stack frame (SP-relative):
;   SP+0  i    loop counter (1 .. 10)
;   SP+1  sum  running total
;
; Expected result: mem[total] = 55 (0x37) after HALT.

        ldc  0x1000
        a2sp
        adj  -2             ; two locals: i and sum

        ldc  1
        stl  0              ; i = 1

        ldc  0
        stl  1              ; sum = 0

; Loop: while i <= 10
loop:
        ldl  0              ; A = i
        adc  -11            ; A = i - 11
        brlz body           ; i < 11  → keep looping
        br   done

body:
        ldl  1              ; A = sum
        ldl  0              ; B = sum,  A = i
        add                 ; A = sum + i
        stl  1              ; sum = sum + i

        ldl  0              ; A = i
        adc  1              ; A = i + 1
        stl  0              ; i = i + 1
        br   loop

done:
        ; store result into 'total'
        ldl  1              ; A = sum  (should be 55 = 0x37)
        ldc  total          ; B = sum,  A = &total
        stnl 0              ; mem[total] = sum
        HALT

total:  data 0              ; result stored here
