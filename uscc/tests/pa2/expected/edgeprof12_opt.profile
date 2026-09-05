=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.body3, while.cond1 -> while.end4, while.body3 -> if.then, while.body3 -> if.else, if.end -> while.cond1, if.else -> if.then10, if.else -> if.else12, if.end11 -> if.end
INSTRUMENTED: while.end4 -> while.cond, if.then -> if.end, if.then10 -> if.end11, if.else12 -> if.end11
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> while.cond1 : 0
while.cond1 -> while.body3 : 0
while.cond1 -> while.end4 : 0
while.body3 -> if.then : 0
while.body3 -> if.else : 0
while.end4 -> while.cond : 15000
if.then -> if.end : 15000
if.end -> while.cond1 : 0
if.else -> if.then10 : 0
if.else -> if.else12 : 0
if.then10 -> if.end11 : 112492500
if.end11 -> if.end : 224985000
if.else12 -> if.end11 : 112492500
END_PROFILE
