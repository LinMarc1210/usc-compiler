=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.then4, if.then -> if.else6, if.end -> while.cond, if.then4 -> if.then10, if.then4 -> if.else12, if.end5 -> if.end, if.then10 -> if.then15, if.then10 -> if.else17, if.end11 -> if.end5, if.end16 -> if.end11
INSTRUMENTED: if.else -> if.end, if.else6 -> if.end5, if.else12 -> if.end11, if.then15 -> if.end16, if.else17 -> if.end16
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then4 : 0
if.then -> if.else6 : 0
if.end -> while.cond : 0
if.else -> if.end : 100000000
if.then4 -> if.then10 : 0
if.then4 -> if.else12 : 0
if.end5 -> if.end : 100000000
if.else6 -> if.end5 : 50000000
if.then10 -> if.then15 : 0
if.then10 -> if.else17 : 0
if.end11 -> if.end5 : 50000000
if.else12 -> if.end11 : 25000000
if.then15 -> if.end16 : 8333333
if.end16 -> if.end11 : 25000000
if.else17 -> if.end16 : 16666667
END_PROFILE
