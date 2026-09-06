=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 16
while.cond -> while.end : 1
while.body -> while.cond : 16
while.end -> while.cond2 : 1
while.cond2 -> while.body5 : 200
while.cond2 -> while.end6 : 1
while.body5 -> if.then : 9
while.body5 -> if.end : 191
if.then -> if.end : 9
if.end -> while.cond2 : 200
END_PROFILE
