=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.end4, while.body3 -> while.cond5, while.end4 -> while.cond, while.cond5 -> while.body7, while.cond5 -> while.end8, while.body7 -> if.then, while.body7 -> if.else, while.end8 -> while.cond1, if.end -> while.cond5, if.then14 -> if.end15, if.end15 -> if.end, if.else16 -> if.end15, if.then22 -> if.end23, if.end23 -> if.end, if.else24 -> if.end23
INSTRUMENTED: while.cond -> while.body, while.cond1 -> while.body3, if.then -> if.then22, if.then -> if.else24, if.else -> if.then14, if.else -> if.else16
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
if.then -> if.then22 : 1313400
if.then -> if.else24 : 2666600
if.end -> while.cond5 : 0
if.else -> if.then14 : 2666600
if.else -> if.else16 : 1353400
if.then14 -> if.end15 : 0
if.end15 -> if.end : 0
if.else16 -> if.end15 : 0
if.then22 -> if.end23 : 0
if.end23 -> if.end : 0
if.else24 -> if.end23 : 0
END_PROFILE
