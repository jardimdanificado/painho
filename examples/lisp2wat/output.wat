(module
  (memory $mem 1) (export "memory" (memory $mem))
  (export "fib" (func $fib))
  (export "fact" (func $fact))
  (export "mem_test" (func $mem_test))
  
  (func $fib (param $n i32) (result i32)
    
    (local.get $n) (i32.const 1) i32.le_s
        (if (result i32)
          (then (local.get $n))
          (else (local.get $n) (i32.const 1) i32.sub (call $fib) (local.get $n) (i32.const 2) i32.sub (call $fib) i32.add)
        )
  )

  (func $fact (param $n i32) (result i32)
    (local $res i32)
    (i32.const 1)
        (local.set $res)
    (loop $l
          (local.get $n) (i32.const 1) i32.gt_s
        (if
          (then
            (local.get $res) (local.get $n) i32.mul
        (local.set $res)
(local.get $n) (i32.const 1) i32.sub
        (local.set $n)
(i32.const 1)
        (br_if $l)
          )
        )
        )
    (local.get $res)
  )

  (func $mem_test (param $addr i32) (param $val i32) (result i32)
    
    (local.get $addr) (local.get $val) i32.store
    (local.get $addr) i32.load
  )
)