=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.then4 : 50000000
if.then -> if.else6 : 50000000
if.end -> while.cond : 200000000
if.else -> if.end : 100000000
if.then4 -> if.then10 : 25000000
if.then4 -> if.else12 : 25000000
if.end5 -> if.end : 100000000
if.else6 -> if.end5 : 50000000
if.then10 -> if.then15 : 8333333
if.then10 -> if.else17 : 16666667
if.end11 -> if.end5 : 50000000
if.else12 -> if.end11 : 25000000
if.then15 -> if.end16 : 8333333
if.end16 -> if.end11 : 25000000
if.else17 -> if.end16 : 16666667
END_PROFILE
