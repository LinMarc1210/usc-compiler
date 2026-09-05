=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> while.cond6, if.end -> if.then14, if.end -> if.end15, if.else -> while.cond1, while.cond1 -> while.end4, while.body3 -> while.cond1, while.cond6 -> while.end9, while.body8 -> while.cond6, if.end15 -> while.cond
INSTRUMENTED: while.cond1 -> while.body3, while.end4 -> if.end, while.cond6 -> while.body8, while.end9 -> if.end, if.then14 -> if.end15
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> while.cond6 : 0
if.end -> if.then14 : 0
if.end -> if.end15 : 2997
if.else -> while.cond1 : 0
while.cond1 -> while.body3 : 4500000
while.cond1 -> while.end4 : 0
while.body3 -> while.cond1 : 0
while.end4 -> if.end : 1500
while.cond6 -> while.body8 : 4500000
while.cond6 -> while.end9 : 0
while.body8 -> while.cond6 : 0
while.end9 -> if.end : 1500
if.then14 -> if.end15 : 3
if.end15 -> while.cond : 0
END_PROFILE
