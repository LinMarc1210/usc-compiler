=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, if.end -> if.end6, if.then5 -> if.end6, if.end6 -> if.end13, if.then12 -> if.end13, if.end13 -> if.end20, if.then19 -> if.end20, if.end20 -> if.end27, if.then26 -> if.end27, if.end27 -> while.cond
INSTRUMENTED: while.body -> if.end, if.then -> if.end, if.end -> if.then5, if.end6 -> if.then12, if.end13 -> if.then19, if.end20 -> if.then26
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.end : 100000000
if.then -> if.end : 100000000
if.end -> if.then5 : 66666667
if.end -> if.end6 : 0
if.then5 -> if.end6 : 0
if.end6 -> if.then12 : 40000000
if.end6 -> if.end13 : 0
if.then12 -> if.end13 : 0
if.end13 -> if.then19 : 28571429
if.end13 -> if.end20 : 0
if.then19 -> if.end20 : 0
if.end20 -> if.then26 : 18181819
if.end20 -> if.end27 : 0
if.then26 -> if.end27 : 0
if.end27 -> while.cond : 0
END_PROFILE
