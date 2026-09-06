; ModuleID = 'cdb_true'
; Constant-true branch; ConstantBranch should fold to unconditional br
; targeting the then-block; DeadBlocks should remove the else-block.

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  br i1 true, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %Phi = phi i32 [ 7, %if.then ], [ 42, %if.else ]
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %Phi)
  ret i32 0
}
