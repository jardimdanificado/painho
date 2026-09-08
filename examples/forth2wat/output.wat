(module
  (memory 1)
  (export "memory" (memory 0))
  (global $sp (mut i32) (i32.const 64))
  (export "SQUARE" (func $__export_SQUARE))
  (export "PYTHAGORAS" (func $__export_PYTHAGORAS))
  (export "FACTORIAL" (func $__export_FACTORIAL))
  (export "MAX_VAL" (func $__export_MAX_VAL))
  
  (func $SQUARE
    (local $__a i32)
    (local $__b i32)
    (local $__c i32)
    (local $__i i32)
    (local $__limit i32)
    ;; Corpo traduzido da palavra Forth
    (global.get $sp) (i32.const 4) i32.add
          (global.get $sp) i32.load
          i32.store
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (i32.const 4) i32.sub
          (global.get $sp) (i32.const 4) i32.sub i32.load
          (global.get $sp) i32.load
          i32.mul
          i32.store
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  )

  (func $__export_SQUARE (param $n1 i32) (result i32)
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (local.get $n1) i32.store
    (call $SQUARE)
    (global.get $sp) i32.load
    (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  )

  (func $PYTHAGORAS
    (local $__a i32)
    (local $__b i32)
    (local $__c i32)
    (local $__i i32)
    (local $__limit i32)
    ;; Corpo traduzido da palavra Forth
    (call $SQUARE)
    (global.get $sp) i32.load (local.set $__b)
          (global.get $sp) (i32.const 4) i32.sub
          i32.load (local.set $__a)
          (global.get $sp) (local.get $__a) i32.store
          (global.get $sp) (i32.const 4) i32.sub (local.get $__b) i32.store
    (call $SQUARE)
    (global.get $sp) (i32.const 4) i32.sub
          (global.get $sp) (i32.const 4) i32.sub i32.load
          (global.get $sp) i32.load
          i32.add
          i32.store
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  )

  (func $__export_PYTHAGORAS (param $n1 i32) (param $n2 i32) (result i32)
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (local.get $n1) i32.store
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (local.get $n2) i32.store
    (call $PYTHAGORAS)
    (global.get $sp) i32.load
    (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  )

  (func $FACTORIAL
    (local $__a i32)
    (local $__b i32)
    (local $__c i32)
    (local $__i i32)
    (local $__limit i32)
    ;; Corpo traduzido da palavra Forth
    (global.get $sp) (i32.const 4) i32.add
          (global.get $sp) i32.load
          i32.store
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (i32.const 1) i32.store
    (global.get $sp) (i32.const 4) i32.sub
          (global.get $sp) (i32.const 4) i32.sub i32.load
          (global.get $sp) i32.load
          i32.le_s
          i32.store
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
    ;; Forth IF
          (global.get $sp) i32.load
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (if
            (then
              (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
              (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (i32.const 1) i32.store
            )
            (else
              (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (i32.const 1) i32.store
              (global.get $sp) i32.load (local.set $__b)
          (global.get $sp) (i32.const 4) i32.sub
          i32.load (local.set $__a)
          (global.get $sp) (local.get $__a) i32.store
          (global.get $sp) (i32.const 4) i32.sub (local.get $__b) i32.store
              (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (i32.const 1) i32.store
              (global.get $sp) (i32.const 4) i32.sub
          (global.get $sp) (i32.const 4) i32.sub i32.load
          (global.get $sp) i32.load
          i32.add
          i32.store
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
              (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (i32.const 2) i32.store
              ;; Forth DO (start limit -> )
          (global.get $sp) i32.load (local.set $__i)
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (global.get $sp) i32.load (local.set $__limit)
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (block $__break_2
            (loop $__loop_2
              (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (local.get $__i) i32.store
              (global.get $sp) (i32.const 4) i32.sub
          (global.get $sp) (i32.const 4) i32.sub i32.load
          (global.get $sp) i32.load
          i32.mul
          i32.store
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
              (local.get $__i) (i32.const 1) i32.add (local.tee $__i)
              (local.get $__limit) i32.ge_s
              (br_if $__break_2)
              (br $__loop_2)
            )
          )
            )
          )
  )

  (func $__export_FACTORIAL (param $n1 i32) (result i32)
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (local.get $n1) i32.store
    (call $FACTORIAL)
    (global.get $sp) i32.load
    (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  )

  (func $MAX_VAL
    (local $__a i32)
    (local $__b i32)
    (local $__c i32)
    (local $__i i32)
    (local $__limit i32)
    ;; Corpo traduzido da palavra Forth
    (global.get $sp) (i32.const 4) i32.sub i32.load (local.set $__a)
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (local.get $__a) i32.store
    (global.get $sp) (i32.const 4) i32.sub i32.load (local.set $__a)
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (local.get $__a) i32.store
    (global.get $sp) (i32.const 4) i32.sub
          (global.get $sp) (i32.const 4) i32.sub i32.load
          (global.get $sp) i32.load
          i32.lt_s
          i32.store
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
    ;; Forth IF
          (global.get $sp) i32.load
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (if
            (then
              (global.get $sp) i32.load (local.set $__b)
          (global.get $sp) (i32.const 4) i32.sub
          i32.load (local.set $__a)
          (global.get $sp) (local.get $__a) i32.store
          (global.get $sp) (i32.const 4) i32.sub (local.get $__b) i32.store
              (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
            )
            (else
              (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
            )
          )
  )

  (func $__export_MAX_VAL (param $n1 i32) (param $n2 i32) (result i32)
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (local.get $n1) i32.store
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (local.get $n2) i32.store
    (call $MAX_VAL)
    (global.get $sp) i32.load
    (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  )
)