=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 10
while.cond -> while.end : 1
while.body -> while.cond : 10
while.end -> while.cond2 : 1
while.cond2 -> while.body5 : 10
while.cond2 -> while.end6 : 1
while.body5 -> while.cond7 : 10
while.cond7 -> while.body10 : 110
while.cond7 -> while.end11 : 10
while.body10 -> while.cond7 : 110
while.end11 -> while.cond2 : 10
END_PROFILE
