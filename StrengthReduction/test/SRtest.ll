; ModuleID = 'SRtest.c'
source_filename = "SRtest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @strength_red(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %7 = load i32, ptr %2, align 4
  %8 = mul nsw i32 %7, 16
  store i32 %8, ptr %3, align 4
  %9 = load i32, ptr %2, align 4
  %10 = mul nsw i32 %9, 15
  store i32 %10, ptr %4, align 4
  %11 = load i32, ptr %2, align 4
  %12 = mul nsw i32 %11, 10
  store i32 %12, ptr %5, align 4
  %13 = load i32, ptr %2, align 4
  %14 = mul nsw i32 %13, 47
  store i32 %14, ptr %6, align 4
  %15 = load i32, ptr %3, align 4
  %16 = load i32, ptr %4, align 4
  %17 = add nsw i32 %15, %16
  %18 = load i32, ptr %5, align 4
  %19 = load i32, ptr %6, align 4
  %20 = add nsw i32 %18, %19
  %21 = add nsw i32 %17, %20
  ret i32 %21
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
