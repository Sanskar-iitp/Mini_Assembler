; test03.asm  –  SET pseudo-instruction test (from spec)
; 'val'  should be assigned value 75 (not its PC).
; 'val2' should be assigned value 66 (not its PC).
; The two ldc/adc instructions load those constant values.

val:    SET 75
        ldc val     ; loads 75 into A
        adc val2    ; adds 66, result = 141
val2:   SET 66
