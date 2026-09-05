=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.end4, while.body3 -> while.cond5, while.end4 -> while.cond, while.cond5 -> while.body7, while.cond5 -> while.end8, while.body7 -> if.then, while.body7 -> if.else, while.end8 -> while.cond1, if.end -> while.cond5, if.then10 -> if.end11, if.end11 -> if.end, if.else12 -> if.end11, if.then15 -> if.end16, if.end16 -> if.end, if.else17 -> if.end16
INSTRUMENTED: while.cond -> while.body, while.cond1 -> while.body3, if.then -> if.then15, if.then -> if.else17, if.else -> if.then10, if.else -> if.else12
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 200
while.cond -> while.end : 1
while.body -> while.cond1 : 0
while.cond1 -> while.body3 : 40000
while.cond1 -> while.end4 : 0
while.body3 -> while.cond5 : 0
while.end4 -> while.cond : 0
while.cond5 -> while.body7 : 0
while.cond5 -> while.end8 : 0
while.body7 -> if.then : 0
while.body7 -> if.else : 0
while.end8 -> while.cond1 : 0
if.then -> if.then15 : 1313400
if.then -> if.else17 : 2666600
if.end -> while.cond5 : 0
if.else -> if.then10 : 2666600
if.else -> if.else12 : 1353400
if.then10 -> if.end11 : 0
if.end11 -> if.end : 0
if.else12 -> if.end11 : 0
if.then15 -> if.end16 : 0
if.end16 -> if.end : 0
if.else17 -> if.end16 : 0
END_PROFILE
