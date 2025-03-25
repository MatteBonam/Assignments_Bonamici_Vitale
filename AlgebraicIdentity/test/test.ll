define i32 @test_mul(i32 %x) {
entry:
    %result1 = mul i32 %x, 1   
    %result2 = mul i32 1, %x    
    ret i32 %result1
}

define i32 @test_add(i32 %x) {
entry:
    %result1 = add i32 %x, 0   
    %result2 = add i32 0, %x   
    ret i32 %result1
}

define i32 @test_add_clean(i32 %x) {
entry:
    %result1 = add i32 %x, 1  
    ret i32 %result1
}

