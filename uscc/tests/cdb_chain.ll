; ModuleID = 'cdb_chain'
; Chain of constant branches. After ConstantBranch folds each and
; DeadBlocks removes the unreachable arms, only entry -> c3 -> exit
; should remain reachable.

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  br i1 false, label %c1, label %c2

c1:                                               ; preds = %entry
  br label %exit

c2:                                               ; preds = %entry
  br i1 true, label %c3, label %c4

c3:                                               ; preds = %c2
  br label %exit

c4:                                               ; preds = %c2
  br label %exit

exit:                                             ; preds = %c1, %c3, %c4
  %Phi = phi i32 [ 10, %c1 ], [ 20, %c3 ], [ 30, %c4 ]
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %Phi)
  ret i32 0
}
