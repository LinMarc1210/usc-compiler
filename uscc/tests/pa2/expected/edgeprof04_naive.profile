=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.then5 : 50000000
if.then -> if.else7 : 50000000
if.end -> while.cond : 200000000
if.else -> if.end : 100000000
if.then5 -> if.then12 : 25000000
if.then5 -> if.else14 : 25000000
if.end6 -> if.end : 100000000
if.else7 -> if.end6 : 50000000
if.then12 -> if.then19 : 12500000
if.then12 -> if.else21 : 12500000
if.end13 -> if.end6 : 50000000
if.else14 -> if.end13 : 25000000
if.then19 -> if.then26 : 6250000
if.then19 -> if.else28 : 6250000
if.end20 -> if.end13 : 25000000
if.else21 -> if.end20 : 12500000
if.then26 -> if.end27 : 6250000
if.end27 -> if.end20 : 12500000
if.else28 -> if.end27 : 6250000
END_PROFILE
