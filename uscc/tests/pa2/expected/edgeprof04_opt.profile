=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.then5, if.end -> while.cond, if.then5 -> if.then12, if.then5 -> if.else14, if.end6 -> if.end, if.else7 -> if.end6, if.then12 -> if.then19, if.then12 -> if.else21, if.end13 -> if.end6, if.then19 -> if.then26, if.then19 -> if.else28, if.end20 -> if.end13, if.end27 -> if.end20
INSTRUMENTED: if.then -> if.else7, if.else -> if.end, if.else14 -> if.end13, if.else21 -> if.end20, if.then26 -> if.end27, if.else28 -> if.end27
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then5 : 0
if.then -> if.else7 : 50000000
if.end -> while.cond : 0
if.else -> if.end : 100000000
if.then5 -> if.then12 : 0
if.then5 -> if.else14 : 0
if.end6 -> if.end : 100000000
if.else7 -> if.end6 : 0
if.then12 -> if.then19 : 0
if.then12 -> if.else21 : 0
if.end13 -> if.end6 : 0
if.else14 -> if.end13 : 25000000
if.then19 -> if.then26 : 0
if.then19 -> if.else28 : 0
if.end20 -> if.end13 : 25000000
if.else21 -> if.end20 : 12500000
if.then26 -> if.end27 : 6250000
if.end27 -> if.end20 : 12500000
if.else28 -> if.end27 : 6250000
END_PROFILE
