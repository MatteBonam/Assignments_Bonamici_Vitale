; ModuleID = 'test/SRtest.optimized.bc'
source_filename = "SRtest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @strength_red(i32 noundef %0) #0 {
  %2 = mul nsw i32 %0, 16
  %shiftLeft = shl i32 %0, 4
  %3 = mul nsw i32 %0, 15
  %shiftP = shl i32 %0, 4
  %sub = sub i32 %shiftP, %0
  %4 = mul nsw i32 %0, 10
  %shiftQ = shl i32 %0, 1
  %shiftP1 = shl i32 %0, 3
  %add = add i32 %shiftP1, %shiftQ
  %5 = mul nsw i32 %0, 47
  %6 = add nsw i32 %shiftLeft, %sub
  %7 = add nsw i32 %add, %5
  %8 = add nsw i32 %6, %7
  ret i32 %8
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 2]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Homebrew clang version 19.1.7"}
