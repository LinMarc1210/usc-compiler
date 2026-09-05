=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> while.cond8, if.end -> if.then20, if.end -> if.end21, if.else -> while.cond2, while.cond2 -> while.end5, while.body4 -> while.cond2, while.cond8 -> while.end12, while.body11 -> while.cond8, if.end21 -> while.cond
INSTRUMENTED: while.cond2 -> while.body4, while.end5 -> if.end, while.cond8 -> while.body11, while.end12 -> if.end, if.then20 -> if.end21
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> while.cond8 : 0
if.end -> if.then20 : 0
if.end -> if.end21 : 2997
if.else -> while.cond2 : 0
while.cond2 -> while.body4 : 4500000
while.cond2 -> while.end5 : 0
while.body4 -> while.cond2 : 0
while.end5 -> if.end : 1500
while.cond8 -> while.body11 : 4500000
while.cond8 -> while.end12 : 0
while.body11 -> while.cond8 : 0
while.end12 -> if.end : 1500
if.then20 -> if.end21 : 3
if.end21 -> while.cond : 0
END_PROFILE
