; ModuleID = 'cdb_nested'
; Two nested constant branches. Both should fold, leaving a single straight
; path through `%a.then` → `%b.else` → `%exit`. Three blocks become dead.

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  br i1 true, label %a.then, label %a.else

a.then:                                           ; preds = %entry
  br i1 false, label %b.then, label %b.else

a.else:                                           ; preds = %entry
  br label %exit

b.then:                                           ; preds = %a.then
  br label %exit

b.else:                                           ; preds = %a.then
  br label %exit

exit:                                             ; preds = %b.else, %b.then, %a.else
  %Phi = phi i32 [ 1, %a.else ], [ 2, %b.then ], [ 3, %b.else ]
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %Phi)
  ret i32 0
}
