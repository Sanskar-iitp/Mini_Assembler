; Sanskar Agrawal
; Bubble Sort in SIMPLEX Assembly

        ldc  0x1000         ; set stack pointer to 0x1000
        a2sp
        adj  -6             ; allocate 6 word frame

        ; pre-compute n-1 and store in local[5]

        ldc  n
        ldnl 0              ; A = n
        adc  -1             ; A = n-1
        stl  5              ; local[5] = n-1

        ; i = 0
        ldc  0
        stl  0              ; local[0] = i = 0
outer:
        ; check i < n-1  →  (n-1) - i > 0
        ldl  5              ; A = n-1
        ldl  0              ; B = n-1,  A = i
        sub                 ; A = (n-1) - i
        brz  outerend      ; i == n-1  → done
        brlz outerend      ; i >  n-1  → done

        ; j = 0
        ldc  0
        stl  1              ; local[1] = j = 0
inner:
        ; check j < (n-1-i)  →  limit - j > 0
        ldl  5              ; A = n-1
        ldl  0              ; B = n-1,  A = i
        sub                 ; A = limit = (n-1) - i
        ldl  1              ; B = limit, A = j
        sub                 ; A = limit - j
        brz  innerend
        brlz innerend

        ; addr_j = arr + j
        ldc  arr            ; A = base address of arr
        ldl  1              ; B = arr,  A = j
        add                 ; A = arr + j
        stl  2              ; local[2] = addr_j

        ; load arr[j]
        ldl  2              ; A = addr_j
        ldnl 0              ; A = mem[addr_j]   = arr[j]
        stl  3              ; local[3] = arr[j]

        ; load arr[j+1]
        ldl  2              ; A = addr_j
        ldnl 1              ; A = mem[addr_j+1] = arr[j+1]
        stl  4              ; local[4] = arr[j+1]

        ; compare: if arr[j+1] - arr[j] < 0  →  swap
        ldl  4              ; A = arr[j+1]
        ldl  3              ; B = arr[j+1],  A = arr[j]
        sub                 ; A = arr[j+1] - arr[j]
        brlz swap           ; arr[j+1] < arr[j]  →  need to swap
        br   noswap
swap:
        ; arr[j] ← arr[j+1]
        ldl  4              ; A = arr[j+1]
        ldl  2              ; B = arr[j+1],  A = addr_j
        stnl 0              ; mem[addr_j + 0] = arr[j+1]

        ; arr[j+1] ← arr[j]
        ldl  3              ; A = arr[j]
        ldl  2              ; B = arr[j],  A = addr_j
        stnl 1              ; mem[addr_j + 1] = arr[j]
noswap:
        ldl  1              ; A = j
        adc  1              ; A = j + 1
        stl  1              ; j = j + 1
        br   inner

innerend:
        ; i++
        ldl  0              ; A = i
        adc  1              ; A = i + 1
        stl  0              ; i = i + 1
        br   outer

outerend:
        HALT
; Unsorted array: 64 34 25 12 22 11 90 5
n:      data 8
arr:    data 64
        data 34
        data 25
        data 12
        data 22
        data 11
        data 90
        data 5
