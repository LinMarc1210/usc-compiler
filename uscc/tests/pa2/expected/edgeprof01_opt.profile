=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.end4, while.body3 -> while.cond5, while.end4 -> while.cond, while.cond5 -> while.body7, while.cond5 -> while.end8, while.end8 -> while.cond1, while.cond9 -> while.body11, while.cond9 -> while.end12, while.end12 -> while.cond5, if.then -> if.end, if.end -> while.cond9
INSTRUMENTED: while.cond -> while.body, while.cond1 -> while.body3, while.body7 -> while.cond9, while.body11 -> if.then, while.body11 -> if.end
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 120
while.cond -> while.end : 1
while.body -> while.cond1 : 0
while.cond1 -> while.body3 : 14400
while.cond1 -> while.end4 : 0
while.body3 -> while.cond5 : 0
while.end4 -> while.cond : 0
while.cond5 -> while.body7 : 0
while.cond5 -> while.end8 : 0
while.body7 -> while.cond9 : 1728000
while.end8 -> while.cond1 : 0
while.cond9 -> while.body11 : 0
while.cond9 -> while.end12 : 0
while.body11 -> if.then : 207355000
while.body11 -> if.end : 5000
while.end12 -> while.cond5 : 0
if.then -> if.end : 207355000
if.end -> while.cond9 : 207360000
END_PROFILE
