=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, if.then -> if.end, if.end -> if.else6, if.else -> if.end, if.then4 -> if.end5, if.end5 -> if.else13, if.else6 -> if.end5, if.then11 -> if.end12, if.end12 -> if.else20, if.else13 -> if.end12, if.then18 -> if.end19, if.end19 -> if.else27, if.else20 -> if.end19, if.then25 -> if.end26, if.end26 -> while.cond, if.else27 -> if.end26
INSTRUMENTED: while.body -> if.then, while.body -> if.else, if.end -> if.then4, if.end5 -> if.then11, if.end12 -> if.then18, if.end19 -> if.then25
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.end : 0
if.end -> if.then4 : 66666667
if.end -> if.else6 : 0
if.else -> if.end : 0
if.then4 -> if.end5 : 0
if.end5 -> if.then11 : 40000000
if.end5 -> if.else13 : 0
if.else6 -> if.end5 : 0
if.then11 -> if.end12 : 0
if.end12 -> if.then18 : 28571429
if.end12 -> if.else20 : 0
if.else13 -> if.end12 : 0
if.then18 -> if.end19 : 0
if.end19 -> if.then25 : 18181819
if.end19 -> if.else27 : 0
if.else20 -> if.end19 : 0
if.then25 -> if.end26 : 0
if.end26 -> while.cond : 0
if.else27 -> if.end26 : 0
END_PROFILE
