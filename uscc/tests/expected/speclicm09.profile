=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 32
while.cond -> while.end : 1
while.body -> if.then : 2
while.body -> if.end : 30
while.end -> while.cond9 : 1
if.then -> if.end : 2
if.end -> while.cond : 32
while.cond9 -> while.body12 : 32
while.cond9 -> while.end13 : 1
while.body12 -> while.cond9 : 32
END_PROFILE
