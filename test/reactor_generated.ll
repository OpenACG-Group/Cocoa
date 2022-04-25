; ModuleID = 'layer0_shader'
source_filename = "layer0_shader"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@__program_host_context = global i8* null

define i32 @__start_user_main(i8* %0) {
  store i8* %0, i8** @__program_host_context, align 8
  %2 = call i32 @__builtin_check_host_context(i8* %0)
  %.not = icmp eq i32 %2, 0
  br i1 %.not, label %normal_ret, label %common.ret

common.ret:                                       ; preds = %1, %normal_ret
  %common.ret.op = phi i32 [ 0, %normal_ret ], [ 1, %1 ]
  ret i32 %common.ret.op

normal_ret:                                       ; preds = %1
  call void @main()
  br label %common.ret
}

declare i32 @__builtin_check_host_context(i8*)

define void @main() {
__user_main_entrypoint:
  %0 = load i8*, i8** @__program_host_context, align 8
  call void @__builtin_v8_trampoline(i8* %0, i32 1)
  ret void
}

declare void @__builtin_v8_trampoline(i8*, i32)
