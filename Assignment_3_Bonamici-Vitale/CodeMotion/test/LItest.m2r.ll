; ModuleID = 'LItest.m2r.bc'
source_filename = "LItest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @test(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 5, %1
  br label %4

4:                                                ; preds = %11, %2
  %.02 = phi i32 [ undef, %2 ], [ %12, %11 ]
  %.01 = phi i32 [ %1, %2 ], [ %14, %11 ]
  %.0 = phi i32 [ 0, %2 ], [ %15, %11 ]
  %5 = icmp slt i32 %.0, 5
  br i1 %5, label %6, label %.loopexit

6:                                                ; preds = %4
  %7 = add nsw i32 %3, %0
  %8 = icmp slt i32 %.0, 3
  br i1 %8, label %9, label %11

9:                                                ; preds = %6
  %10 = add nsw i32 %3, 3
  br label %16

11:                                               ; preds = %6
  %12 = add nsw i32 %7, 4
  %13 = add nsw i32 %7, 1
  %14 = add nsw i32 %12, 2
  %15 = add nsw i32 %.0, 1
  br label %4, !llvm.loop !6

.loopexit:                                        ; preds = %4
  br label %16

16:                                               ; preds = %.loopexit, %9
  %.1 = phi i32 [ %10, %9 ], [ %.02, %.loopexit ]
  %17 = add nsw i32 %.1, %.01
  ret i32 %17
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @fun(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  br label %4

4:                                                ; preds = %6, %3
  %.01 = phi i32 [ 0, %3 ], [ %9, %6 ]
  %.0 = phi i32 [ %0, %3 ], [ %7, %6 ]
  %5 = icmp slt i32 %.01, 5
  br i1 %5, label %6, label %10

6:                                                ; preds = %4
  %7 = add nsw i32 %1, %2
  %8 = add nsw i32 %7, 1
  %9 = add nsw i32 %.01, 1
  br label %4

10:                                               ; preds = %4
  %11 = add nsw i32 %1, 1
  ret i32 %.0
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @main() #0 {
  %1 = call i32 @fun(i32 noundef 1, i32 noundef 2, i32 noundef 3)
  ret i32 %1
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 5]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Homebrew clang version 19.1.7"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
