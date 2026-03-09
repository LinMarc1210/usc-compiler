=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.body3, while.cond1 -> while.end4, while.body3 -> if.then, while.body3 -> if.else, if.end -> while.cond1, if.else -> if.then7, if.else -> if.else9, if.end8 -> if.end
INSTRUMENTED: while.end4 -> while.cond, if.then -> if.end, if.then7 -> if.end8, if.else9 -> if.end8
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> while.cond1 : 0
while.cond1 -> while.body3 : 0
while.cond1 -> while.end4 : 0
while.body3 -> if.then : 0
while.body3 -> if.else : 0
while.end4 -> while.cond : 500000
if.then -> if.end : 499999
if.end -> while.cond1 : 0
if.else -> if.then7 : 0
if.else -> if.else9 : 0
if.then7 -> if.end8 : 4192515
if.end8 -> if.end : 8475732
if.else9 -> if.end8 : 4283217
END_PROFILE
